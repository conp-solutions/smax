/*******************************************************************************[SmootherSolver.cc]

Copyright (c) 2017-2018, Norbert Manthey, all rights reserved.

--------------- Original Minisat Copyrights

Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **************************************************************************************************/

// SMax
#include "smax-src/SMaxSolver.h"
#include "smax-src/SMaxLimits.h"

// open-wbo
#include "open-wbo/MaxSAT.h"
#include "open-wbo/MaxSAT_Partition.h"
// Algorithms
#include "open-wbo/algorithms/Alg_LinearSU.h"
#include "open-wbo/algorithms/Alg_MSU3.h"
#include "open-wbo/algorithms/Alg_OLL.h"
#include "open-wbo/algorithms/Alg_PartMSU3.h"
#include "open-wbo/algorithms/Alg_WBO.h"

// Minisat
#include "core/Solver.h"
#include "mtl/Sort.h"
#include "utils/System.h"

// std
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

using namespace SMax;
using namespace openwbo;
using namespace NSPACE;

static BoolOption   opt_debug            ("SMAX ALGORITHM", "debug", "Allow debug output", false);

static IntOption    cardinality          ("SMAX - Open WBO - Encodings", "cardinality",
                                          "Cardinality encoding (0=cardinality networks, "
                                          "1=totalizer, 2=modulo totalizer).\n",
                                          1, IntRange(0, 2));


inline std::ostream& operator<<(std::ostream& other, const NSPACE::Lit l)
{
  if(sign(l)) other << "-";
  other << var(l) + 1;
  return other;
}

/** print elements of a std::vector */
template <typename T>
inline std::ostream& operator<<(std::ostream& other, const std::vector<T>& data)
{
    for (unsigned i = 0 ; i < data.size(); ++ i) {
        other << " " << data[i];
    }
    return other;
}


/** print lbool */
inline std::ostream& operator<<(std::ostream& other, const NSPACE::lbool& b)
{
    if(b == l_False) other << "false";
    else if (b == l_True) other << "true";
    else other << "undef";
    return other;
}

/** print elements of a vec */
template <typename T>
inline std::ostream& operator<<(std::ostream& other, const NSPACE::vec<T>& data)
{
    for (int i = 0 ; i < data.size(); ++ i) {
        other << " ";
	other << "(" << i << ")";
	operator<< <T>(other, data[i]);
    }
    return other;
}

/** checks the time and might kill the current process with an error */
static bool checktime ()
{
  time_t seconds;
  seconds = time (NULL);

  if(false && 1523918580 < seconds) // mid April 2018
  {
    std::cerr << "[ERROR] smoother detected it's being used beyond the scope of it's license, abort" << std::endl;
    _exit(1);
  }
  return true;
}

SMaxSolver::SMaxSolver(const int inputVariables)
: varCnt(inputVariables)
, inputVarCnt(inputVariables)
, lastErrno(0)
, simplify_debug(opt_debug)
{
  checktime();

#ifndef NDEBUG
  if(!simplify_debug) simplify_debug = getenv("DEBUG_LIBSMAX") != NULL;
#endif
}

SMaxSolver::~SMaxSolver()
{
    if(maxsat_formula)
    {
        delete maxsat_formula;
        maxsat_formula = nullptr;
    }
}

void SMaxSolver::reserveVars(unsigned int variables)
{
  literalPresent.reserve(2*variables);
}

Var SMaxSolver::newVar()
{
  varCnt ++;
  literalPresent.push_back(0);
  literalPresent.push_back(0);
  if(simplify_debug) std::cout << "use next new variable " << varCnt << std::endl;
  return varCnt;
}

void SMaxSolver::addClause(const NSPACE::vec< NSPACE::Lit >& literals, uint64_t weight)
{
  // create the formula if we did not add a clause until now
  if(maxsat_formula == nullptr)
  {
    maxsat_formula = new MaxSATFormula();
    maxsat_formula->setFormat(_FORMAT_MAXSAT_);
    maxsat_formula->setProblemType(_UNWEIGHTED_);
  }
  
  // get the top variable right
  for(int i = 0 ; i < literals.size(); ++i)
  {
    const Var v = NSPACE::var(literals[i]);
    // set the top variable in the formula
    if(v >= maxsat_formula->nVars()) maxsat_formula->newVar(v+1);
  }
  
  if(weight == 0)
  {
    maxsat_formula->addHardClause(literals);
  }
  else if (weight > 0)
  {
    // Updates the maximum weight of soft clauses.
    maxsat_formula->setMaximumWeight(weight);
    // Updates the sum of the weights of soft clauses.
    maxsat_formula->updateSumWeights(weight);
    // Finally add the clause
    maxsat_formula->addSoftClause(weight, literals);
  }
}


void SMaxSolver::addNegatedUnits(const std::vector< int >& literals)
{
  temporary_lits.growTo(1);
  for(int literal : literals)
  {
    temporary_lits[0] = toLit(literal);
    addClause(temporary_lits, 0);
  }
}

void SMaxSolver::addAtMostOne(const std::vector< int >& literals)
{
  // for now, use the binary encoding only
  temporary_lits.clear();
  temporary_lits.push(NSPACE::lit_Undef);
  temporary_lits.push(NSPACE::lit_Undef);
  
  if(simplify_debug) std::cerr << "c add at-most-one for " << literals << std::endl;
  assert(temporary_lits.size() == 2 && "the added clauses are binary clauses");
  
  for(unsigned i = 0 ; i < literals.size(); ++ i)
  {
    temporary_lits[0] = ~toLit(literals[i]); // -l_i
    for(unsigned j = i+1 ; j < literals.size(); ++ j )
    {
      temporary_lits[1] = ~toLit(literals[j]); // -l_j
      addClause(temporary_lits, 0); // add -l_i \lor -l_j
    }
  }
  return;
}

void SMaxSolver::encodeAtMosK_SWC(const std::vector< int >& lits, const int k)
{

  const int n = lits.size();
  std::vector< std::vector<Lit> > s (n, std::vector<Lit>(k, lit_Undef));
  
  if(simplify_debug) std::cout << "c encode SWC for k==" << k << " and literals " << lits << std::endl;
  if(simplify_debug) std::cout << "c start with variable " << nVars() << std::endl;
  
  // encode AMK, lazily add new variables
  // (1) from SWC paper:
  temporary_lits.clear();
  temporary_lits.growTo(2);
  for( int i = 1; i + 1< n ; ++i ) {
    for( int j = 0; j < k; ++j ) {
      if( s[i-1][j] == lit_Undef ) s[i-1][j] = SMaxSolver::toLit(newVar());
      if( s[i][j] == lit_Undef ) s[i][j] = SMaxSolver::toLit(newVar());
      temporary_lits[0] = ~s[i-1][j];
      temporary_lits[1] = s[i][j];
      addClause(temporary_lits, 0);
    }
  }
  
  // (2) from paper
  for( int i = 0 ; i + 1 < n; ++ i ) {
    const int j = 0;
    if( s[i][j] == lit_Undef ) s[i][j] = SMaxSolver::toLit(newVar());
    temporary_lits[0] = ~SMaxSolver::toLit(lits[i]);
    temporary_lits[1] = s[i][j];
    addClause(temporary_lits, 0);
  }

  // (4) from paper
  for( int i = 1; i < n; ++ i ) {
    if(s[i-1][k-1] == lit_Undef) s[i-1][k-1] = SMaxSolver::toLit(newVar());
    temporary_lits[0] = ~s[i-1][k-1];
    temporary_lits[1] = ~SMaxSolver::toLit(lits[i]);
    addClause(temporary_lits, 0);
  }
  
  // (3) from paper
  temporary_lits.growTo(3);
  for( int i = 1; i + 1 < n; ++i ) {
    for( int j = 0; j + 1 < k; ++ j ) {
      if( s[i-1][j] == lit_Undef ) s[i-1][j] = SMaxSolver::toLit(newVar());
      if( s[i][j+1] == lit_Undef ) s[i][j+1] = SMaxSolver::toLit(newVar());
      temporary_lits[0] = ~s[i-1][j];
      temporary_lits[1] = ~SMaxSolver::toLit(lits[i]);
      temporary_lits[2] = s[i][j+1];
      addClause(temporary_lits, 0);
    }
  }

  if(simplify_debug) std::cout << "c end with variables " << nVars() << std::endl;
  
  return;
}


bool SMaxSolver::addClause(const std::vector< int >& literals, uint64_t weight)
{
  // check limits
  if(weight > SMOOTHER_MAX_WEIGHT)
  {
    setErrno(-EINVAL);
    return false;
  }
  
  // ignore this constraint in case it has been too large?
  for(int literal : literals)
    if(literal == 0 || abs(literal) > inputVarCnt) return false;
  
  // convert the input into the internal format
  temporary_lits.clear();
  temporary_lits.growTo(literals.size());
  for(unsigned i = 0 ; i < literals.size(); ++i) temporary_lits[i] = toLit(literals[i]);
  
  addClause(temporary_lits, weight);

  // in case we see a weight different from 1, and it's not a hard class, the input formula is "weighted"
  if(weight != 1) maxsat_formula->setProblemType(_WEIGHTED_);
  
  assert(maxsat_formula != nullptr && "has been initialized during adding constraint");
  return true;
}

bool SMaxSolver::addAtMostK(const std::vector< int >& literals, unsigned int k)
{
  // trivial satisfiable?
  if(k >= literals.size()) return true;
  
  // ignore this constraint in case it has been too large?
  for(int literal : literals)
    if(literal == 0 || abs(literal) > inputVarCnt) return false;

  if(k == 0) addNegatedUnits(literals);
  else if(k == 1) addAtMostOne(literals);
  else {
    // TODO switch to built-in PB constraint
    /*
    PB cardinalityConstraint;
    maxsat_formula->addPBConstraint(cardinalityConstraint);
    */
    encodeAtMosK_SWC(literals, k);
  }
  
  assert(maxsat_formula != nullptr && "has been initialized during adding constraint");
  return true;
}


MaxSATSolver::ReturnCode SMaxSolver::compute_maxsat(vector< int >& model, uint64_t& cost, uint64_t maxCost, const vector< int >* startAssignment, int64_t maxMinimizeSteps)
{
  cost = UINT64_MAX;

  // check environment
  if(maxsat_formula == nullptr) {
    setErrno(-EINVAL);
    return MaxSATSolver::ERROR;
  }
  
  model.clear();
  
  // set the weight for the formula
  maxsat_formula->setHardWeight(maxCost);

  // print what we are going to do next
  if(simplify_debug) 
    std::cerr << "[SMAX] Solve formula with " << maxsat_formula->nVars() << " vars, "
                                              << maxsat_formula->nHard() << " hard cls, "
					      << maxsat_formula->nSoft() << " soft cls, "
					      << maxsat_formula->nCard() << " card constraints"
					      << std::endl;
					      
  if(simplify_debug) std::cerr << "[SMAX] max steps per SAT call: " << maxMinimizeSteps << std::endl;
  
  MaxSAT *S = NULL;
  
  /* 
  Solver *SAT = initMUS(activeGroups, startAssignment);
  if(maxMinimizeSteps >= 0) SAT->setConfBudget(maxMinimizeSteps);
  ret = SAT->solveLimited(activeGroups);
  if(maxMinimizeSteps >= 0) SAT->budgetOff();
  */

  // algorithm selection code taken from open-wbo  
  if (maxsat_formula->getProblemType() == _UNWEIGHTED_) {
    // Unweighted
    if(simplify_debug) std::cerr << "[SMAX] start with PartMSU3" << std::endl;
    S = new PartMSU3(_VERBOSITY_MINIMAL_, _PART_BINARY_, RES_GRAPH,
                         cardinality);
    S->loadFormula(maxsat_formula);

    if (((PartMSU3 *)S)->chooseAlgorithm() == _ALGORITHM_MSU3_) {
      delete S; // the new version will keep the formula around
      // TODO there might be a memory leak here, check and fix!
      S = new MSU3(_VERBOSITY_MINIMAL_);
      // if(simplify_debug) std::cerr << "[SMAX] switch to MSU3" << std::endl;
    }
  } else {
    // Weighted
    if(simplify_debug) std::cerr << "[SMAX] use OLL" << std::endl;
    S = new OLL(_VERBOSITY_MINIMAL_, cardinality);
  }

  if (S->getMaxSATFormula() == NULL)
    S->loadFormula(maxsat_formula);

  S->search();
  
  StatusCode ret = S->getStatus();
  
  /*
  if(ret == l_True) {
    storeSATmodel(SAT, model);
    return MaxSATSolver::ReturnCode::SATISFIABLE;
  }
  */
  
  switch(ret)
  {
    case _UNSATISFIABLE_:
      delete S; S = nullptr;
      return MaxSATSolver::ReturnCode::UNSATISFIABLE;
    case _ERROR_:
      delete S; S = nullptr;
      return MaxSATSolver::ReturnCode::ERROR;
    case _SATISFIABLE_:
      cost = S->getLastBound();
      model.resize(inputVarCnt + 1, 0);
      for(Var v = 0; v < inputVarCnt; ++ v)
	model[v+1] = S->getValue(v);
      delete S; S = nullptr;
      return MaxSATSolver::ReturnCode::SATISFIABLE;
    case _OPTIMUM_:
      cost = S->getLastBound();
      model.resize(inputVarCnt + 1, 0);
      for(Var v = 0; v < inputVarCnt; ++ v)
	model[v+1] = S->getValue(v);
      delete S; S = nullptr;
      return MaxSATSolver::ReturnCode::OPTIMAL;
    default:
      delete S; S = nullptr;
      return MaxSATSolver::UNKNOWN;
  }
}
