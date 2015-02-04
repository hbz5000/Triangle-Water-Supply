/* Copyright 2012-2013 The Pennsylvania State University
 *
 * This software was written by David Hadka and others.
 * 
 * The use, modification and distribution of this software is governed by the
 * The Pennsylvania State University Research and Educational Use License.
 * You should have received a copy of this license along with this program.
 * If not, contact <dmh309@psu.edu>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include "borg.h"

typedef enum ConstraintMode {
	LT,
	GT,
	EQ
} ConstraintMode;

typedef struct Parameter_t* Parameter;

struct Parameter_t {
	char* name;
	double lowerBound;
	double upperBound;
	double value;
	Parameter next;
};

/* command line options */
int maxEvaluations = -1;
int numberOfVariables = -1;
int numberOfObjectives = -1;
int numberOfConstraints = 0;
double* lowerBounds = NULL;
double* upperBounds = NULL;
double* epsilons = NULL;
double* directions = NULL;
ConstraintMode constraintMode = EQ;
char* runtimeFile = NULL;
int frequency = 100;
char* parameterFile = NULL;
char* samplesFile = NULL;
char* outputFile = NULL;
char* seed = NULL;
int useSocket = 0;
char* host = NULL;
char* port = NULL;
int delay = 1;
BORG_Restart restartMode = RESTART_DEFAULT;
int maxMutationIndex = 10;
BORG_Probabilities probabilityMode = PROBABILITIES_DEFAULT;
int debug = 0;

/* internal variables */
int sockfd;
int pipetoexe[2];
int pipefromexe[2];
FILE* pipein;
FILE* pipeout;
BORG_Problem problem;
Parameter parameters = NULL;
FILE* runtime = NULL;
FILE* samples = NULL;
FILE* output = NULL;

/**
 * Pipes the variables to the executable, blocking until the objectives and constraints
 * are returned.
 */
void function(double* variables, double* objectives, double* constraints) {
	int i;

	for (i=0; i<numberOfVariables; i++) {
		if (i > 0) {
			fprintf(pipeout, " ");
		}

		fprintf(pipeout, "%0.17g", variables[i]);
	}

	fprintf(pipeout, "\n");
	fflush(pipeout);

	for (i=0; i<numberOfObjectives; i++) {
		if (fscanf(pipein, "%lg", &objectives[i]) != 1) {
			perror("fscanf");
			exit(EXIT_FAILURE);
		}

		/* negate maximization objectives */
		if (directions[i]) {
			objectives[i] *= -1;
		}
	}

	for (i=0; i<numberOfConstraints; i++) {
		if (fscanf(pipein, "%lg", &constraints[i]) != 1) {
			perror("fscanf");
			exit(EXIT_FAILURE);
		}

		/* normalize constraints, so that 0.0 represents feasible solutions */
		if (constraintMode == LT) {
			if (constraints[i] <= 0.0) {
				constraints[i] = 0.0;
			}	
		} else if (constraintMode == GT) {
			if (constraints[i] >= 0.0) {
				constraints[i] = 0.0;
			}
		}
	}
}

/**
 * Parses the comma-separated command line arguments.
 */
void parseArgumentArray(char* argument, int size, double* values) {
	int i;

	for (i=0; i<size; i++) {
		if (i > 0) {
			if (*argument != ',') {
				fprintf(stderr, "Failed parsing argument, invalid format or invalid number of elements\n");
				exit(EXIT_FAILURE);
			}

			argument++;
		}

		values[i] = strtod(argument, &argument);
	}

	if (*argument != '\0') {
		fprintf(stderr, "Failed parsing argument, invalid format or invalid number of elements\n");
		exit(EXIT_FAILURE);
	}
}

/**
 * Frees the memory used by the parameters list.
 */
void freeParameters() {
	Parameter parameter = parameters;
	Parameter next;

	while (parameter) {
		next = parameter->next;
		free(parameter->name);
		free(parameter);
		parameter = next;
	}
}

/**
 * Appends the specified parameter to the parameters list.
 */
void appendParameter(char* name, double lowerBound, double upperBound) {
	Parameter parameter = (Parameter)malloc(sizeof(struct Parameter_t));

	if (!parameter) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	parameter->name = name;
	parameter->lowerBound = lowerBound;
	parameter->upperBound = upperBound;
	parameter->next = NULL;

	if (parameters == NULL) {
		// if this is the first parameter
		parameters = parameter;
	} else {
		// otherwise, append to the end
		Parameter lastParameter = parameters;

		while (lastParameter->next != NULL) {
			lastParameter = lastParameter->next;
		}

		lastParameter->next = parameter;
	}
}

/**
 * Loads the parameter description file, populating the parameters list.
 *
 * This method uses the unsafe fscanf method, and should be fixed if accessible to
 * untrusted inputs.  Do NOT use parameter names longer than 99 characters.
 */
void loadParameterFile(char* parameterFile) {
	char* name = (char*)calloc(100, sizeof(char));
	double lowerBound = 0.0;
	double upperBound = 0.0;
	FILE* fp = fopen(parameterFile, "r");

	if (!fp) {
		fprintf(stderr, "Failed opening parameter description file\n");
		exit(EXIT_FAILURE);
	}

	while (fscanf(fp, "%s %lg %lg", name, &lowerBound, &upperBound) == 3) {
		if (name[99] != '\0') {
			fprintf(stderr, "Parameter name too long\n");
			exit(EXIT_FAILURE);
		}

		appendParameter(name, lowerBound, upperBound);

		name = (char*)calloc(100, sizeof(char));
	}

	free(name); /* the last name is unused */
	fclose(fp);
}

/**
 * Reads the next sample from the parameter input file, returning 1 if the next sample
 * was read; otherwise 0.
 */
int loadNextSample() {
	int size;
	Parameter parameter = parameters;

	while (parameter) {
		size = fscanf(samples, "%lg", &parameter->value);

		if (size == EOF) {
			return 0;
		} else if (size != 1) {
			perror("fscanf");
			exit(EXIT_FAILURE);
		}

		parameter = parameter->next;
	}

	return 1;
}

/**
 * Returns the value of the specified parameter from the last invocation of loadNextSample().
 * If the parameter is not explicitly defined in the parameter file, then default settings are
 * returned.
 */
double getParameterValue(const char* name) {
	Parameter parameter = parameters;

	while (parameter) {
		if (strcmp(name, parameter->name) == 0) {
			return parameter->value;
		}

		parameter = parameter->next;
	}

	/* default settings */
	if (strcmp(name, "pm.rate") == 0) {
		return 1.0 / numberOfVariables;
	} else if (strcmp(name, "pm.distributionIndex") == 0) {
		return 20.0;
	} else if (strcmp(name, "sbx.rate") == 0) {
		return 1.0;
	} else if (strcmp(name, "sbx.distributionIndex") == 0) {
		return 15.0;
	} else if (strcmp(name, "de.crossoverRate") == 0) {
		return 0.1;
	} else if (strcmp(name, "de.stepSize") == 0) {
		return 0.5;
	} else if (strcmp(name, "um.rate") == 0) {
		return 1.0 / numberOfVariables;
	} else if (strcmp(name, "spx.parents") == 0) {
		return 10;
	} else if (strcmp(name, "spx.offspring") == 0) {
		return 2;
	} else if (strcmp(name, "spx.epsilon") == 0) {
		return 3.0; //sqrt(numberOfVariables+1);
	} else if (strcmp(name, "pcx.parents") == 0) {
		return 10;
	} else if (strcmp(name, "pcx.offspring") == 0) {
		return 2;
	} else if (strcmp(name, "pcx.eta") == 0) {
		return 0.1;
	} else if (strcmp(name, "pcx.zeta") == 0) {
		return 0.1;
	} else if (strcmp(name, "undx.parents") == 0) {
		return 10;
	} else if (strcmp(name, "undx.offspring") == 0) {
		return 2;
	} else if (strcmp(name, "undx.eta") == 0) {
		return 0.35;
	} else if (strcmp(name, "undx.zeta") == 0) {
		return 0.5;
	} else if (strcmp(name, "initialPopulationSize") == 0) {
		return 100;
	} else if (strcmp(name, "maxEvaluations") == 0) {
		return 10000;
	} else if (strcmp(name, "injectionRate") == 0) {
		return 0.25;
	} else {
		fprintf(stderr, "Undefined parameter %s\n", name);
		exit(EXIT_FAILURE);
	}
}

long timediff(struct timeval* start, struct timeval* end) {
	struct timeval diff;
	timersub(end, start, &diff);
	return diff.tv_sec*1000000L + diff.tv_usec;
}

/**
 * Executes the BORG algorithm using any assigned parameter settings.
 */
void runWithParameters() {
	int maxEvaluations = (int)getParameterValue("maxEvaluations");
	int currentEvaluations = 0;
	int lastSnapshot = 0;
	struct timeval start;
	struct timeval now;

	gettimeofday(&start, NULL);

	BORG_Operator pm = BORG_Operator_create("PM", 1, 1, 2, BORG_Operator_PM);
	BORG_Operator_set_parameter(pm, 0, getParameterValue("pm.rate"));
	BORG_Operator_set_parameter(pm, 1, getParameterValue("pm.distributionIndex"));

	BORG_Operator sbx = BORG_Operator_create("SBX", 2, 2, 2, BORG_Operator_SBX);
	BORG_Operator_set_parameter(sbx, 0, getParameterValue("sbx.rate"));
	BORG_Operator_set_parameter(sbx, 1, getParameterValue("sbx.distributionIndex"));
	BORG_Operator_set_mutation(sbx, pm);

	BORG_Operator de = BORG_Operator_create("DE", 4, 1, 2, BORG_Operator_DE);
	BORG_Operator_set_parameter(de, 0, getParameterValue("de.crossoverRate"));
	BORG_Operator_set_parameter(de, 1, getParameterValue("de.stepSize"));
	BORG_Operator_set_mutation(de, pm);

	BORG_Operator um = BORG_Operator_create("UM", 1, 1, 1, BORG_Operator_UM);
	BORG_Operator_set_parameter(um, 0, getParameterValue("um.rate"));

	BORG_Operator spx = BORG_Operator_create("SPX", (int)getParameterValue("spx.parents"), 
		(int)getParameterValue("spx.offspring"), 1, BORG_Operator_SPX);
	BORG_Operator_set_parameter(spx, 0, getParameterValue("spx.epsilon"));

	BORG_Operator pcx = BORG_Operator_create("PCX", (int)getParameterValue("pcx.parents"), 
		(int)getParameterValue("pcx.offspring"), 2, BORG_Operator_PCX);
	BORG_Operator_set_parameter(pcx, 0, getParameterValue("pcx.eta"));
	BORG_Operator_set_parameter(pcx, 1, getParameterValue("pcx.zeta"));

	BORG_Operator undx = BORG_Operator_create("UNDX", (int)getParameterValue("undx.parents"), 
		(int)getParameterValue("undx.offspring"), 2, BORG_Operator_UNDX);
	BORG_Operator_set_parameter(undx, 0, getParameterValue("undx.zeta"));
	BORG_Operator_set_parameter(undx, 1, getParameterValue("undx.eta"));

	BORG_Algorithm algorithm = BORG_Algorithm_create(problem, 6);
	BORG_Algorithm_set_operator(algorithm, 0, sbx);
	BORG_Algorithm_set_operator(algorithm, 1, de);
	BORG_Algorithm_set_operator(algorithm, 2, pcx);
	BORG_Algorithm_set_operator(algorithm, 3, spx);
	BORG_Algorithm_set_operator(algorithm, 4, undx);
	BORG_Algorithm_set_operator(algorithm, 5, um);

	BORG_Algorithm_set_initial_population_size(algorithm, (int)getParameterValue("initialPopulationSize"));
	BORG_Algorithm_set_population_ratio(algorithm, 1.0 / getParameterValue("injectionRate"));
	BORG_Algorithm_set_restart_mode(algorithm, restartMode);
	BORG_Algorithm_set_max_mutation_index(algorithm, maxMutationIndex);
	BORG_Algorithm_set_probability_mode(algorithm, probabilityMode);

	while ((currentEvaluations = BORG_Algorithm_get_nfe(algorithm)) < maxEvaluations) {
		BORG_Algorithm_step(algorithm);

		if (runtime && (currentEvaluations - lastSnapshot >= frequency)) {
			gettimeofday(&now, NULL);
			fprintf(runtime, "//NFE=%d\n", currentEvaluations);
			fprintf(runtime, "//ElapsedTime=%0.17lg\n", timediff(&start, &now)/(double)(1000000L));
			fprintf(runtime, "//SBX=%0.17lg\n", BORG_Operator_get_probability(sbx));
			fprintf(runtime, "//DE=%0.17lg\n", BORG_Operator_get_probability(de));
			fprintf(runtime, "//PCX=%0.17lg\n", BORG_Operator_get_probability(pcx));
			fprintf(runtime, "//SPX=%0.17lg\n", BORG_Operator_get_probability(spx));
			fprintf(runtime, "//UNDX=%0.17lg\n", BORG_Operator_get_probability(undx));
			fprintf(runtime, "//UM=%0.17lg\n", BORG_Operator_get_probability(um));
			fprintf(runtime, "//Improvements=%d\n", BORG_Algorithm_get_number_improvements(algorithm));
			fprintf(runtime, "//Restarts=%d\n", BORG_Algorithm_get_number_restarts(algorithm));
			fprintf(runtime, "//PopulationSize=%d\n", BORG_Algorithm_get_population_size(algorithm));
			fprintf(runtime, "//ArchiveSize=%d\n", BORG_Algorithm_get_archive_size(algorithm));

			if (restartMode == RESTART_ADAPTIVE) {
				fprintf(runtime, "//MutationIndex=%d\n", BORG_Algorithm_get_mutation_index(algorithm));
			}

			BORG_Archive result = BORG_Algorithm_get_result(algorithm);
			BORG_Archive_append(result, runtime);
			BORG_Archive_destroy(result);

			lastSnapshot = currentEvaluations;
		}
	}

	BORG_Archive result = BORG_Algorithm_get_result(algorithm);
	BORG_Archive_append(result, output);

	BORG_Debug("Improvements - %d\n", BORG_Archive_get_improvements(result));
	BORG_Debug("NFE - %d\n", BORG_Algorithm_get_nfe(algorithm));

	BORG_Operator_destroy(sbx);
	BORG_Operator_destroy(de);
	BORG_Operator_destroy(pm);
	BORG_Operator_destroy(um);
	BORG_Operator_destroy(spx);
	BORG_Operator_destroy(pcx);
	BORG_Operator_destroy(undx);
	BORG_Archive_destroy(result);
	BORG_Algorithm_destroy(algorithm);
}

int openSocket(const char* host, const char* service) {
	int status;
	int sockfd;
	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (host == NULL) {
		hints.ai_flags = AI_PASSIVE;
	}

	if (service == NULL) {
		service = "16801";
	}

	if ((status = getaddrinfo(host, service, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}

	if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}

	return sockfd;
}

/**
 * The BORG command-line interface.
 */
int main(int argc, char* argv[]) {
	int i;
	int opt;
	int value;
	time_t start;

	time(&start);

	/* parse command line arguments */
	opterr = 0;

	while ((opt = getopt(argc, argv, "hn:o:v:l:u:c:e:d:C:f:R:F:p:i:s:SH:P:D:I:M:U:X:")) != -1) {
		switch (opt) {
		case 'h':
			BORG_Copyright(stdout);
			printf("\nVersion: %s\n", BORG_VERSION);
			printf("Usage: %s [options] executable\n", argv[0]);
			printf("\n == Basic Options ==\n");
			printf("   -h          : Display help information\n");
			printf("   -v <value>  : Number of decision variables\n");
			printf("   -o <value>  : Number of objectives\n");
			printf("   -c <value>  : Number of constraints\n");
			printf("   -l <values> : Comma-separated list of decision variable lower bounds\n");
			printf("   -u <values> : Comma-separated list of decision variable upper bounds\n");
			printf("   -e <values> : Comma-separated list of epsilon values\n");
			printf("   -d <values> : Comma-separated list of direction flags (0=min, 1=max)\n");
			printf("   -C <option> : Constraints are feasible when (E=equal to zero (default),\n");
			printf("                 L=less than or equal to zero, G=greater than or equal to zero\n");
			printf("   -f <file>   : Approximation set output file\n");
			printf("   -s <value>  : Pseudo-random number generator seed\n");
			printf("\n == Single Run with Default Settings ==\n");
			printf("   -n <value>  : Maximum number of evaluations\n");
			printf("   -R <file>   : Runtime dynamics output file\n");
			printf("   -F <value>  : Frequency (NFE) for recording runtime dynamics\n");
			printf("\n == Many Runs with Set Parameters ==\n");
			printf("   -p <file>   : Parameter description file\n");
			printf("   -i <file>   : Parameter input file\n");
			printf("\n == Communicating over Sockets ==\n");
			printf("   -S          : Enable socket communication\n");
			printf("   -H <value>  : Host name or IP address (default is localhost)\n");
			printf("   -P <value>  : Port number (default is 16801)\n");
			printf("   -D <value>  : Startup delay (default is 1 sec)\n");
			printf("\n == Experimental Options ==\n");
			printf("   -I <value>  : The restart injection mode (0=1/L mutation (default),\n");
			printf("                 1=100%% mutation, 2=ramped mutation, 3=adaptive)\n");
			printf("   -M <value>  : The max mutation index when adaptive restarts is\n");
			printf("                 enabled (default is 10)\n");
			printf("   -U <value>  : The operator probability update mode (0=archive membership\n");
			printf("                 (default), 1=archive recency, 2=both, 3=adaptive)\n");
			printf("\n == Example ==\n");
			printf("   ./borg.exe -v 11 -o 2 -l 0,0,0,0,0,0,0,0,0,0,0 -u 1,1,1,1,1,1,1,1,1,1,1\n");
			printf("       -e 0.01,0.01 -n 10000 -- python dtlz2.py\n");       
			printf("\n");
			
			exit(EXIT_SUCCESS);
		case 'n':
			maxEvaluations = atoi(optarg);
			break;
		case 'o':
			numberOfObjectives = atoi(optarg);
			break;
		case 'v':
			numberOfVariables = atoi(optarg);
			break;
		case 'c':
			numberOfConstraints = atoi(optarg);
			break;
		case 'l':
			if (numberOfVariables < 0) {
				fprintf(stderr, "The -v option must be specified before -l\n");
				exit(EXIT_FAILURE);
			}

			lowerBounds = (double*)calloc(numberOfVariables, sizeof(double));
			parseArgumentArray(optarg, numberOfVariables, lowerBounds);
			break;
		case 'u':
			if (numberOfVariables < 0) {
				fprintf(stderr, "The -v option must be specified before -u\n");
				exit(EXIT_FAILURE);
			}

			upperBounds = (double*)calloc(numberOfVariables, sizeof(double));
			parseArgumentArray(optarg, numberOfVariables, upperBounds);
			break;
		case 'e':
			if (numberOfObjectives < 0) {
				fprintf(stderr, "The -o option must be specified before -e\n");
				exit(EXIT_FAILURE);
			}

			epsilons = (double*)calloc(numberOfObjectives, sizeof(double));
			parseArgumentArray(optarg, numberOfObjectives, epsilons);
			break;
		case 'd':
			if (numberOfObjectives < 0) {
				fprintf(stderr, "The -o option must be specified before -d\n");
				exit(EXIT_FAILURE);
			}

			directions = (double*)calloc(numberOfObjectives, sizeof(double));
			parseArgumentArray(optarg, numberOfObjectives, directions);
			break;
		case 'C':
			if (toupper(optarg[0]) == 'L') {
				constraintMode = LT;
			} else if (toupper(optarg[0]) == 'G') {
				constraintMode = GT;
			} else if (toupper(optarg[0]) == 'E') {
				constraintMode = EQ;
			} else {
				fprintf(stderr, "Invalid -m option value, use L, G or E\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'R':
			runtimeFile = optarg;
			break;
		case 'F':
			frequency = atoi(optarg);
			break;
		case 'f':
			outputFile = optarg;
			break;
		case 'p':
			parameterFile = optarg;
			break;
		case 'i':
			samplesFile = optarg;
			break;
		case 's':
			seed = optarg;
			break;
		case 'S':
			useSocket = 1;
			break;
		case 'H':
			useSocket = 1;
			host = optarg;
			break;
		case 'P':
			useSocket = 1;
			port = optarg;
			break;
		case 'D':
			useSocket = 1;
			delay = atoi(optarg);
			break;
		case 'I':
			value = atoi(optarg);

			if (value == 0) {
				restartMode = RESTART_DEFAULT;
			} else if (value == 1) {
				restartMode = RESTART_RANDOM;
			} else if (value == 2) {
				restartMode = RESTART_RAMPED;
			} else if (value == 3) {
				restartMode = RESTART_ADAPTIVE;
			} else if (value == 4) {
				restartMode = RESTART_INVERTED;
			} else {
				fprintf(stderr, "Unknown restart injection mode\n");
				exit(EXIT_FAILURE);
			}

			break;
		case 'M':
			maxMutationIndex = atoi(optarg);
			break;
		case 'U':
			value = atoi(optarg);

			if (value == 0) {
				probabilityMode = PROBABILITIES_DEFAULT;
			} else if (value == 1) {
				probabilityMode = PROBABILITIES_RECENCY;
			} else if (value == 2) {
				probabilityMode = PROBABILITIES_BOTH;
			} else if (value == 3) {
				probabilityMode = PROBABILITIES_ADAPTIVE;
			} else {
				fprintf(stderr, "Unknown operator probability update mode\n");
				exit(EXIT_FAILURE);
			}

			break;
		case 'X': // special, hidden extensions
			if (strcmp(optarg, "debug") == 0) {
				debug = 1;
			}

			break;
		case '?':
		default:
			fprintf(stderr, "Unrecognized option %c\n", optopt);
			exit(EXIT_FAILURE);
		}
	}

	/* validate command line arguments */
	if ((optind >= argc) && !useSocket) {
		fprintf(stderr, "Missing executable name\n");
		exit(EXIT_FAILURE);
	}

	if (maxEvaluations < 0 && !parameterFile) {
		fprintf(stderr, "The -n or -p option is required\n");
		exit(EXIT_FAILURE);
	}

	if (numberOfVariables < 0) {
		fprintf(stderr, "The -v option is required\n");
		exit(EXIT_FAILURE);
	}

	if (numberOfObjectives < 0) {
		fprintf(stderr, "The -o option is required\n");
		exit(EXIT_FAILURE);
	}

	if (!lowerBounds && !upperBounds) {
		lowerBounds = (double*)calloc(numberOfVariables, sizeof(double));
		upperBounds = (double*)calloc(numberOfVariables, sizeof(double));

		for (i=0; i<numberOfVariables; i++) {
			lowerBounds[i] = 0.0;
			upperBounds[i] = 1.0;
		}
	} else if (!lowerBounds || !upperBounds) {
		fprintf(stderr, "Both -l and -u options are required\n");
		exit(EXIT_FAILURE);
	}

	if (!epsilons) {
		epsilons = (double*)calloc(numberOfObjectives, sizeof(double));

		for (i=0; i<numberOfObjectives; i++) {
			epsilons[i] = 0.01;
		}
	}

	if (!directions) {
		directions = (double*)calloc(numberOfObjectives, sizeof(double));

		for (i=0; i<numberOfObjectives; i++) {
			directions[i] = 0;
		}
	}

	if (runtimeFile && parameterFile) {
		fprintf(stderr, "Runtime output only available in single run mode\n");
		exit(EXIT_FAILURE);
	} else if (runtimeFile) {
		runtime = fopen(runtimeFile, "w");

		if (!runtime) {
			fprintf(stderr, "Failed to open runtime file\n");
			exit(EXIT_FAILURE);	
		}
	}

	if (outputFile) {
		output = fopen(outputFile, "w");

		if (!output) {
			fprintf(stderr, "Failed to open output file\n");
			exit(EXIT_FAILURE);
		}
	} else {
		output = stdout;
	}

	if (parameterFile) {
		loadParameterFile(parameterFile);
	}

	if (samplesFile) {
		if (!parameterFile) {
			fprintf(stderr, "Both -i and -p options are required\n");
			exit(EXIT_FAILURE);
		}

		samples = fopen(samplesFile, "r");

		if (!samples) {
			fprintf(stderr, "Failed to open parameter sample file\n");
			exit(EXIT_FAILURE);
		}
	} else if (parameterFile) {
		samples = stdin;
	}

	if (seed) {
		BORG_Random_seed(atol(seed));
	} else {
		BORG_Random_seed(start);
	}

	/* print output header */
	fprintf(output, "# BORG version: %s\n", BORG_VERSION);
	fprintf(output, "# Current time: %s", ctime(&start));
	fprintf(output, "# Problem: %s", argv[optind]);
	for (i=optind+1; i<argc; i++) {
		fprintf(output, " %s", argv[i]);
	}
	fprintf(output, "\n");
	fprintf(output, "# Number of variables: %d\n", numberOfVariables);
	fprintf(output, "# Number of objectives: %d\n", numberOfObjectives);
	fprintf(output, "# Number of constraints: %d\n", numberOfConstraints);
	fprintf(output, "# Lower bounds:");
	for (i=0; i<numberOfVariables; i++) {
		fprintf(output, " %g", lowerBounds[i]);
	}
	fprintf(output, "\n");
	fprintf(output, "# Upper bounds:");
	for (i=0; i<numberOfVariables; i++) {
		fprintf(output, " %g", upperBounds[i]);
	}
	fprintf(output, "\n");
	fprintf(output, "# Epsilons:");
	for (i=0; i<numberOfObjectives; i++) {
		fprintf(output, " %g", epsilons[i]);
	}
	fprintf(output, "\n");
	fprintf(output, "# Directions:");
	for (i=0; i<numberOfObjectives; i++) {
		fprintf(output, " %g", directions[i]);
	}
	fprintf(output, "\n");
	fprintf(output, "# Constraint mode: %s\n", constraintMode == LT ? "L" : constraintMode == GT ? "G" : "E");
	if (samples) {
		fprintf(output, "# Parameter file: %s\n", parameterFile);
		fprintf(output, "# Samples file: %s\n", samplesFile ? samplesFile : "(stdin)");
	}
	fprintf(output, "# Seed: %s\n", seed ? seed : "(time)");

	/* setup and run the algorithm */
	if (debug) {
		BORG_Debug_on();
	}

	if (!useSocket) {
		if (pipe(pipetoexe) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}

		if (pipe(pipefromexe) == -1) {
			perror("pipe");
			exit(EXIT_FAILURE);
		}
	}

	pid_t pid = fork();

	if (pid == -1) {
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		/* child (function evaluation) process */
		if (!useSocket) {
			if (dup2(pipetoexe[0], 0) == -1 || dup2(pipefromexe[1], 1) == -1) {
				perror("dup2");
				exit(EXIT_FAILURE);
			}
		}

		/* no executable defined */
		if (optind >= argc) {
			exit(EXIT_SUCCESS);
		}	

		if (execvp(argv[optind], argv+optind) == -1) {
			perror("execv");
			_exit(EXIT_FAILURE);
		}
	} else {
		/* parent (algorithm) process */
		if (useSocket) {
			sleep(delay);

			sockfd = openSocket(host, port);
			
			if ((pipein = fdopen(sockfd, "r")) == NULL) {
				perror("fdopen");
				exit(EXIT_FAILURE);
			}

			if ((pipeout = fdopen(dup(sockfd), "w")) == NULL) {
				perror("fdopen");
				exit(EXIT_FAILURE);
			}
		} else {
			if ((pipein = fdopen(pipefromexe[0], "r")) == NULL) {
				perror("fdopen");
				exit(EXIT_FAILURE);
			}

			if ((pipeout = fdopen(pipetoexe[1], "w")) == NULL) {
				perror("fdopen");
				exit(EXIT_FAILURE);
			}
		} 

		problem = BORG_Problem_create(numberOfVariables, numberOfObjectives, 
			numberOfConstraints, function);

		for (i=0; i<numberOfVariables; i++) {
			BORG_Problem_set_bounds(problem, i, lowerBounds[i], upperBounds[i]);
		}

		for (i=0; i<numberOfObjectives; i++) {
			BORG_Problem_set_epsilon(problem, i, epsilons[i]);
		}

		if (samples) {
			int count = 0;

			while (loadNextSample()) {
				count++;

				if (outputFile) {
					printf("\rProcessing sample %d...", count);
					fflush(stdout);
				}

				runWithParameters();
			}

			if (outputFile) {
				printf("\n");
			}
		} else {
			/* add maxEvaluations parameter, rest are defaults */
			char* name = (char*)calloc(15, sizeof(char));
			strcpy(name, "maxEvaluations");
			appendParameter(name, maxEvaluations, maxEvaluations);
			parameters->value = maxEvaluations;

			runWithParameters();
		}

		BORG_Problem_destroy(problem);
		fprintf(pipeout, "\n");
		fflush(pipeout);
		fclose(pipeout);
		fclose(pipein);
		freeParameters();
	}

	fprintf(output, "# Finished\n");

	if (outputFile) {
		printf("Finished, output written to %s\n", outputFile);
	}

	if ((samples != NULL) && (samples != stdin)) {
		fclose(samples);
	}

	if ((output != NULL) && (output != stdout)) {
		fclose(output);
	}

	if (useSocket) {
		close(sockfd);
	} else {
		close(pipetoexe[1]);
		close(pipetoexe[0]);
		close(pipefromexe[1]);
		close(pipefromexe[0]);
	}

	free(lowerBounds);
	free(upperBounds);
	free(epsilons);
	free(directions);

	return EXIT_SUCCESS;
}
