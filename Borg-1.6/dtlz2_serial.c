/* Copyright 2012-2013 The Pennsylvania State University
 *
 * This software was written by David Hadka and others.
 * 
 * The use, modification and distribution of this software is governed by the
 * The Pennsylvania State University Research and Educational Use License.
 * You should have received a copy of this license along with this program.
 * If not, contact <dmh309@psu.edu>.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "borg.h"

#define PI 3.14159265358979323846

int nvars = 11;
int nobjs = 2;

// All optimization problems must define a function similar to the one below.
// This function is responsible for reading the decision variables and
// evaluating the problem.  The resulting objectives and constraints (if any)
// are saved to the objs and consts arguments, respectively.  Note that
// all objectives MUST be minimized.
void dtlz2(double* vars, double* objs, double* consts) {
	int i;
	int j;
	int k = nvars - nobjs + 1;
	double g = 0.0;

	for (i=nvars-k; i<nvars; i++) {
		g += pow(vars[i] - 0.5, 2.0);
	}

	for (i=0; i<nobjs; i++) {
		objs[i] = 1.0 + g;

		for (j=0; j<nobjs-i-1; j++) {
			objs[i] *= cos(0.5*PI*vars[j]);
		}

		if (i != 0) {
			objs[i] *= sin(0.5*PI*vars[nobjs-i-1]);
		}
	}
}

int main(int argc, char* argv[]) {
	int i;

	// Create the DTLZ2 problem, defining the number of decision variables,
	// objectives and constraints.  The last argument, dtlz2, references
	// the function that evaluates the DTLZ2 problem.
	BORG_Problem problem = BORG_Problem_create(nvars, nobjs, 0, dtlz2);

	// Set the lower and upper bounds for each decision variable.
	for (i=0; i<nvars; i++) {
		BORG_Problem_set_bounds(problem, i, 0.0, 1.0);
	}

	// Set the epsilon values used by the Borg MOEA.  Epsilons define the
	// problem resolution, which controls how many Pareto optimal solutions
	// are generated and how far apart they are spaced.
	for (i=0; i<nobjs; i++) {
		BORG_Problem_set_epsilon(problem, i, 0.01);
	}

	// Run the Borg MOEA on the DTLZ2 problem for 1 million function
	// evaluations.
	BORG_Archive result = BORG_Algorithm_run(problem, 1000000);

	// Print the Pareto optimal solutions.
	BORG_Archive_print(result, stdout);

	// Free any allocated memory.
	BORG_Archive_destroy(result);
	BORG_Problem_destroy(problem);
	return EXIT_SUCCESS;
}

