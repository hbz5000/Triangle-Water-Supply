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
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "mpi.h"
#include "borgmm.h"
#include "borg.c" /* import c file directly to access opaque pointers */

//#define BORG_MPI_NONBLOCK

/* LIFO queue structure */
typedef struct BORG_Queue_t *BORG_Queue;

struct BORG_Queue_t {
	BORG_Solution solution;
	BORG_Queue next;	
};

/* MPI shared variables */
static int aerovisFormat = 0;
static int maxEvaluations = 0;
static double maxHours = 0.0;
static int numberOfIslands = 1;
static BORG_Initialization initialization = INITIALIZATION_UNIFORM;
static BORG_Problem problem = NULL;
static int worldRank;
static int worldSize;
static int island;
static int islandRank;
static int islandSize;
static MPI_Comm islandComm;
static char name[256];

/* MPI master variables */
static BORG_Solution* processing = NULL;
static BORG_Queue queue = NULL;
static int runtimeFrequency = 100;
static char* runtimeFilename = NULL;
static char* timingFilename = NULL;
static FILE* timing = NULL;
static struct timeval start;
static int updateFrequency = 10000;
static int delayForHelp = 5;
static int helpRequests = 0;

#ifdef BORG_MPI_NONBLOCK
static MPI_Request* requests = NULL;
#endif

/* Methods for configuring the parallel algorithm */
void BORG_Algorithm_output_timing(char* filename) {
	timingFilename = filename;
}

void BORG_Algorithm_output_runtime(char* filename) {
	runtimeFilename = filename;
}

void BORG_Algorithm_output_frequency(int frequency) {
	runtimeFrequency = frequency;
}

void BORG_Algorithm_output_aerovis() {
	aerovisFormat = 1;
}

void BORG_Algorithm_ms_max_evaluations(int _maxEvaluations) {
	maxEvaluations = _maxEvaluations;
}

void BORG_Algorithm_ms_max_time(double _maxHours) {
	maxHours = _maxHours;
}

void BORG_Algorithm_ms_islands(int _numberOfIslands) {
	if ((_numberOfIslands < 1) || (_numberOfIslands > worldSize-1)) {
		fprintf(stderr, "Invalid number of islands\n");
		MPI_Abort(MPI_COMM_WORLD, -1);
	}
	
	numberOfIslands = _numberOfIslands;
}

void BORG_Algorithm_ms_update_frequency(int _updateFrequency) {
	updateFrequency = _updateFrequency;
}

void BORG_Algorithm_ms_initialization(BORG_Initialization _initialization) {
	initialization = _initialization;
}

void BORG_Algorithm_ms_delay_for_help(int _delayForHelp) {
	delayForHelp = _delayForHelp;
}

/* LIFO queue methods */
void BORG_Queue_push(BORG_Solution solution) {
	BORG_Validate_pointer(solution);

	BORG_Queue head = (BORG_Queue)malloc(sizeof(struct BORG_Queue_t));

	BORG_Validate_malloc(head);

	head->solution = solution;
	head->next = queue;
	queue = head;
}

BORG_Solution BORG_Queue_pop() {
	if (queue) {
		BORG_Queue head = queue;
		BORG_Solution solution = head->solution;
	
		queue = head->next;
		free(head);	

		return solution;
	} else {	
		return NULL;
	}
}

/* high-resolution timer */
#include <sys/time.h>

#ifndef USEC_TO_SEC
#define USEC_TO_SEC 1000000L
#endif

static struct timeval BORG_Timer_startval;
static struct timeval BORG_Timer_endval;

long BORG_Timer_diff(struct timeval* start, struct timeval* end) {
	struct timeval diff;
	timersub(end, start, &diff);
	return diff.tv_sec*USEC_TO_SEC + diff.tv_usec;
}

void BORG_Timer_now(struct timeval* time) {
	gettimeofday(time, NULL);
}

void BORG_Timer_start(const char* type) {
	if (timing) {
		BORG_Timer_now(&BORG_Timer_startval);
	}
}

long BORG_Timer_stop(const char* type) {
	long time = 0;

	if (timing) {
		BORG_Timer_now(&BORG_Timer_endval);
		time = BORG_Timer_diff(&BORG_Timer_startval, &BORG_Timer_endval);
		fprintf(timing, "%s: %ld\n", type, time);
	}

	return time;
}

void BORG_Timer_open() {
	if (timingFilename) {
		int filenameLength = strlen(timingFilename);
		char filename[filenameLength+10];

		sprintf(filename, timingFilename, island, islandRank);

		timing = fopen(filename, "w");

		if (!timing) {
			BORG_Debug("Unable to open timing output file\n");
		}

		fprintf(timing, "# Name: %s\n", name);
		fprintf(timing, "# World Rank: %d\n", worldRank);
		fprintf(timing, "# World Size: %d\n", worldSize);

		if (numberOfIslands > 1) {
			fprintf(timing, "# Number of Islands: %d\n", numberOfIslands);
			fprintf(timing, "# Island: %d\n", island);
			fprintf(timing, "# Island Rank: %d\n", islandRank);
			fprintf(timing, "# Island Size: %d\n", islandSize);
		}
	}

	BORG_Timer_now(&start);
}

void BORG_Timer_close() {
	if (timing) {
		struct timeval now;
		BORG_Timer_now(&now);
		fprintf(timing, "elapsed: %ld\n", BORG_Timer_diff(&start, &now));

		fclose(timing);
	}
}

/* MPI message tags */
#define TAG_EVALUATE 0
#define TAG_RESULT 1
#define TAG_SHUTDOWN 2
#define TAG_UPDATE 3
#define TAG_HELP 4
#define TAG_GUIDANCE 5
#define TAG_UPDATE_AND_SHUTDOWN 6
#define TAG_STARTUP 7
#define TAG_INITIALIZATION 8

void BORG_Random_shuffle(double* array, int size) {
	BORG_Validate_pointer(array);
	BORG_Validate_positive(size);

	int i;
	int j;
	double temp;

	for (i=size-1; i>=1; i--) {
		j = BORG_Random_int(i+1);

		if (i != j) {
			temp = array[i];
			array[i] = array[j];
			array[j] = temp;
		}
	}
}

void BORG_Master_initialize_uniform(BORG_Problem problem, int size) {
	BORG_Validate_pointer(problem);
	BORG_Validate_positive(size);

	int i;
	BORG_Solution solution;

	for (i=0; i<size; i++) {
		solution = BORG_Solution_create(problem);
		BORG_Solution_initialize(solution);
		BORG_Queue_push(solution);
	}
}

void BORG_Master_initialize_latin(BORG_Problem problem, int size) {
	BORG_Validate_pointer(problem);
	BORG_Validate_positive(size);

	int i;
	int j;
	BORG_Solution solutions[size];
	double samples[size];
	int numberOfVariables = problem->numberOfVariables;
	double* lowerBounds = problem->lowerBounds;
	double* upperBounds = problem->upperBounds;
	double d = 1.0 / size;

	for (j=0; j<size; j++) {
		solutions[j] = BORG_Solution_create(problem);
	}

	for (i=0; i<numberOfVariables; i++) {
		for (j=0; j<size; j++) {
			samples[j] = lowerBounds[i] + (upperBounds[i]-lowerBounds[i])*BORG_Random_uniform(j*d, (j+1)*d);
		}

		BORG_Random_shuffle(samples, size);

		for (j=0; j<size; j++) {
			solutions[j]->variables[i] = samples[j];
		}
	}

	for (j=0; j<size; j++) {
		BORG_Queue_push(solutions[j]);
	}
}

void BORG_Master_initialize_global(int size) {
	BORG_Debug("Requesting %d initial solutions from controller\n", size);

	int i;
	int rc;
	int nvars = problem->numberOfVariables;
	MPI_Status status;

	rc = MPI_Send(&size, 1, MPI_INTEGER, 0, TAG_STARTUP, MPI_COMM_WORLD);

	if (rc != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Send: %s failed to send startup signal\n", name);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	// wait for initialization
	rc = MPI_Probe(0, TAG_INITIALIZATION, MPI_COMM_WORLD, &status);

	if (rc != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Probe: %s failed to receive initialization packet\n", name);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	// determine the initialization packet size
	MPI_Get_count(&status, MPI_DOUBLE, &size);

	// validate initialization packet size
	if (size == MPI_UNDEFINED) {
		fprintf(stderr, "MPI_Probe: %s received initialization packet with an undefined size\n", name);
		MPI_Abort(MPI_COMM_WORLD, rc);
	} else if (size % nvars != 0) {
		fprintf(stderr, "MPI_Probe: %s received initialization packet with an invalid size\n", name);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	// receive the initialization packet
	double* input = (double*)calloc(size, sizeof(double));
	BORG_Validate_malloc(input);

	rc = MPI_Recv(input, size, MPI_DOUBLE, 0, TAG_INITIALIZATION, MPI_COMM_WORLD, &status);

	if (rc != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Recv: %s failed to receive initialization packet\n", name);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

	// split into solutions
	int count = size / nvars;

	for (i=0; i<count; i++) {
		BORG_Solution solution = BORG_Solution_deserialize(&input[i * nvars], 0);
		BORG_Queue_push(solution);
	}

	// clean up
	free(input);

	BORG_Debug("Received %d initial solution from controller\n", count);
}

BORG_Solution BORG_Master_next(BORG_Algorithm algorithm) {
	BORG_Validate_pointer(algorithm);

	BORG_Solution result = BORG_Queue_pop();

	if (result) {
		/* use cached solution */
	} else if (algorithm->population->size == 0) {
		if (initialization == INITIALIZATION_UNIFORM) {
			BORG_Master_initialize_uniform(algorithm->problem, 2*algorithm->population->capacity);
		} else if (initialization == INITIALIZATION_LATIN) {
			BORG_Master_initialize_latin(algorithm->problem, 2*algorithm->population->capacity);
		} else if (initialization == INITIALIZATION_LATIN_GLOBAL) {
			if (numberOfIslands == 1) {
				BORG_Master_initialize_latin(algorithm->problem, 2*algorithm->population->capacity);
			} else {
				BORG_Master_initialize_global(2*algorithm->population->capacity);
			}
		} else {
			BORG_Error("Unknown initialization strategy\n");
		}

		result = BORG_Queue_pop();
	} else if (algorithm->population->size < algorithm->population->capacity) {
		/* this should never occur, but just in case */
		printf("Underfull, size=%d, capacity=%d\n", algorithm->population->size, algorithm->population->capacity);

		result = BORG_Solution_create(algorithm->problem);
		BORG_Solution_initialize(result);
	} else {
		/* generate new solutions via crossover/mutation */
		int i;
		BORG_Population population = algorithm->population;
		BORG_Archive archive = algorithm->archive;
		int operatorIndex = BORG_Algorithm_select(algorithm);
		BORG_Operator variation = algorithm->operators[operatorIndex];
		int numberOfParents = variation->numberOfParents;
		int numberOfOffspring = variation->numberOfOffspring;
		BORG_Solution* parents = (BORG_Solution*)calloc(numberOfParents, sizeof(struct BORG_Solution_t*));
		BORG_Solution* offspring = (BORG_Solution*)calloc(numberOfOffspring, sizeof(struct BORG_Solution_t*));

		BORG_Validate_malloc(parents);
		BORG_Validate_malloc(offspring);
	
		if (archive->size <= 1) {
			BORG_Population_select(population, numberOfParents, parents, algorithm->tournamentSize);
		} else {
			BORG_Population_select(population, numberOfParents-1, parents, algorithm->tournamentSize);
			parents[numberOfParents-1] = BORG_Archive_select(archive);
		}

		BORG_Algorithm_shuffle(numberOfParents, parents);
		BORG_Operator_apply(variation, parents, offspring);

		for (i=0; i<numberOfOffspring; i++) {
			offspring[i]->operatorIndex = operatorIndex;
			BORG_Queue_push(offspring[i]);
		}

		free(parents);
		free(offspring);
		
		result = BORG_Queue_pop();
	}

	return result;
}

void BORG_Master_shutdown(int rank) {
	BORG_Validate_positive(rank);

	BORG_Debug("Sending shutdown signal to worker %d\n", rank);

	int rc = MPI_Send(NULL, 0, MPI_DOUBLE, rank, TAG_SHUTDOWN, islandComm);

	if (rc != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Send: %s failed sending shutdown signal to worker %d\n", name, rank);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
}

void BORG_Master_send(int rank, BORG_Solution solution) {
	BORG_Validate_positive(rank);
	BORG_Validate_pointer(solution);

	BORG_Debug("Sending solution to worker %d\n", rank);

	processing[rank-1] = solution;

#ifdef BORG_MPI_NONBLOCK	
	int rc = MPI_Isend(solution->variables, solution->problem->numberOfVariables,
			MPI_DOUBLE, rank, TAG_EVALUATE, islandComm, &requests[rank-1]);
#else
	int rc = MPI_Send(solution->variables, solution->problem->numberOfVariables,
			MPI_DOUBLE, rank, TAG_EVALUATE, islandComm);
#endif

	if (rc != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Send: %s failed sending variables to worker %d\n", name, rank);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
}

int BORG_Master_receive() {
	int rc;
	int nobjs = problem->numberOfObjectives;
	int ncons = problem->numberOfConstraints;
	MPI_Status status;
	double* message = (double*)calloc(nobjs+ncons, sizeof(double));

	BORG_Validate_malloc(message);

	rc = MPI_Recv(message, nobjs+ncons, MPI_DOUBLE, MPI_ANY_SOURCE,
			TAG_RESULT, islandComm, &status);

	if (rc != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Recv: %s failed receiving result\n", name);
		MPI_Abort(MPI_COMM_WORLD, rc);
	}

#ifdef BORG_MPI_NONBLOCK
	/* frees the non-blocking send request */
	MPI_Wait(&requests[status.MPI_SOURCE-1], MPI_STATUS_IGNORE);
#endif

	BORG_Solution solution = processing[status.MPI_SOURCE-1];
	memcpy(solution->objectives, message, nobjs*sizeof(double));

	if (ncons > 0) {
		memcpy(solution->constraints, message+nobjs, ncons*sizeof(double));
	}

	BORG_Debug("Received result from worker %d\n", status.MPI_SOURCE);

	free(message);
	return status.MPI_SOURCE;
}

void BORG_Master_injection(BORG_Algorithm algorithm, int size) {
	BORG_Validate_pointer(algorithm);

	int i;
	int remaining = size;
	BORG_Archive archive = algorithm->archive;
	BORG_Operator injectionOperator = BORG_Operator_create("UM", 1, 1, 1, BORG_Operator_UM);
	int numberOfParents = injectionOperator->numberOfParents;
	int numberOfOffspring = injectionOperator->numberOfOffspring;
	BORG_Solution* parents = (BORG_Solution*)calloc(numberOfParents, sizeof(struct BORG_Solution_t*));
	BORG_Solution* offspring = (BORG_Solution*)calloc(numberOfOffspring, sizeof(struct BORG_Solution_t*));

	BORG_Validate_malloc(parents);
	BORG_Validate_malloc(offspring);

	while (remaining > 0) {
		if (algorithm->restartMode == RESTART_DEFAULT) {
			BORG_Operator_set_parameter(injectionOperator, 0, 1.0 / algorithm->problem->numberOfVariables);
		} else if (algorithm->restartMode == RESTART_RANDOM) {
			BORG_Operator_set_parameter(injectionOperator, 0, 1.0);
		} else if (algorithm->restartMode == RESTART_RAMPED) {
			BORG_Operator_set_parameter(injectionOperator, 0, remaining / (double)size);
		} else if (algorithm->restartMode == RESTART_ADAPTIVE) {
			double d = 0.0;

			if (algorithm->maxMutationIndex > 0) {
				d = algorithm->baseMutationIndex/(double)algorithm->maxMutationIndex;
			}

			double rate = d + (1.0 - d)/algorithm->problem->numberOfVariables;

				BORG_Operator_set_parameter(injectionOperator, 0, rate);
		} else if (algorithm->restartMode == RESTART_INVERTED) {
			double d = 0.0;

			if (algorithm->maxMutationIndex > 0) {
				d = (algorithm->maxMutationIndex-algorithm->baseMutationIndex)/(double)algorithm->maxMutationIndex;
			}

			double rate = d + (1.0 - d)/algorithm->problem->numberOfVariables;

			BORG_Operator_set_parameter(injectionOperator, 0, rate);
		} else {
			BORG_Error("unknown restart mode\n");
		}

		for (i=0; i<numberOfParents; i++) {
			parents[i] = BORG_Archive_select(archive);
		}

		BORG_Operator_apply(injectionOperator, parents, offspring);

		for (i=0; i<numberOfOffspring; i++) {
			offspring[i]->operatorIndex = -1;
			BORG_Queue_push(offspring[i]);
			remaining--;
		}
	}

	BORG_Operator_destroy(injectionOperator);
	free(parents);
	free(offspring);
}

void BORG_Master_restart(BORG_Algorithm algorithm) {
	BORG_Validate_pointer(algorithm);


	BORG_Population population = algorithm->population;
	BORG_Archive archive = algorithm->archive;
	int newPopulationSize = (int)(algorithm->populationRatio * archive->size);

	if (newPopulationSize < algorithm->minimumPopulationSize) {
		newPopulationSize = algorithm->minimumPopulationSize;
	} else if (newPopulationSize > algorithm->maximumPopulationSize) {
		newPopulationSize = algorithm->maximumPopulationSize;
	}

	BORG_Debug("Restart (%d NFE) - New Population Size: %d\n",
		algorithm->numberOfEvaluations, newPopulationSize);

	/* clear and resize population */
	BORG_Population_reset(population, newPopulationSize);

	/* fill population with all individuals from the archive */
	BORG_Entry entry = archive->head;

	while (entry) {
		BORG_Population_add(population, entry->solution);
		entry = entry->next;
		newPopulationSize--;
	}

	/* inject remaining individuals */
	BORG_Master_injection(algorithm, newPopulationSize);

	/* adjust selection pressure */
	algorithm->tournamentSize = (int)(algorithm->selectionRatio * population->capacity);

	if (algorithm->tournamentSize < 2) {
		algorithm->tournamentSize = 2;
	}

	algorithm->evaluationsAtLastRestart = algorithm->numberOfEvaluations;
}

void BORG_Master_initialize(BORG_Algorithm algorithm) {
	BORG_Timer_start("initializing");
	BORG_Validate_pointer(algorithm);
	BORG_Algorithm_validate(algorithm);
	BORG_Algorithm_update(algorithm);


	int i;
	BORG_Population population = algorithm->population;

	if (population->capacity != algorithm->initialPopulationSize) {
		BORG_Population_reset(population, algorithm->initialPopulationSize);
	}

	if (islandSize > 1) {
		processing = (BORG_Solution*)calloc(islandSize-1, sizeof(BORG_Solution));
	} else {
		/* hack to allow run with no worker nodes */
		processing = (BORG_Solution*)calloc(1, sizeof(BORG_Solution));
	}

	BORG_Validate_malloc(processing);

#ifdef BORG_MPI_NONBLOCK
	requests = (MPI_Request*)calloc(size-1, sizeof(MPI_Request));
	BORG_Validate_malloc(requests);
#endif

	if (islandSize > 1) {
		for (i=1; i<islandSize; i++) {
			BORG_Solution solution = BORG_Master_next(algorithm);
			BORG_Master_send(i, solution);
		}
	} else {
		/* hack to allow run with no worker nodes */
		processing[0] = BORG_Master_next(algorithm);
		problem->function(processing[0]->variables, processing[0]->objectives,
				processing[0]->constraints);
	}

	BORG_Timer_stop("initializing");
}

void BORG_Master_output_header(BORG_Algorithm algorithm, FILE* file) {
	int i;


	if (aerovisFormat) {
		fprintf(file, "# Nondominated Solutions:\n");
		fprintf(file, "# Format:  Variables = %d | Objectives = %d\n",
				problem->numberOfVariables, problem->numberOfObjectives);
		fprintf(file, "# <DATA_HEADER> ");

		for (i=0; i<problem->numberOfVariables; i++) {
			if (i > 0) {
				fprintf(file, ", ");
			}

			fprintf(file, "DV%d", i+1);
		}

		for (i=0; i<problem->numberOfObjectives; i++) {
			if ((i > 0) || (problem->numberOfVariables > 0)) {
				fprintf(file, ", ");
			}

			if (problem->names[i]) {
				fprintf(file, "%s", problem->names[i]);
			} else {
				fprintf(file, "OBJ%d", i+1);
			}
		}

		fprintf(file, "\n# <GEN_HEADER> NFE, Time (sec)");

		for (i=0; i<algorithm->numberOfOperators; i++) {
			fprintf(file, ", %s", algorithm->operators[i]->name);
		}

		fprintf(file, "\n#\n");
	}
}

void BORG_Master_output_entry(BORG_Algorithm algorithm, double elapsedSeconds, BORG_Archive archive, FILE* file) {
	int i;

	if (aerovisFormat) {
		/* print the generation header */
		fprintf(file, "%d %f", algorithm->numberOfEvaluations, elapsedSeconds);

		for (i=0; i<algorithm->numberOfOperators; i++) {
			fprintf(file, " %lg", algorithm->operators[i]->selectionProbability);
		}

		fprintf(file, "\n#\n");
	} else {
		fprintf(file, "//NFE=%d\n", algorithm->numberOfEvaluations);
		fprintf(file, "//ElapsedTime=%f\n", elapsedSeconds);

		for (i=0; i<algorithm->numberOfOperators; i++) {
			fprintf(file, "//%s=%lg\n", algorithm->operators[i]->name,
					algorithm->operators[i]->selectionProbability);
		}

		fprintf(file, "//Improvements=%d\n", BORG_Algorithm_get_number_improvements(algorithm));
				fprintf(file, "//Restarts=%d\n", BORG_Algorithm_get_number_restarts(algorithm));
				fprintf(file, "//PopulationSize=%d\n", BORG_Algorithm_get_population_size(algorithm));
				fprintf(file, "//ArchiveSize=%d\n", BORG_Algorithm_get_archive_size(algorithm));

		if (algorithm->restartMode == RESTART_ADAPTIVE) {
					fprintf(file, "//MutationIndex=%d\n", BORG_Algorithm_get_mutation_index(algorithm));
				}

		if (numberOfIslands > 1) {
			fprintf(file, "//HelpRequests=%d\n", helpRequests);
		}
	}

	/* print the current archive (ends with # automatically) */
	BORG_Archive_append(archive, file);
}

void BORG_Master_loop(BORG_Algorithm algorithm) {
	BORG_Validate_pointer(algorithm);

	int i;
	int rc;
	int rc2;
	int remaining = islandSize-1;
	MPI_Status status;
	BORG_Problem problem = algorithm->problem;
	BORG_Population population = algorithm->population;
	BORG_Archive archive = algorithm->archive;
	int nvars = problem->numberOfVariables;
	int nobjs = problem->numberOfObjectives;
	int ncons = problem->numberOfConstraints;
	FILE* runtime = NULL;
	int lastUpdate = 0;
	int lastSnapshot = 0;
	struct timeval now;
	double elapsedSeconds = 0.0;
	double elapsedHours = 0.0;
	int helpDelay = 0;

	if (runtimeFilename) {
		int filenameLength = strlen(runtimeFilename);
		char filename[filenameLength+10];
		sprintf(filename, runtimeFilename, island);

		runtime = fopen(filename, "w");
		
		if (!runtime) {
			BORG_Debug("Unable to open runtime output file\n");
		}

		BORG_Master_output_header(algorithm, runtime);
	}

	if (islandSize == 1) {
		/* hack to allow run with no worker nodes */
		processing[0] = BORG_Master_next(algorithm);
		problem->function(processing[0]->variables, processing[0]->objectives,
				processing[0]->constraints);
	}

	while ((remaining > 0) || 
			(((maxEvaluations <= 0) || (algorithm->numberOfEvaluations < maxEvaluations)) &&
			((maxHours <= 0.0) || (elapsedHours < maxHours)))) {
		/* convert elapsed time to seconds and hours for output and termination purposes */
		BORG_Timer_now(&now);
		elapsedSeconds = BORG_Timer_diff(&start, &now)/(double)USEC_TO_SEC;
		elapsedHours = elapsedSeconds/3600.0;
		
		/* receive evaluated solution */
		if (islandSize > 1) {
			/* probe is only needed to measure idle time */
			BORG_Timer_start("wait");
			MPI_Probe(MPI_ANY_SOURCE, TAG_RESULT, islandComm, &status);
			BORG_Timer_stop("wait");

			BORG_Timer_start("recv");
			rc = BORG_Master_receive();
			BORG_Timer_stop("recv");
		} else {
			/* hack to allow run with no worker nodes */
			rc = 1;
		}

		BORG_Timer_start("algo");
		remaining--;
		algorithm->numberOfEvaluations++;
		BORG_Debug("%d of %d evaluations complete\n", algorithm->numberOfEvaluations, maxEvaluations);

		BORG_Solution solution = processing[rc-1];
		BORG_Population_add(population, solution);
		BORG_Archive_add(archive, solution);
		BORG_Solution_destroy(solution);

		/* record runtime dynamics */
		if (runtime && (algorithm->numberOfEvaluations-lastSnapshot >= runtimeFrequency)) {
			BORG_Master_output_entry(algorithm, elapsedSeconds, archive, runtime);
			lastSnapshot = algorithm->numberOfEvaluations;
		}

		/* check if a restart is necessary */
		if ((algorithm->numberOfEvaluations-algorithm->evaluationsAtLastCheck) >= algorithm->windowSize) {
			/* wait until queue is empty before restarting */
			if (queue == NULL) {
				/* send update to controller */
				if ((numberOfIslands > 1) && (algorithm->numberOfEvaluations-lastUpdate >= updateFrequency)) {
					int count = BORG_Archive_get_size(archive);
					int size = count * (nvars + nobjs + ncons + 1);

					// prepare the update packet
					double* output = (double*)calloc(size, sizeof(double));

					for (i=0; i<count; i++) {
						BORG_Solution solution = BORG_Archive_get(archive, i);
						BORG_Solution_serialize(solution, &output[i * (nvars + nobjs + ncons + 1)], 1);
					}

					// send the update packet
					rc2 = MPI_Send(output, size, MPI_DOUBLE, 0, TAG_UPDATE, MPI_COMM_WORLD);

					if (rc2 != MPI_SUCCESS) {
						fprintf(stderr, "MPI_Send: %s failed to send update packet\n", name);
						MPI_Abort(MPI_COMM_WORLD, rc2);
					}

					// clean up
					free(output);
					lastUpdate = algorithm->numberOfEvaluations;
				}

				/* check if restart is necessary */
				if (BORG_Algorithm_check(algorithm)) {
					if ((numberOfIslands > 1) && (helpDelay >= delayForHelp)) {
						helpRequests++;

						// ask for help
						rc2 = MPI_Send(NULL, 0, MPI_DOUBLE, 0, TAG_HELP, MPI_COMM_WORLD);

						if (rc2 != MPI_SUCCESS) {
							fprintf(stderr, "MPI_Send: %s failed to send help signal\n", name);
							MPI_Abort(MPI_COMM_WORLD, rc2);
						}

						// wait for guidance
						rc2 = MPI_Probe(0, TAG_GUIDANCE, MPI_COMM_WORLD, &status);

						if (rc2 != MPI_SUCCESS) {
							fprintf(stderr, "MPI_Probe: %s failed to receive guidance packet\n", name);
							MPI_Abort(MPI_COMM_WORLD, rc2);
						}

						// determine the guidance packet size
						int size;
						MPI_Get_count(&status, MPI_DOUBLE, &size);

						// validate guidance packet size (solutions*(nvars+nobjs+ncons+1))
						if (size == MPI_UNDEFINED) {
							fprintf(stderr, "MPI_Probe: %s received guidance packet with an undefined size\n", name);
							MPI_Abort(MPI_COMM_WORLD, rc2);
						} else if (size % (nvars + nobjs + ncons + 1) != 0) {
							fprintf(stderr, "MPI_Probe: %s received guidance packet with an invalid size\n", name);
							MPI_Abort(MPI_COMM_WORLD, rc2);
						}

						// receive the guidance packet
						double* input = (double*)calloc(size, sizeof(double));
						BORG_Validate_malloc(input);

						rc2 = MPI_Recv(input, size, MPI_DOUBLE, 0, TAG_GUIDANCE, MPI_COMM_WORLD, &status);

						if (rc2 != MPI_SUCCESS) {
							fprintf(stderr, "MPI_Recv: %s failed to receive guidance packet\n", name);
							MPI_Abort(MPI_COMM_WORLD, rc2);
						}

						// split into solutions
						int count = size / (nvars + nobjs + ncons + 1);

						for (i=0; i<count; i++) {
							BORG_Solution solution = BORG_Solution_deserialize(&input[i * (nvars + nobjs + ncons + 1)], 1);
							BORG_Archive_add(archive, solution);
							BORG_Solution_destroy(solution);
						}

						// clean up
						free(input);
						helpDelay = 0;
						algorithm->restartedLastCheck = 0;
						algorithm->baseMutationIndex = 0;
					}

					if (algorithm->restartedLastCheck) {
						algorithm->baseMutationIndex++;

						if (algorithm->baseMutationIndex > algorithm->maxMutationIndex) {
							algorithm->baseMutationIndex = algorithm->maxMutationIndex;
							helpDelay++;
						} else {
							helpDelay = 0;
						}
					}

					BORG_Master_restart(algorithm);
					algorithm->restartedLastCheck = 1;
				} else {
					if (algorithm->restartedLastCheck) {
						algorithm->baseMutationIndex--;

						if (algorithm->baseMutationIndex < 0) {
							algorithm->baseMutationIndex = 0;
						}
					}

					helpDelay = 0;
					algorithm->restartedLastCheck = 0;
				}

				algorithm->improvementsAtLastCheck = archive->numberOfImprovements;
				algorithm->evaluationsAtLastCheck = algorithm->numberOfEvaluations +
						(population->capacity-population->size);

			}
		}

		/* send out next solution for evaluation */
		if (((maxEvaluations <= 0) || (algorithm->numberOfEvaluations <= maxEvaluations-(islandSize-1))) &&
				((maxHours <= 0.0) || (elapsedHours < maxHours))) {
			if (islandSize > 1) {
				BORG_Solution next = BORG_Master_next(algorithm);
				BORG_Timer_stop("algo");

				BORG_Timer_start("send");
				BORG_Master_send(rc, next);
				BORG_Timer_stop("send");
			} else {
				/* hack to allow run with no worker nodes */
				processing[0] = BORG_Master_next(algorithm);
				BORG_Timer_stop("algo");

				BORG_Timer_start("eval");
				problem->function(processing[0]->variables, processing[0]->objectives,
						processing[0]->constraints);
				BORG_Timer_stop("eval");
			}
	
			remaining++;
		} else {
			BORG_Timer_stop("algo");
		}
	}

	BORG_Timer_start("shutdown");

	/* shut down the workers */
	for (i = 1; i < islandSize; i++) {
		BORG_Master_shutdown(i);
	}

	/* send update and shutdown notification to controller */
	if (numberOfIslands > 1) {
		int count = BORG_Archive_get_size(archive);
		int size = count * (nvars + nobjs + ncons + 1);

		// prepare the update packet
		double* output = (double*)calloc(size, sizeof(double));

		for (i=0; i<count; i++) {
			BORG_Solution solution = BORG_Archive_get(archive, i);
			BORG_Solution_serialize(solution, &output[i * (nvars + nobjs + ncons + 1)], 1);
		}

		// send the update packet
		rc = MPI_Send(output, size, MPI_DOUBLE, 0, TAG_UPDATE_AND_SHUTDOWN, MPI_COMM_WORLD);

		if (rc != MPI_SUCCESS) {
			fprintf(stderr, "MPI_Send: %s failed to send update packet\n", name);
			MPI_Abort(MPI_COMM_WORLD, rc);
		}

		// clean up
		free(output);
	}

	/* record runtime dynamics */
	if (runtime) {
		if (lastSnapshot < algorithm->numberOfEvaluations) {
			BORG_Timer_now(&now);
			elapsedSeconds = BORG_Timer_diff(&start, &now)/(double)USEC_TO_SEC;

			BORG_Master_output_entry(algorithm, elapsedSeconds, archive, runtime);
		}

		fclose(runtime);
	}

	BORG_Timer_stop("shutdown");
}

BORG_Archive BORG_Master_run() {
	BORG_Operator pm = BORG_Operator_create("PM", 1, 1, 2, BORG_Operator_PM);
	BORG_Operator_set_parameter(pm, 0, 1.0/problem->numberOfVariables);
	BORG_Operator_set_parameter(pm, 1, 20.0);

	BORG_Operator sbx = BORG_Operator_create("SBX", 2, 2, 2, BORG_Operator_SBX);
	BORG_Operator_set_parameter(sbx, 0, 1.0);
	BORG_Operator_set_parameter(sbx, 1, 15.0);

	BORG_Operator de = BORG_Operator_create("DE", 4, 1, 2, BORG_Operator_DE);
	BORG_Operator_set_parameter(de, 0, 0.1);
	BORG_Operator_set_parameter(de, 1, 0.5);
	BORG_Operator_set_mutation(de, pm);

	BORG_Operator um = BORG_Operator_create("UM", 1, 1, 1, BORG_Operator_UM);
	BORG_Operator_set_parameter(um, 0, 1.0 / problem->numberOfVariables);

	BORG_Operator spx = BORG_Operator_create("SPX", 10, 2, 1, BORG_Operator_SPX);
	BORG_Operator_set_parameter(spx, 0, 3.0);

	BORG_Operator pcx = BORG_Operator_create("PCX", 10, 2, 2, BORG_Operator_PCX);
	BORG_Operator_set_parameter(pcx, 0, 0.1);
	BORG_Operator_set_parameter(pcx, 1, 0.1);

	BORG_Operator undx = BORG_Operator_create("UNDX", 10, 2, 2, BORG_Operator_UNDX);
	BORG_Operator_set_parameter(undx, 0, 0.5);
	BORG_Operator_set_parameter(undx, 1, 0.35);

	BORG_Algorithm algorithm = BORG_Algorithm_create(problem, 6);
	BORG_Algorithm_set_operator(algorithm, 0, sbx);
	BORG_Algorithm_set_operator(algorithm, 1, de);
	BORG_Algorithm_set_operator(algorithm, 2, pcx);
	BORG_Algorithm_set_operator(algorithm, 3, spx);
	BORG_Algorithm_set_operator(algorithm, 4, undx);
	BORG_Algorithm_set_operator(algorithm, 5, um);

	BORG_Algorithm_set_minimum_population_size(algorithm, islandSize-1 > 100 ? islandSize-1 : 100);
	BORG_Algorithm_set_initial_population_size(algorithm, islandSize-1 > 100 ? islandSize-1 : 100);
	BORG_Algorithm_set_restart_mode(algorithm, RESTART_ADAPTIVE);
	BORG_Algorithm_set_max_mutation_index(algorithm, 5);
	BORG_Algorithm_set_probability_mode(algorithm, PROBABILITIES_BOTH);

	BORG_Master_initialize(algorithm);
	BORG_Master_loop(algorithm);

	BORG_Archive result = BORG_Algorithm_get_result(algorithm);

	BORG_Operator_destroy(sbx);
	BORG_Operator_destroy(de);
	BORG_Operator_destroy(pm);
	BORG_Operator_destroy(um);
	BORG_Operator_destroy(spx);
	BORG_Operator_destroy(pcx);
	BORG_Operator_destroy(undx);
	BORG_Algorithm_destroy(algorithm);
	free(processing);

	while (queue) {
		BORG_Solution_destroy(BORG_Queue_pop());
	}

#ifdef BORG_MPI_NONBLOCK
	free(requests);
#endif

	return result;
}

void BORG_Solution_serialize(BORG_Solution solution, double* output, int full) {
	BORG_Validate_pointer(solution);
	BORG_Validate_pointer(output);

	int i;
	int index = 0;
	int nvars = problem->numberOfVariables;
	int nobjs = problem->numberOfObjectives;
	int ncons = problem->numberOfConstraints;

	for (i=0; i<nvars; i++) {
		output[index++] = solution->variables[i];
	}

	if (full) {
		for (i=0; i<nobjs; i++) {
			output[index++] = solution->objectives[i];
		}

		for (i=0; i<ncons; i++) {
			output[index++] = solution->constraints[i];
		}

		output[index++] = solution->operatorIndex;
	}
}

BORG_Solution BORG_Solution_deserialize(double* input, int full) {
	BORG_Validate_pointer(input);

	int i;
	int index = 0;
	int nvars = problem->numberOfVariables;
	int nobjs = problem->numberOfObjectives;
	int ncons = problem->numberOfConstraints;
	BORG_Solution solution = BORG_Solution_create(problem);

	for (i=0; i<nvars; i++) {
		solution->variables[i] = input[index++];
	}

	if (full) {
		for (i=0; i<nobjs; i++) {
			solution->objectives[i] = input[index++];
		}

		for (i=0; i<ncons; i++) {
			solution->constraints[i] = input[index++];
		}

		solution->operatorIndex = (int)input[index++];
	}

	return solution;
}

BORG_Archive BORG_Controller_run() {
	int i;
	int j;
	int rc;
	int size = 0;
	int remaining = numberOfIslands;
	int nvars = problem->numberOfVariables;
	int nobjs = problem->numberOfObjectives;
	int ncons = problem->numberOfConstraints;
	BORG_Archive archive = BORG_Archive_create();
	MPI_Status status;

	/* the initialization loop */
	if (initialization == INITIALIZATION_LATIN_GLOBAL) {
		BORG_Timer_start("initialization");
		int count[numberOfIslands];
		int source[numberOfIslands];

		for (i=0; i<numberOfIslands; i++) {
			rc = MPI_Recv(&j, 1, MPI_INTEGER, MPI_ANY_SOURCE, TAG_STARTUP, MPI_COMM_WORLD, &status);

			if (rc != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Recv: %s failed to receive startup signal\n", name);
				MPI_Abort(MPI_COMM_WORLD, rc);
			}

			count[i] = j;
			source[i] = status.MPI_SOURCE;
			size += j;
		}

		BORG_Master_initialize_latin(problem, size);
	
		for (i=0; i<numberOfIslands; i++) {
			size = count[i] * nvars;
			double* output = (double*)calloc(size, sizeof(double));

			for (j=0; j<count[i]; j++) {
				BORG_Solution solution = BORG_Queue_pop();
				BORG_Solution_serialize(solution, &output[j * nvars], 0);
			}

			rc = MPI_Send(output, size, MPI_DOUBLE, source[i], TAG_INITIALIZATION, MPI_COMM_WORLD);

			if (rc != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send: %s failed to send initialization packet\n", name);
				MPI_Abort(MPI_COMM_WORLD, rc);
			}

			// clean up
			free(output);
		}
		BORG_Timer_stop("initialization");
	}

	/* the main controller loop */
	while (remaining > 0) {
		BORG_Timer_start("wait");
		rc = MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
		BORG_Timer_stop("wait");

		if (rc != MPI_SUCCESS) {
			fprintf(stderr, "MPI_Probe: %s failed\n", name);
			MPI_Abort(MPI_COMM_WORLD, rc);
		} else if ((status.MPI_TAG == TAG_UPDATE) || (status.MPI_TAG == TAG_UPDATE_AND_SHUTDOWN)) {
			BORG_Timer_start("update");

			// determine the update packet size
			MPI_Get_count(&status, MPI_DOUBLE, &size);

			// validate update packet size (solutions*(nvars+nobjs+ncons+1))
			if (size == MPI_UNDEFINED) {
				fprintf(stderr, "MPI_Probe: %s received update packet with an undefined size\n", name);
				MPI_Abort(MPI_COMM_WORLD, rc);
			} else if (size % (nvars + nobjs + ncons + 1) != 0) {
				fprintf(stderr, "MPI_Probe: %s received update packet with an invalid size\n", name);
				MPI_Abort(MPI_COMM_WORLD, rc);
			}

			// receive the update packet
			double* input = (double*)calloc(size, sizeof(double));
			BORG_Validate_malloc(input);

			rc = MPI_Recv(input, size, MPI_DOUBLE, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

			if (rc != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Recv: %s failed to receive update packet\n", name);
				MPI_Abort(MPI_COMM_WORLD, rc);
			}

			// split into solutions
			int count = size / (nvars + nobjs + ncons + 1);

			for (i=0; i<count; i++) {
				BORG_Solution solution = BORG_Solution_deserialize(&input[i * (nvars + nobjs + ncons + 1)], 1);
				BORG_Archive_add(archive, solution);
				BORG_Solution_destroy(solution);
			}

			// clean up
			free(input);
			BORG_Timer_stop("update");
		} else if (status.MPI_TAG == TAG_HELP) {
			BORG_Timer_start("help");

			// receive the help message to clear the queue
			rc = MPI_Recv(NULL, 0, MPI_DOUBLE, status.MPI_SOURCE, TAG_HELP, MPI_COMM_WORLD, &status);

			if (rc != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Recv: %s failed to receive help packet\n", name);
				MPI_Abort(MPI_COMM_WORLD, rc);
			}

			int count = BORG_Archive_get_size(archive);
			size = count * (nvars + nobjs + ncons + 1);

			// prepare the guidance packet
			double* output = (double*)calloc(size, sizeof(double));

			for (i=0; i<count; i++) {
				BORG_Solution solution = BORG_Archive_get(archive, i);
				BORG_Solution_serialize(solution, &output[i * (nvars + nobjs + ncons + 1)], 1);
			}

			// send the guidance packet
			rc = MPI_Send(output, size, MPI_DOUBLE, status.MPI_SOURCE, TAG_GUIDANCE, MPI_COMM_WORLD);

			if (rc != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send: %s failed to send guidance packet\n", name);
				MPI_Abort(MPI_COMM_WORLD, rc);
			}

			// clean up
			free(output);
			BORG_Timer_stop("help");
		} else if (status.MPI_TAG == TAG_STARTUP) {
			fprintf(stderr, "MPI_Probe: %s received unexpected startup tag, ensure initial population size is at least the processor count\n", name);
			MPI_Abort(MPI_COMM_WORLD, rc);
		} else {
			fprintf(stderr, "MPI_Probe: %s received a message with an unknown tag: %d\n", name, status.MPI_TAG);
			MPI_Abort(MPI_COMM_WORLD, rc);
		}

		if (status.MPI_TAG == TAG_UPDATE_AND_SHUTDOWN) {
			remaining--;
		}
	}

	return archive;
}

void BORG_Worker_run() {
	int rc;
	int nvars = problem->numberOfVariables;
	int nobjs = problem->numberOfObjectives;
	int ncons = problem->numberOfConstraints;
	MPI_Status status;
	double* input = (double*)calloc(nvars, sizeof(double));
	double* output = (double*)calloc(nobjs+ncons, sizeof(double));

	BORG_Validate_malloc(input);
	BORG_Validate_malloc(output);

	/* process jobs */
	while (1) {
		BORG_Timer_start("wait");
		MPI_Probe(0, MPI_ANY_TAG, islandComm, &status);
		BORG_Timer_stop("wait");

		BORG_Timer_start("recv");
		rc = MPI_Recv(input, nvars, MPI_DOUBLE, 0, MPI_ANY_TAG, islandComm, &status);
		BORG_Timer_stop("recv");

		if (rc != MPI_SUCCESS) {
			fprintf(stderr, "MPI_Recv: %s failed receiving variables\n", name);
			MPI_Abort(MPI_COMM_WORLD, rc);
		} else if (status.MPI_TAG == TAG_SHUTDOWN) {
			BORG_Debug("Received shutdown signal\n");
			break;
		} else {
			BORG_Timer_start("eval");
			problem->function(input, output, ncons == 0 ? NULL : output+nobjs);
			BORG_Timer_stop("eval");

			BORG_Timer_start("send");
			rc = MPI_Send(output, nobjs+ncons, MPI_DOUBLE, 0, TAG_RESULT, islandComm);
			BORG_Timer_stop("send");

			if (rc != MPI_SUCCESS) {
				fprintf(stderr, "MPI_Send: %s failed to send objectives and constraints\n", name);
				MPI_Abort(MPI_COMM_WORLD, rc);
			}
		}
	}

	free(input);
	free(output);
}

void BORG_Algorithm_ms_startup(int* argc, char*** argv) {
	int rc;

	if ((rc = MPI_Init(argc, argv)) != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Init: error initializing MPI program\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}	

	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);
	MPI_Barrier(MPI_COMM_WORLD);
}

BORG_Archive BORG_Algorithm_ms_run(BORG_Problem _problem) {
	BORG_Validate_pointer(_problem);

	int rc;
	BORG_Archive result = NULL;
	problem = _problem;
	helpRequests = 0;

	/* ensure valid termination condition */
	if ((maxEvaluations <= 0) && (maxHours <= 0.0)) {
		fprintf(stderr, "BORG_Algorithm_ms_run: set the max evaluations and/or max time allocated");
		MPI_Abort(MPI_COMM_WORLD, -1);
	}

	/* split into islands, if necessary */
	if (numberOfIslands == 1) {
		island = 0;
		islandSize = worldSize;
		islandRank = worldRank;
		islandComm = MPI_COMM_WORLD;
	} else {
		if (worldRank == 0) {
			/* the controller gets put into a solitary island for the MPI_Comm_split method to work */
			island = -1;
		} else {
			/* determine the island and rank for this node */
			island = ((worldRank-1) / ((worldSize-1) / numberOfIslands));

			/* balance any remaining nodes among the original islands */
			if (island >= numberOfIslands) {
				island = (islandRank % numberOfIslands);
			}
		}

		/* create communicator for within islands */
		if ((rc = MPI_Comm_split(MPI_COMM_WORLD, island+1, islandRank, &islandComm)) != MPI_SUCCESS) {
			fprintf(stderr, "MPI_Comm_split: failed to create islands\n");
			MPI_Abort(MPI_COMM_WORLD, rc);
		}

		MPI_Comm_size(islandComm, &islandSize);
		MPI_Comm_rank(islandComm, &islandRank);
	}

	/* launch the controller / master / worker node */
	if ((numberOfIslands > 1) && (worldRank == 0)) {
		sprintf(name, "Controller");


		BORG_Debug_set_name(name);
		BORG_Timer_open();
		BORG_Debug("Starting!\n");
		result = BORG_Controller_run();
		BORG_Debug("Exiting!\n");
	} else if (islandRank == 0) {
		if (numberOfIslands == 1) {	
			sprintf(name, "Master");

		} else {
			sprintf(name, "Master (%d)", island);
		}

		BORG_Debug_set_name(name);
		BORG_Timer_open();
		BORG_Debug("Starting!\n");
		result = BORG_Master_run();
		BORG_Debug("Exiting!\n");

		// if multiple islands, the controller returns the result
		if (numberOfIslands > 1) {
			result = NULL;
		}
	} else {
		if (numberOfIslands == 1) {
			sprintf(name, "Worker %d", islandRank);
		} else {
			sprintf(name, "Worker (%d.%d)", island, islandRank);
		}

		BORG_Debug_set_name(name);
		BORG_Timer_open();
		BORG_Debug("Starting!\n");
		BORG_Worker_run();
		BORG_Debug("Exiting!\n");
	}

	BORG_Timer_close();
	MPI_Barrier(MPI_COMM_WORLD);

	return result;
}

void BORG_Algorithm_ms_shutdown() {
	int rc;
	
	MPI_Barrier(MPI_COMM_WORLD);

	if ((rc = MPI_Finalize()) != MPI_SUCCESS) {
		fprintf(stderr, "MPI_Finalize: error finalizing MPI program\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
}

