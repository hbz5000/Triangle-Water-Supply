#ifndef borgProblemDefinition_h
#define borgProblemDefinition_h

#include "Borg-1.6/borgms.h"

void setProblemDefinition(BORG_Problem &problem, int formulation)
{
	// The parameter bounds are the same for all formulations
	if(formulation == 0)
	{
		BORG_Problem_set_bounds(problem, 0, 0.2, 1.0);
		BORG_Problem_set_bounds(problem, 1, 0.2, 1.0);
		BORG_Problem_set_bounds(problem, 2, 0.2, 1.0);
		BORG_Problem_set_bounds(problem, 3, 0.2, 1.0);
		BORG_Problem_set_bounds(problem, 4, 0.2, 1.0);
		BORG_Problem_set_bounds(problem, 5, 0.2, 1.0);
		BORG_Problem_set_bounds(problem, 6, 0.2, 1.0);
	}
	else
	{
		BORG_Problem_set_bounds(problem, 0, 0.001, 1.0);
		BORG_Problem_set_bounds(problem, 1, 0.001, 1.0);
		BORG_Problem_set_bounds(problem, 2, 0.001, 1.0);
		BORG_Problem_set_bounds(problem, 3, 0.001, 1.0);
		BORG_Problem_set_bounds(problem, 4, 0.001, 1.0);
		BORG_Problem_set_bounds(problem, 5, 0.001, 1.0);
		BORG_Problem_set_bounds(problem, 6, 0.001, 1.0);
	}
	BORG_Problem_set_bounds(problem, 7, 0.001, 1.0);
	BORG_Problem_set_bounds(problem, 8, 0.001, 1.0);
	BORG_Problem_set_bounds(problem, 9, 0.001, 1.0);
	BORG_Problem_set_bounds(problem, 10, 0.0, 0.1);
	BORG_Problem_set_bounds(problem, 11, 0.0, 0.1);
	BORG_Problem_set_bounds(problem, 12, 0.0, 0.1);
	BORG_Problem_set_bounds(problem, 13, 0.0, 0.1);
	BORG_Problem_set_bounds(problem, 14, 0.0, 5.0);
	BORG_Problem_set_bounds(problem, 15, 0.0, 5.0);
	BORG_Problem_set_bounds(problem, 16, 0.0, 5.0);
	BORG_Problem_set_bounds(problem, 17, 0.0, 5.0);
	BORG_Problem_set_bounds(problem, 18, 1.0, 2.0);
	BORG_Problem_set_bounds(problem, 19, 1.0, 2.0);
	BORG_Problem_set_bounds(problem, 20, 1.0, 2.0);
	BORG_Problem_set_bounds(problem, 21, 1.0, 2.0);
	BORG_Problem_set_bounds(problem, 22, 1.0, 2.0);
	BORG_Problem_set_bounds(problem, 23, 1.0, 2.0);
	BORG_Problem_set_bounds(problem, 24, 1.0, 2.0);
	BORG_Problem_set_bounds(problem, 25, 1.0, 2.0);
	BORG_Problem_set_bounds(problem, 26, 0.1, 0.47);
	BORG_Problem_set_bounds(problem, 27, 0.0, 0.37);
	BORG_Problem_set_bounds(problem, 28, 0.05, 0.42);
	BORG_Problem_set_bounds(problem, 29, 0.355, 0.725);
	
	if(formulation == 0)
	{
		BORG_Problem_set_bounds(problem, 30, 0.2, 1.0);
	}
	else
	{
		BORG_Problem_set_bounds(problem, 30, 0.001, 1.0);
	}
	
	// Set epsilons for objectives
	// Original values: (works fine for Formulations 0-4)
	
	BORG_Problem_set_epsilon(problem, 0, 0.001);
	BORG_Problem_set_epsilon(problem, 1, 0.01);
	BORG_Problem_set_epsilon(problem, 2, 0.0005);

	if(formulation >= 2)
		BORG_Problem_set_epsilon(problem, 3, 0.001);

	if(formulation >= 3)
		BORG_Problem_set_epsilon(problem, 4, 0.001);

	if(formulation == 6)
		BORG_Problem_set_epsilon(problem, 5, 0.005);

}

#endif
