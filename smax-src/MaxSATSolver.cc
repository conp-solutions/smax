/**********************************************************************************[MaxSATSolver.h]

Copyright (c) 2017, Norbert Manthey, all rights reserved.

**************************************************************************************************/

#include <cstdint>
#include <vector>

#include <errno.h>

#include "include/MaxSATSolver.h"



const char* MaxSATSolver::getSolverName () const
{
	return "smax - Norbert Manthey, 2018";
}

MaxSATSolver::MaxSATSolver(int nVars, int nClausesEstimate)
{
  
}
    
MaxSATSolver::~MaxSATSolver()
{
  
}

    int MaxSATSolver::getErrno() const
    {
      return -ENOMEM;
    }

    bool MaxSATSolver::addClause(const std::vector<int> &literals, uint64_t weight)
    {
	return false; // not yet implemented
    }

    bool MaxSATSolver::addAtMostK(const std::vector<int> &literals, const unsigned k)
    {
	return false; // not yet implemented      
    }
    
    MaxSATSolver::ReturnCode MaxSATSolver::compute_maxsat(std::vector<int> &model,
                              uint64_t maxCost,
                              const std::vector<int> *startAssignment,
                              int maxMinimizeSteps)
    {
      return MaxSATSolver::ReturnCode::UNKNOWN; // not yet implemented
    }

