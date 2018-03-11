/***************************************************************************************[WDimacs.h]

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

#ifndef SMax_WDimacs_h
#define SMax_WDimacs_h

#include <stdio.h>

#include "utils/ParseUtils.h"
#include "core/Dimacs.h"
#include "core/SolverTypes.h"

#include "include/MaxSATSolver.h"

#include <vector>

namespace SMax {

//=================================================================================================
// DIMACS Parser:

template<class B, class Solver>
static void readWClause(B& in, Solver& S, std::vector<int>& lits, int64_t& weight) {
    int     parsed_lit;
    lits.clear();
    weight = parseInt(in);
    for (;;){
        parsed_lit = parseInt(in);
        if (parsed_lit == 0) break;
        lits.push_back(parsed_lit);
    }
}

template<class B, class Solver>
static void readClause(B& in, Solver& S, std::vector<int>& lits) {
    int     parsed_lit;
    lits.clear();
    for (;;){
        parsed_lit = parseInt(in);
        if (parsed_lit == 0) break;
        lits.push_back(parsed_lit);
    }
}

template<class B>
static MaxSATSolver *parse_WDIMACS_main(B& in, int64_t &top_weight) {
    std::vector<int> lits;
    int64_t weight = 0;
    int vars    = 0;
    int clauses = 0;
    int cnt     = 0;
    MaxSATSolver *solver = nullptr;

    for (;;){
        skipWhitespace(in);
        if (*in == EOF) break;
        else if (*in == 'p'){ // find header
	    ++in;
	    skipWhitespace(in);
	    if(*in == 'w')
	    {
	      if (eagerMatch(in, "wcnf")){
		vars    = parseInt(in);
                clauses = parseInt(in);
		top_weight  = parseInt(in);
	      } else {
		printf("PARSE ERROR! Unexpected char: %c while looking for 'p gcnf'\n", *in), exit(3);
	      }
	    }
	    else if(*in == 'c')
	    {
	      if (eagerMatch(in, "cnf")){
		vars    = parseInt(in);
                clauses = parseInt(in);
		top_weight  = -1; // this is a plain CNF instance
	      } else {
		printf("PARSE ERROR! Unexpected char: %c while looking for 'p cnf'\n", *in), exit(3);
	      }
	    } else {
                printf("PARSE ERROR! Unexpected char: %c\n", *in), exit(3);
            }
            if(solver != nullptr) {
	       printf("PARSE ERROR! Found two header lines, unexpected char: %c\n", *in), exit(3);
	    }
	    solver = new MaxSATSolver(vars, clauses);
        } else if (*in == 'c' || *in == 'p')
            skipLine(in);
        else{
	    if(solver == nullptr) {
	      printf("PARSE ERROR! Unexpected char: %c while looking for 'p cnf' Require header before first clause\n", *in), exit(3);
	    }
            cnt++;
	    if(top_weight != -1) readWClause(in, *solver, lits, weight);
	    else { // plain CNF, just number the clauses
	      readClause(in, *solver, lits);
	      weight = 1;
	    }
            /* try to add clause, exit in case of failure*/
            if( ! solver->addClause(lits, weight) )
	    {
	      printf("PARSE ERROR! Failure while adding clause, error number %d\n", solver->getErrno()), exit(3);
	    }
	}
    }

    if (cnt  != clauses)
        fprintf(stderr, "WARNING! WDIMACS header mismatch: wrong number of clauses.\n");
    
    return solver;
}

// Inserts problem into solver.
//
static MaxSATSolver *parse_WDIMACS(gzFile input_stream, int64_t &top_weight) {
    NSPACE::StreamBuffer in(input_stream);
    return parse_WDIMACS_main(in, top_weight); }

//=================================================================================================
}

#endif
