Norbert Manthey, Copyright 2018, All rights reserved

# smax

This package provides a C++ API to a MaxSAT solver, and includes an example
implementation, as well as a test that show-cases how the interface can be used.

# Implementation

The current implementation is based on an older commit of the

    Open-WBO MaxSAT Solver [1],

with a few modifications added on top to support the required features for
implementing this interface.

# License

The license of this package can be found in the file LICENSE. The MaxSAT solver
picked for the example implementation comes with the MIT license. See [2] for
more details, and below for the relevant license of Open-WBO.

# Getting Started

As this packages comes with a pre-build library, the simplest way to start
testing and using this library is the provided Dockerfile, as well as script
to run commands in the Docker environment. The test case shows how the interface
can be used, as well as how to compile a consumer, and which compile flags to
use.

To compile and test the interface, run the following command (requires docker):

```
./run_in_container.sh Dockerfile test/libso/run.sh
```

This command will build a docker image with the relevant dependencies, based on
an Ubuntu 16.04 environment, and will run the script test/libso/run.sh as the
calling user.

# References

    1. https://github.com/sat-group/open-wbo
    2. https://github.com/sat-group/open-wbo/blob/master/LICENSE

# Licenses of tools used in this package

## Open-WBO

<pre>
MiniSat  -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
            Copyright (c) 2007-2010  Niklas Sorensson
Open-WBO -- Copyright (c) 2013-2017, Ruben Martins, Vasco Manquinho, Ines Lynce

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
</pre>
