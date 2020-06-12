/************************************************************************************[SMaxSolver.h]

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

#ifndef SMaxSolver_h
#define SMaxSolver_h

// generic interface
#include "include/MaxSATSolver.h"

// from open-wbo
#include "open-wbo/MaxSATFormula.h"

// from Minisat
#include "core/Solver.h"

// std
#include <vector>

// make the life of IDEs simpler
#ifndef NSPACE
#define NSPACE Minisat
#endif

namespace SMax {

//=================================================================================================


class SMaxSolver {
  
protected:
  
    NSPACE::Var varCnt; /// total variables in the input
    int inputVarCnt; /// total variables in the input formula (set, if positive)

    /// pointer to the formula object that is used to get the formula
    openwbo::MaxSATFormula *maxsat_formula = nullptr; 
    
    /// detect pure literals
    std::vector<char> literalPresent;
    
    /// vector used to convert input clauses and constraints
    NSPACE::vec<Lit> temporary_lits;

    /** initialize a solver object to perform MaxSAT computation
     *  Relaxation variables start at varCnt before the call to the function
     * @param activeGroups vector for all group activation variables (excluding group 0, i.e. variable at index 0 points to group 1!)
     * @return Solver object
     */
    NSPACE::Solver* initMaxSAT();

    /** add a clause with the internal representation
     *
     * @param literals literals of the clause to add
     * @param weight  weight of the clause, 0 means it's a hard clause
     */
    void addClause(const NSPACE::vec< NSPACE::Lit >& literals, uint64_t weight);

    // methods to convert int based constraints into internal formula
    void addNegatedUnits(const std::vector< int >& literals);
    void addAtMostOne(const std::vector< int >& literals);
    void encodeAtMosK_SWC(const std::vector< int >& literals, const int k);

    /** write the hard CNF to the given file */
    void write_hard_formula(const char* filename);

    /** convert the model from a Solver into an IPASIR style vector
     *
     *  @param SAT pointer to a solver object that stores a model
     *  @param model model in IPASIR representation (output parameter)
     */
    void storeSATmodel(NSPACE::Solver* SAT, std::vector< int >& model);

    /** convert integer representation into internal literal representation */
    NSPACE::Lit toLit(const int lit) const { return NSPACE::mkLit(abs(lit) - 1, lit < 0); }
    
    /** stores that last found error code */
    int lastErrno;
    
    /** stores whether we are actually debugging */
    bool simplify_debug = false;

    /** verbosity of the used SAT solvers */
    int setSATverbosity = 0;
    
 public:
    // Constructor/Destructor:
    //
    SMaxSolver();
    SMaxSolver(const int inputVariables);
    ~SMaxSolver();
    
    std::vector<int> model; // in case we found some kind of SAT result, store the model here

    /** remove all left overs from previous results */
    void clean_state();

    // Problem specification:
    //
    NSPACE::Var newVar(); // Add a new variable.
    void    reserveVars(unsigned variables); // Reserve space for most of the structures that are used per variable
    
    int     nVars      ()      const {return varCnt;}         // The current number of variables.
    void    interrupt() {};          // Trigger a (potentially asynchronous) interruption of the solver.

    // For error handling
    int getErrno() const { return lastErrno; }
    void setErrno(const int errorNumber) { lastErrno = errorNumber; }

    /** add a clause, weight == 0 means it's a hard clause */
    bool addClause(const std::vector< int >& literals, uint64_t weight);

    bool addAtMostK(const std::vector< int >& literals, unsigned k);
    
    /** compute MaxSAT solution
     *
     *  Implements the method from the MaxSATSolver.h interface
     */
    MaxSATSolver::ReturnCode compute_maxsat(std::vector<int> &model,
                              uint64_t &cost,
                              uint64_t maxCost = INT64_MAX,
                              const std::vector<int> *startAssignment = 0,
                              int64_t maxMinimizeSteps = -1);

    int getValue(unsigned variable)
    {
      if(variable > 0 && variable < model.size())
      {
        return model[variable];
      }
      return 0;
    }
};


//=================================================================================================
}

#endif
