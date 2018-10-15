/**********************************************************************************[MaxSATSolver.h]

Copyright (c) 2017, Norbert Manthey, all rights reserved.

**************************************************************************************************/

#include <cstdint>
#include <vector>

#include <errno.h>
#include "smax-src/SMaxLimits.h"
#include "include/MaxSATSolver.h"
#include "smax-src/SMaxSolver.h"

using namespace SMax;

const char* MaxSATSolver::getSolverName () const
{
  return "smax - Norbert Manthey, 2018";
}

MaxSATSolver::MaxSATSolver(int nVars, int nClausesEstimate)
{
  // try to run usual code, but catch exceptions if they happen
  try {
    // fail if the input values are too large, results in errno -ENOMEM
    if(nVars >= SMAX_MAX_VAR ) return;
    if(nClausesEstimate >= SMAX_MAX_CLS ) return;

    externalData = new SMax::SMaxSolver(nVars);
    if(!externalData) return; // results in -ENOMEM
    SMaxSolver *solver = (SMaxSolver*)externalData;
    
    solver->reserveVars(solver->nVars());
    while ((int)nVars > solver->nVars()) solver->newVar();
  }
  catch (...)
  {
    // in case we created an object, kill it again, and set -ENOMEM
    if(externalData) {
      delete (SMaxSolver*)externalData;
      externalData = nullptr; 
    }
  }
}
    
MaxSATSolver::~MaxSATSolver()
{
  if(externalData) delete (SMaxSolver*)externalData;
  externalData = nullptr;
}

int MaxSATSolver::getErrno() const
{
  if(!externalData) return -ENOMEM;
  SMaxSolver *solver = (SMaxSolver*)externalData;
  return solver->getErrno();
}

bool MaxSATSolver::addClause(const std::vector<int> &literals, uint64_t weight)
{
  if(!externalData) return false;
  SMaxSolver *solver = (SMaxSolver*)externalData;
  if(solver->getErrno() != 0 && solver->getErrno() != -EINVAL) return false;
  
  if(weight > SMOOTHER_MAX_WEIGHT) {
    solver->setErrno(-EINVAL);
    return false;
  }
  
  solver->setErrno(0);

  try {
    bool ret = solver->addClause(literals, weight);
    if(!ret) solver->setErrno(-EINVAL);
    return ret;
  }
  catch (...)
  {
    // in case of the exception, we assume insufficient memory
    solver->setErrno(-ENOMEM);
    return ERROR;
  }
}

bool MaxSATSolver::addAtMostK(const std::vector<int> &literals, const unsigned k)
{
  if(!externalData) return false;
  SMaxSolver *solver = (SMaxSolver*)externalData;
  if(solver->getErrno() != 0 && solver->getErrno() != -EINVAL) return false;
  solver->setErrno(0);

  try {
    bool ret = solver->addAtMostK(literals, k);
    if(!ret) solver->setErrno(-EINVAL);
    return ret;
  }
  catch (...)
  {
    // in case of the exception, we assume insufficient memory
    solver->setErrno(-ENOMEM);
    return ERROR;
  }
}
    
MaxSATSolver::ReturnCode MaxSATSolver::compute_maxsat(std::vector<int> &model,
                              uint64_t & cost,
                              uint64_t maxCost,
                              const std::vector<int> *startAssignment,
                              int maxMinimizeSteps)
{
  if(!externalData) return ERROR;
  SMaxSolver *solver = (SMaxSolver*)externalData;
  if(solver->getErrno() != 0 && solver->getErrno() != -EINVAL) return ERROR;
  solver->setErrno(0);
  
  // try to run usual code, but catch exceptions if they happen
  try {
    // in case we got errors before, avoid computations without overwriting the error code
    return solver->compute_maxsat(model, cost, maxCost, startAssignment, maxMinimizeSteps);
  }
  catch (...)
  {
    // in case of the exception, set the error number
    solver->setErrno(-ENOMEM);
    return ERROR;
  }
}

