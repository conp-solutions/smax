/* Norbert Manthey, Copyright 2018, All rights reserved
 *
 * This file exists to show case how the MaxSATSolver interface can be used.
 */

#include <cassert>
#include <iostream>

#include "errno.h"

#include <sys/time.h>
#include <sys/resource.h>

#include "include/MaxSATSolver.h"

using namespace std;

void versiontest ()
{
  MaxSATSolver maxsat(1, 0);
  cout << "test solver: " << maxsat.getSolverName () << " implementing version " << maxsat.getVersion() << endl;
}

void amotest ()
{
  cout << "run AMO test ..." << endl;
  vector<int> lits;
  MaxSATSolver maxsat(2, 0);
  lits = {1, 2};
  maxsat.addAtMostK(lits, 1);
  lits = {-1, -2};
  maxsat.addAtMostK(lits, 1);

  std::vector<int> model;
  uint64_t cost = 0;
  MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost);

  if(ret == MaxSATSolver::ReturnCode::SATISFIABLE)
	cout << "satisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNSATISFIABLE)
	cout << "unsatisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNKNOWN)
	cout << "unknown" << endl;
  else  if(ret == MaxSATSolver::ReturnCode::UNKNOWN)
	cout << "unknown" << endl;
  else
        cout << "unspecified (" << (int)ret << ")" << endl;

  cout << "cost: " << cost << endl;

  assert (ret == MaxSATSolver::ReturnCode::SATISFIABLE || ret == MaxSATSolver::ReturnCode::OPTIMAL);
  int positive = 0;
  for(int i = 1; i <= 2; ++i)
  {
    if(model[i] > 0)
      positive ++;
    cout << "v(" << i << ") == " << model[i] << std::endl;
  }

  assert (positive == 1);
  assert (cost == 0);
}

void amktest_simple ()
{
  cout << "run AMK (2 out of 4) test ..." << endl;
  vector<int> lits;
  MaxSATSolver maxsat(4, 0);
  lits = {1, 2, 3, 4};
  maxsat.addAtMostK(lits, 2);
  lits = {-1, -2, -3 , -4};
  maxsat.addAtMostK(lits, 2);

  std::vector<int> model;
  uint64_t cost = 0;
  MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost);

  if(ret == MaxSATSolver::ReturnCode::SATISFIABLE)
	cout << "satisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNSATISFIABLE)
	cout << "unsatisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNKNOWN)
	cout << "unknown" << endl;
  else if(ret == MaxSATSolver::ReturnCode::OPTIMAL)
	cout << "optimal" << endl;
  else
        cout << "unspecified (" << (int)ret << ")" << endl;

  cout << "cost: " << cost << endl;

  int positive = 0;
  for(int i = 1; i <= 4; ++i)
  {
    if(model[i] > 0)
      positive ++;
    cout << "v(" << i << ") == " << model[i] << std::endl;
  }

  assert (ret == MaxSATSolver::ReturnCode::SATISFIABLE || ret == MaxSATSolver::ReturnCode::OPTIMAL);
  assert (positive == 2);
  assert (ret == MaxSATSolver::ReturnCode::SATISFIABLE || cost == 0);
}

void amktest ()
{
  cout << "run AMK (5 out of 12) test ..." << endl;
  vector<int> lits;
  MaxSATSolver maxsat(12, 0);
  lits = {1, 2, 3, 4, 5 , 6, 7, 8,9 , 10, 11, 12};
  maxsat.addAtMostK(lits, 5);
  lits = {-1, -2, -3, -4, -5 , -6, -7, -8, -9, -10, -11, -12};
  maxsat.addAtMostK(lits, 7);

  // assign weights to negative variables
  for(int variable = 1; variable <= 12; ++ variable)
  {
    vector<int> clause;
    clause.push_back(variable);
    maxsat.addClause(clause, 13 - variable);
  }
  std::vector<int> model;
  uint64_t cost = 0;
  MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost);

  if(ret == MaxSATSolver::ReturnCode::SATISFIABLE)
	cout << "satisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNSATISFIABLE)
	cout << "unsatisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNKNOWN)
	cout << "unknown" << endl;
  else if(ret == MaxSATSolver::ReturnCode::OPTIMAL)
	cout << "optimal" << endl;
  else
        cout << "unspecified (" << (int)ret << ")" << endl;

  int positive = 0;
  for(int i = 1; i <= 12; ++i)
  {
    if(model[i] > 0)
      positive ++;
    cout << "v(" << i << ") == " << model[i] << std::endl;
  }
  cout << "cost: " << cost << endl;
  
  for(int i = 1; i <= 5; ++i)
    assert(model[i] > 0 && "first variables have highest weghts, hence, should be positive");

  assert (ret == MaxSATSolver::ReturnCode::SATISFIABLE || ret == MaxSATSolver::ReturnCode::OPTIMAL);
  assert (positive == 5);
  assert (ret == MaxSATSolver::ReturnCode::SATISFIABLE || cost == 28);
}

void amktest_full ()
{
  cout << "run AMK full test ..." << endl;
  vector<int> lits;
  
  for(int total = 2; total < 12; ++ total)
  {
    for(int positive = 1; positive + 1 < total; ++ positive)
    {
      const int negative = total - positive;
      for(int iteration = 0; iteration < 2; ++ iteration)
      {
	MaxSATSolver maxsat(total, 0);
	vector<int> lits;
	for(int l = 1; l <= total; ++ l) lits.push_back(l);
	maxsat.addAtMostK(lits, positive);
	lits.clear();
	for(int l = 1; l <= total; ++ l) lits.push_back(-l);
	maxsat.addAtMostK(lits, total-positive);
	
	if(iteration>0) {
	  // assign weights to negative variables
	  for(int variable = 1; variable <= total; ++ variable)
	  {
	    vector<int> clause;
	    clause.push_back(-variable);
	    maxsat.addClause(clause, variable);
	  }
	}

	std::vector<int> model;
	uint64_t cost = 0;
	MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost);
	cout << "full AMK: total: " << total << " positive: " << positive << " iteartion: " << iteration << " cost: " << cost << " estimate: " << ((positive)*(positive + 1))/2 << " model: ";
	int positive_count = 0;
	if(model.size() > 0) {
	  for(int i = 1; i < (int)model.size(); ++i)
	  {
	    cout << " v(" << i << ") == " << model[i];
	    if( model[i] > 0 ) ++positive_count;
	  }
	}
	cout << endl;
	assert(positive_count == positive && "hard formula should encode AMK");
	if(iteration == 1)
	  assert((int64_t)cost == ((positive)*(positive + 1))/2 && "the cost should match the estimate");
      }
    }
  }
}

void unsattest ()
{
  cout << "run unsat test ..." << endl;
  vector<int> lits;
  MaxSATSolver maxsat(3, 0);
  lits = {1, 2};
  maxsat.addClause(lits);
  lits = {1, -2};
  maxsat.addClause(lits);
  lits = {-1, -2};
  maxsat.addClause(lits);
  lits = {-1, 2};
  maxsat.addClause(lits);

  std::vector<int> model;
  uint64_t cost = 0;
  MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost);

  if(ret == MaxSATSolver::ReturnCode::SATISFIABLE)
	cout << "satisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNSATISFIABLE)
	cout << "unsatisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNKNOWN)
	cout << "unknown" << endl;

  assert (ret == MaxSATSolver::ReturnCode::UNSATISFIABLE);
}

void sattest ()
{
  cout << "run maxsat test ..." << endl;
  vector<int> lits;
  MaxSATSolver maxsat(3, 0);
  lits = {1, 2};
  maxsat.addClause(lits, 1);
  lits = {1, -2};
  maxsat.addClause(lits, 2);
  lits = {-1, -2};
  maxsat.addClause(lits, 3);
  lits = {-1, 2};
  maxsat.addClause(lits, 4);
  lits = {3, 2};
  maxsat.addClause(lits, 5);
  lits = {-3, 2};
  maxsat.addClause(lits, 6);

  std::vector<int> model;
  uint64_t core = 0;
  MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, core);

  if(ret == MaxSATSolver::ReturnCode::SATISFIABLE)
	cout << "satisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNSATISFIABLE)
	cout << "unsatisfiable" << endl;
  else if(ret == MaxSATSolver::ReturnCode::UNKNOWN)
	cout << "unknown" << endl;
  else if(ret == MaxSATSolver::ReturnCode::OPTIMAL)
	cout << "optimal" << endl;
  else
	cout << "unknown result" << endl;

  cout << "returned core of size " << model.size() << endl;

  assert (ret == MaxSATSolver::ReturnCode::OPTIMAL);
  assert (model.size() == 4 && "first for elements");
}

void invaltest ()
{
  cout << "run invalid input test ..." << endl;
  vector<int> lits;
  MaxSATSolver maxsat(3, 0);
  lits = {7,6};
  bool added = maxsat.addClause(lits);
  assert(!added && "too large lits");
  cout << "errno: " << maxsat.getErrno() << endl;
  assert(maxsat.getErrno() == -EINVAL);
  lits = {1,2};
  added = maxsat.addClause(lits);
  assert(maxsat.getErrno() == 0);
  added = maxsat.addClause(lits, 12);
  assert(maxsat.getErrno() == 0);

  MaxSATSolver vmaxsat(1 << 28, 0);
  assert(vmaxsat.getErrno() == -ENOMEM);

  MaxSATSolver cmaxsat(3, 1 << 28);
  assert(cmaxsat.getErrno() == -ENOMEM);
}


void nomemtest ()
{
  cout << "run no memory test ..." << endl;
  vector<int> lits = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  
  // limit to 40 MB, so that the limit can be reached fast
  rlim_t new_mem_lim = (rlim_t)40 * 1024*1024;
  rlimit rl;
  getrlimit(RLIMIT_AS, &rl);
  if (rl.rlim_max == RLIM_INFINITY || new_mem_lim < rl.rlim_max){
    rl.rlim_cur = new_mem_lim;
    if (setrlimit(RLIMIT_AS, &rl) == -1)
        assert(false && "WARNING! Could not set resource limit: Virtual memory.\n");
  }

  // check whether we can add many clauses
  int clauseiter = 0;
  {
    MaxSATSolver maxsat(10, 0);
    bool ok = true;
    do {
      ok = maxsat.addClause(lits, 0);
      clauseiter ++;
    } while(ok);
    cout << "c stopped after adding " << clauseiter << " clauses" << endl;
    assert(maxsat.getErrno() == -ENOMEM);
  }

  // check whether we can add many clauses
  {
    MaxSATSolver maxsat(10, 0);
    bool ok = true;
    int iter = 0;
    do {
      ok = maxsat.addAtMostK(lits, 5);
      iter ++;
    } while(ok);
    cout << "c stopped after adding " << iter << " at most k constraints" << endl;
    assert(maxsat.getErrno() == -ENOMEM);
  }
  {
    cout << "c run constrainted maxsat test" << endl;
    MaxSATSolver maxsat(10, 0);
    for(int i = 0 ; i < clauseiter/2; ++ i)
    {
      maxsat.addClause(lits, 0);
      assert(maxsat.getErrno() == 0);
    }
    std::vector<int> model;
    uint64_t cost = 0;
    MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost);
    assert(ret == MaxSATSolver::ReturnCode::ERROR);
    assert(maxsat.getErrno() == -ENOMEM);
  }
}

void amktest_limted ()
{
  cout << "run AMK (5 out of 12) test with limits ..." << endl;
  
  int64_t limits[9] = {-1,1024,64,32,16,8,4,2,1};
  
  for(int iteration = 0; iteration < 9; ++ iteration)
  {
    int64_t stepLimit = limits[iteration];

    vector<int> lits;
    MaxSATSolver maxsat(12, 0);
    lits = {1, 2, 3, 4, 5 , 6, 7, 8,9 , 10, 11, 12};
    maxsat.addAtMostK(lits, 5);
    lits = {-1, -2, -3, -4, -5 , -6, -7, -8, -9, -10, -11, -12};
    maxsat.addAtMostK(lits, 7);

    // assign weights to negative variables
    for(int variable = 1; variable <= 12; ++ variable)
    {
      vector<int> clause;
      clause.push_back(variable);
      maxsat.addClause(clause, 13 - variable);
    }
    
    std::vector<int> model;
    uint64_t cost = 0;
    cout << endl << "solve with limit: " << stepLimit << endl;
    MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost, UINT64_MAX, 0x0, stepLimit);

    if(ret == MaxSATSolver::ReturnCode::SATISFIABLE)
	  cout << "satisfiable" << endl;
    else if(ret == MaxSATSolver::ReturnCode::UNSATISFIABLE)
	  cout << "unsatisfiable" << endl;
    else if(ret == MaxSATSolver::ReturnCode::UNKNOWN)
	  cout << "unknown" << endl;
    else if(ret == MaxSATSolver::ReturnCode::OPTIMAL)
	  cout << "optimal" << endl;
    else
	  cout << "unspecified (" << (int)ret << ")" << endl;

    if(ret != MaxSATSolver::ReturnCode::UNKNOWN && ret != MaxSATSolver::ReturnCode::ERROR)
    {
      int positive = 0;
      for(int i = 1; i <= 12; ++i)
      {
	if(i >= (int)model.size()) continue;
	if(model[i] > 0)
	  positive ++;
	cout << "v(" << i << ") == " << model[i] << std::endl;
      }
      assert (ret == MaxSATSolver::ReturnCode::SATISFIABLE || ret == MaxSATSolver::ReturnCode::OPTIMAL);
      assert (positive == 5);
      assert (ret == MaxSATSolver::ReturnCode::SATISFIABLE || cost == 28);
    }
  }
}

void maxsat_1_test ()
{
  for(int iteration = 0; iteration < 2; ++ iteration)
  {
    cout << "run maxsat_1_" << iteration << " test ..." << endl;
    MaxSATSolver maxsat(12, 0);

    // assign weights to negative variables
    for(int variable = 1; variable <= 12; ++ variable)
    {
      vector<int> clause;
      clause.push_back(variable);
      maxsat.addClause(clause, 13 - variable);
    }
    
    // second iteration adds all units as negation
    if(iteration>0) {
      for(int variable = 1; variable <= 12; ++ variable)
      {
	vector<int> clause;
	clause.push_back(-variable);
	maxsat.addClause(clause);
      }      
    }
    
    std::vector<int> model;
    uint64_t cost = 0;
    MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost);

    int positive = 0;
    for(int i = 1; i <= 2; ++i)
      cout << "v(" << i << ") == " << model[i] << std::endl;
    cout << "cost: " << cost << endl;
    
    if(iteration==0) assert (cost == 0);
    if(iteration==1) assert (cost == 78);
  }
}

void maxsat_2_test ()
{
  for(int iteration = 0; iteration < 2; ++ iteration)
  {
    cout << "run maxsat_2_" << iteration << " test ..." << endl;
    MaxSATSolver maxsat(2, 0);

    // assign weights to negative variables
    maxsat.addClause({-1, -2}, 1);
    maxsat.addClause({-1, 2}, 1);
    maxsat.addClause({1, -2}, 1);
    maxsat.addClause({1, 2}, 1);

    if(iteration==1) {
      maxsat.addClause({-1, -2}, 0);
      maxsat.addClause({1, 2}, 0);
    }
    
    std::vector<int> model;
    uint64_t cost = 0;
    MaxSATSolver::ReturnCode ret = maxsat.compute_maxsat(model, cost);

    int positive = 0;
    for(int i = 1; i <= 2; ++i)
      cout << "v(" << i << ") == " << model[i] << std::endl;
    cout << "cost: " << cost << endl;
    
    if(iteration==0) assert (cost == 1);
    if(iteration==1) assert (cost == 1);
  }
}

int main(int argc, char **argv)
{
  versiontest ();
  cout << endl;
  unsattest ();
  cout << endl;
  sattest ();
  cout << endl;
  maxsat_1_test ();
  cout << endl;
  maxsat_2_test ();
  cout << endl;
  amotest ();
  cout << endl;
  amktest_simple ();
  cout << endl;
  amktest_full ();
  cout << endl;
  amktest ();
  cout << endl;
  invaltest ();
  cout << endl;
  amktest_limted();
  cout << endl;
  if(argc==1) {
    nomemtest ();
    cout << endl;
  } else {
    cout << "skip nomemtest, as requested by additional cli arguments ..." << endl;
  }
}
