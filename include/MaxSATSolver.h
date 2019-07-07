/**********************************************************************************[MaxSATSolver.h]

Copyright (c) 2017, Norbert Manthey, all rights reserved.

**************************************************************************************************/

#ifndef MaxSATSolver_Interface_h
#define MaxSATSolver_Interface_h

#include <cstdint>
#include <vector>

static_assert(sizeof(int) == 4,      "The library will assume that 'int' represents numbers between -2G to 2G");
static_assert(sizeof(unsigned) == 4, "The library will assume that 'unsigned' represents numbers between 0 to 4G");
static_assert(sizeof(uint64_t) == 8, "The library will assume that 'int64_t' represents 64 bit numbers");

#if __GNUC__ >= 4
  #define DLL_PUBLIC __attribute__ ((visibility ("default")))
#else
  #define DLL_PUBLIC
#endif

/** Class to solve the MaxSAT problem
 *
 *  For many applications, a MaxSAT solution of a CNF (weighted) CNF formula is
 *  of interest, because a solution represents (one of) the best possible
 *  state(s) of a given input problem. Depending on the use case it is
 *  important to either quickly generate some non-optimal solution, or to
 *  insist on computing the global optimum. This trade of is respected in the
 *  interface by allowing the caller to specify a number of steps that the
 *  implementation is allowed to perform to optimize a found solution before
 *  the currently best known solution might be returned.
 */
class DLL_PUBLIC MaxSATSolver {
  
    /** Allow class to point to some (arbitrary) data */
    void* externalData;

    /** Explicitly disallow copy constructors */
    MaxSATSolver(const MaxSATSolver& other) = delete;

    /** Explicitly disallow copy operator */
    MaxSATSolver& operator=(MaxSATSolver const&) = delete;
  
public:

    /** Return codes for the caller to compute a MaxSAT solution
     *
     *  The input problem might be unsatisfiable, hence this case is included.
     *  If at least one solution is found, the problem is known to be
     *  satisfiable, and the flag satisfiable is returned.
     *  Furthermore, the computation might stop without finding a solution at
     *  all, e.g. due to hitting memory limits. In this case, the return
     *  code UNKNOWN is used.
     *  In case an optimal solution is returned by a method, the OPTIMAL flag
     *  will be returned.
     *
     *  In case an error happened during executing the call, the code ERROR
     *  will be returned. In this case, the error number can be extracted with
     *  the method getErrno(). Furthermore, when the return code is UNKNOWN, an
     *  error number might be set, e.g. -ENOMEM.
     **/
    enum ReturnCode
    {
        UNKNOWN = 0,
        SATISFIABLE = 1,
        UNSATISFIABLE = 2,
        OPTIMAL = 3,
        ERROR = 4,
    };

    /** This integer represents the version of the MaxSAT interface */
    unsigned getVersion () const;

    /** This string contains the name of the used backend */
    const char* getSolverName () const;

    /** Initialize the MaxSAT solver for a given formula. The number of
     *  variables has to be given. The variables to be added later are not
     *  allowed to exceed this number.
     *
     *  Note: the number of clauses is mainly used to initialize internal data
     *  structures, but should not be treated as a hard limit.
     *
     *  The upper limits for variables, clauses and groups are currently set to
     *  the following values:
     *
     *    variables: (1 << 24), i.e. 16 M
     *    clauses:   (1 << 26), i.e. 64 M
     *
     *  Possible error codes:
     *   -ENOMEM ... in case of a failure during the solver initialzation
     *
     *  @param nVars highest variable in the input problem
     *  @param nClausesEstimate rough estimate number of clauses in the input
     *         problem. This can be wrong, but could be used to reserve space
     *         in the solver.
     **/
    MaxSATSolver(int nVars, int nClausesEstimate = 8192);
    
    /** A call to this method frees all resources of the solver. */
    ~MaxSATSolver();

    /** Return error number code in case the last call to another method failed
     *
     *  When a computation method returns, it usually does not fail and returns
     *  the state of the computation. However, in case exceptions had to be
     *  caught, the computation of the solver has to be aborted. In this case,
     *  the computation method will return ERROR, and the actual error code is
     *  stored internally. This code can be extracted with this method. The
     *  returned value is a negative value that relates to the defined values in
     *  errno.h.
     *
     *  Note: once the error value is set, no further computation can be carried
     *  out. Any further computataion call might result in invalid computations.
     *
     *  An exception is the error code -EINVAL, which might be set when clauses
     *  or cardinality constraints are added to the solver. In case literals
     *  exceed the initially set variable limit, or too high group IDs are
     *  used, -EINVAL is returned. In this case, fixed constraints or clauses
     *  can still be added to the solver.
     */
    int getErrno() const;

    /** Add a clause to the solver
     *
     *  The given literals will be added as a clause to the solver (which is
     *  equivalent to an "at-least-one" constraint). If no weight is specified,
     *  or if the weight 0 is specified, the clause will be added to the set
     *  of hard constraint clauses, which always has to be satisfied.
     *  Otherwise, the clause will be added with the given weight, and whenever
     *  a solution falsifies this clause the weight has to be paid as a cost.
     * 
     *  This method cannot be called anymore once a compute method has been
     *  called.
     * 
     *  Possible error codes:
     *   -ENOMEM ... the additional clause could not be added to the solver
     *   -EINVAL ... a literal of the clause is greater than the maximal
     *               specified variable, or 0

     *  @param literals list of literals that represent the clause
     *  @param weight == 0, if hard clause
     *                 > 0, weight to falsify this clause in a solution
     *
     *  @return true, if adding the constraint was ok
     *          false, if adding resulted in a failure, e.g. invalid literals,
     *                 or group ID (check error code!)
     */
    bool addClause(const std::vector<int> &literals, uint64_t weight = 0);
    
    /** Add an at-most-k constraint to the solver
     *
     *  The given literals will be added as an at-most-k constraint to the
     *  solver as part of the hard clauses. Based on the number of literals and
     *  k, the solver might encode the constraint into CNFs and adds the
     *  corresponding clauses to the formula. Auxiliary variables might be
     *  introduced during encoding. As an alternative, a solver might natively
     *  support this kind of cardinality constraint.
     * 
     *  This method cannot be called anymore once a compute method has been
     *  called.
     *
     *  Note: the produced clauses are always hard clauses. They do not have a
     *        weight. To use weighted constraints, use the addClause method!
     *
     *  Possible error codes:
     *   -ENOMEM ... the additional constraint could not be added to the solver
     *   -EINVAL ... a literal of the clause is greater than the maximal
     *               specified variable, or 0
     *
     *  @param literals list of literals that represent the clause
     *  @param k number of literals that are allowed to be true
     *
     *  @return true, if adding the constraint was ok, false otherwise, e.g.
     *                because of too large variables or groups
     *          false, if adding resulted in a failure, e.g. invalid literals
     *                (check error code!)
     */
    bool addAtMostK(const std::vector<int> &literals, const unsigned k);
    
    /** Compute a MaxSAT solution for the added (weighted) formula
     *
     *  Given all added clauses with their weight (or being hard clauses),
     *  return a model that satisfied all hard clauses, and falsifies the soft
     *  clauses such that the least cost is paid for falsifying the remaining
     *  clauses.
     * 
     *  Note: the returned model might be different among different
     *        implementations of this function, because it is up to the
     *        implementation which solution to report (if multiple are present).
     *
     *  In case the input problem is satisfiable, the variable model will
     *  store the satisfying model, where model[i] stores that truth value for
     *  variable i. For model[i] > 1, variable i is mapped to true,
     *  in case model[i] < 0, variable i is mapped to false.
     *  Note: the field model[0] will be 0, as it does not map a value to a
     *        valid input variable.
     *
     *  If the set of hard clauses (and constraints) is unsatisfiable, the
     *  model will be empty.
     * 
     *  If the optional parameter maxCost is specified, a model is only
     *  accepted as a solution if the caused cost is below the given value,
     *  e.g. cost(model) < maxCost.
     *
     *  If the parameter startAssignment is given, the given integers are
     *  used as initialization to the variable assignments in the search. With
     *  this parameter, the search can be guided towards solutions that have
     *  been found already on similar problems. Note: this might not speed up
     *  the search in general, but usually leads to performance improvements.
     * 
     *  If the optional parameter maxMinimizeSteps is specified, the
     *  computational effort to turn the first reported model (which causes
     *  less costs than specified with maxCost) into a MaxSAT solution will be
     *  limited. In case of a negative integer, the minimization will be run
     *  until completion.
     * 
     *  @param model stores the return result: (optimal) model, or empty
     *  @param cost  stores the resulting cost, in case a model is reported
     *  @param maxCost maximal cost to accept a solution as solution
     *  @param startAssignment list of variable assignments to start with
     *         (optional, might help to speedup computation, invalid literals
     *          will be ignored)
     *  @param maxMinimizeSteps steps to limit effort for optimizing solution
     *
     *  @return status of the search:
     *          OPTIMAL, if an optimal solution could be found, which satisfies
     *                   all clauses at a minimal cost
     *          SATISFIABLE, if the input was satisfiable but the cost could
     *                       not be proven to be minimal (reached some limit)
     *          UNSATISFIABLE, if the hard clauses of the input are
     *                         unsatisfiable
     *          UNKNOWN, if the computation has been interrupted before a first
     *                   satisfying assignment with some cost could be found
     **/
    ReturnCode compute_maxsat(std::vector<int> &model,
                              uint64_t &cost,
                              uint64_t maxCost = UINT64_MAX,
                              const std::vector<int> *startAssignment = 0,
                              int64_t maxMinimizeSteps = -1);
};

#endif
