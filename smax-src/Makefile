#
# Makefile to depend on an external solver directory.
# This way, other SAT backends can be tested easily.
#

# setup the solver to be used as a single variable
SOLVER ?= minisat

# include the solver.mk file from the solvers directory
include ../open-wbo/solvers/$(SOLVER).mk

# THE REMAINING OF THE MAKEFILE SHOULD BE LEFT UNCHANGED
EXEC       = smax
LIB        = smax

# from minisat
DEPDIR     = mtl utils core simp

# from open-wbo
DEPDIR     += ../../../open-wbo
DEPDIR     += ../../../open-wbo/encodings
DEPDIR     += ../../../open-wbo/algorithms
DEPDIR     += ../../../open-wbo/graph
DEPDIR     += ../../../open-wbo/classifier

# Point to minisat directory
MROOT      = $(PWD)/../open-wbo/solvers/$(SOLVERDIR)

# Set compile and link flags
CFLAGS     += -I .. -Wall -Wno-parentheses -std=c++11 -DNSPACE=$(NSPACE) -DSOLVERNAME=$(SOLVERNAME) -DVERSION=$(VERSION)
# Set a flag to identify the used solver during compile time
CFLAGS     += -DUSE_$(SOLVER)
LFLAGS     += -lgmpxx -lgmp
LFLAGS     += -Wall $(EXTRA_LFLAGS) 

# Make sure, all classes that are not in the API are not present in the .so file (after strip -x)
CFLAGS    += -fvisibility=hidden

# Use common flags that can be used by other builds as well, e.g. tests
include common.mk

# make sure the CLI flags overwrite the others
CFLAGS    +=  $(EXTRA_CFLAGS)

include $(MROOT)/mtl/template.mk
