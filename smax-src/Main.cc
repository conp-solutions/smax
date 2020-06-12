/***************************************************************************************[Main.cc]

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

#include <errno.h>

#include <signal.h>
#include <zlib.h>
#include <sys/resource.h>

#include "utils/System.h"
#include "utils/ParseUtils.h"
#include "utils/Options.h"

#include "include/MaxSATSolver.h"

#include "smax-src/WDimacs.h"

using namespace SMax;
using namespace NSPACE;

//=================================================================================================

static MaxSATSolver* solver;
// Terminate by notifying the solver and back out gracefully. This is mainly to have a test-case
// for this feature of the Solver as it may take longer than an immediate call to '_exit()'.
static void SIGINT_interrupt(int signum) { 
  // (void)signum; solver->interrupt();
}

// Note that '_exit()' rather than 'exit()' has to be used. The reason is that 'exit()' calls
// destructors and may cause deadlocks if a malloc/free function happens to be running (these
// functions are guarded by locks for multithreaded use).
static void SIGINT_exit(int signum) {
    (void)signum; 
    printf("\n"); printf("*** INTERRUPTED ***\n");
    _exit(1); }


//=================================================================================================
// Main:

int main(int argc, char** argv)
{

      printf("c\nc This is smax -- based on MiniSAT\nc\n");
      
      setUsageHelp("c USAGE: %s [options] <input-file> <result-output-file>\n\n  where input may be either in plain or gzipped GDIMACS.\n");
        
        
#if defined(__linux__)
        fpu_control_t oldcw, newcw;
        _FPU_GETCW(oldcw); newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE; _FPU_SETCW(newcw);
        //printf("c WARNING: for repeatability, setting FPU to use double precision\n");
#endif
        // Extra options:
        //
        IntOption    verb   ("MAIN", "verb",   "Verbosity level (0=silent, 1=some, 2=more).", 1, IntRange(0, 2));
        IntOption    cpu_lim("MAIN", "cpu-lim","Limit on CPU time allowed in seconds.\n", INT32_MAX, IntRange(0, INT32_MAX));
        IntOption    mem_lim("MAIN", "mem-lim","Limit on memory usage in megabytes.\n", INT32_MAX, IntRange(0, INT32_MAX));

	IntOption    maxMinimizeSteps("SMAX", "maxSteps","Limit number of conflicts per SAT call.\n", -1, IntRange(-1, INT32_MAX));

        parseOptions(argc, argv, true);
        
        MaxSATSolver *S;
        double      initial_time = cpuTime();

        // Use signal handlers that forcibly quit until the solver will be able to respond to
        // interrupts:
        signal(SIGINT, SIGINT_exit);
        signal(SIGXCPU,SIGINT_exit);


        // Set limit on CPU-time:
        if (cpu_lim != INT32_MAX){
            rlimit rl;
            getrlimit(RLIMIT_CPU, &rl);
            if (rl.rlim_max == RLIM_INFINITY || (rlim_t)cpu_lim < rl.rlim_max){
                rl.rlim_cur = cpu_lim;
                if (setrlimit(RLIMIT_CPU, &rl) == -1)
                    printf("c WARNING! Could not set resource limit: CPU-time.\n");
            } }

        // Set limit on virtual memory:
        if (mem_lim != INT32_MAX){
            rlim_t new_mem_lim = (rlim_t)mem_lim * 1024*1024;
            rlimit rl;
            getrlimit(RLIMIT_AS, &rl);
            if (rl.rlim_max == RLIM_INFINITY || new_mem_lim < rl.rlim_max){
                rl.rlim_cur = new_mem_lim;
                if (setrlimit(RLIMIT_AS, &rl) == -1)
                    printf("c WARNING! Could not set resource limit: Virtual memory.\n");
            } }
        
        if (argc == 1)
            printf("c Reading from standard input... Use '--help' for help.\n");

        gzFile in = (argc == 1) ? gzdopen(0, "rb") : gzopen(argv[1], "rb");
        if (in == NULL)
            printf("ERROR! Could not open file: %s\n", argc == 1 ? "<stdin>" : argv[1]), exit(1);
        

    try {
	int64_t top_weight;
        S = parse_WDIMACS(in, top_weight);
        solver = S;
        gzclose(in);

        double parsed_time = cpuTime();
        if(verb>0) printf("c parse time:           %12.2f s\n", parsed_time - initial_time);

        // Change to signal-handlers that will only notify the solver and allow it to terminate
        // voluntarily:
        signal(SIGINT, SIGINT_interrupt);
        signal(SIGXCPU,SIGINT_interrupt);

	if(verb>0) printf("c start search with top weight %lu\n", top_weight < 0 ? UINT64_MAX : top_weight);
	std::vector<int> model;
	
	MaxSATSolver::ReturnCode maxsat_ret;
	uint64_t cost = ~0UL;
	maxsat_ret = S->compute_maxsat(model, cost, top_weight < 0 ? UINT64_MAX : top_weight, 0, maxMinimizeSteps);
	if(verb>0) printf("c return from search with status %d, and model size %lu\n", maxsat_ret, model.size());

	int retStatus = 0;
	switch(maxsat_ret) {
	case MaxSATSolver::ReturnCode::UNSATISFIABLE:
	  printf("s UNSATISFIABLE\n");
	  retStatus = 20;
	  break;
	case MaxSATSolver::ReturnCode::OPTIMAL:
	case MaxSATSolver::ReturnCode::UNKNOWN:
	  if(maxsat_ret == MaxSATSolver::ReturnCode::UNKNOWN && model.size() == 0) {
	    printf("s UNKNOWN\n");
	    break;
	  }
	  /* print S line */
	  if(maxsat_ret == MaxSATSolver::ReturnCode::OPTIMAL) printf("s OPTIMAL\no %" PRIu64 "\n", cost);
	  else printf("s SATISFIABLE\n"); /* even if unknown, as we already found a model */
	  
	  /* print model */
	  printf("v ");
	  for(unsigned index = 1; index < model.size(); ++ index)
	    printf("%d ", model[index]);
	  printf("0\n");
	  retStatus = 30;
	  break;
	case MaxSATSolver::ReturnCode::ERROR:
	default:
	  printf("s ERROR\n");
	  printf("c some error occurred, abort\n");
	}
	
	// simple stats
	double cpu_time = cpuTime();
	double mem_used = memUsedPeak();
	printf("c Stats: Memory : %.2f MB, CPU : %g s\n", mem_used, cpu_time);
	
        delete S;
        return retStatus;
    } catch (OutOfMemoryException&){
	        printf("c =========================================================================================================\n");
        printf("s UNKNOWN\n");
        delete S;
        exit(0);
    }

}
