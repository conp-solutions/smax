#!/bin/bash
# make-starexec.sh, Norbert Manthey, 2020
#
# build the starexec package from the current git commit

# make sure we notice failures early
set -e -x

# make sure we know where the code is
SOLVERDIR=$(pwd)
BRANCH=$(git rev-parse --short HEAD)

if [ ! -x "$SOLVERDIR"/misc/make-starexec.sh ]
then
	echo "Error: script has to be called from base directory, abort!"
	exit 1
fi

# make sure we clean up
trap 'rm -rf $TMPD' EXIT
TMPD=$(mktemp -d)

# create the project directory
pushd "$TMPD"

# copy template
mkdir -p build

cp -r $SOLVERDIR/misc/starexec .
pushd starexec
mkdir -p code
mkdir -p doc
mkdir -p bin
BIN_PATH=$(readlink -e bin)

# copy actual source by using the git tree, only the current branch
pushd code
git clone "$SOLVERDIR" smax
pushd smax
git checkout $BRANCH
# get and clean open-wbo
cp -r "$SOLVERDIR"/open-wbo .
cd open-wbo
make clean -k ||:
make clean SOLVER=mergesat ||:
# also clean git, as the current change might not be added to the repository yet
git clean -xfdf ||:
git submodule foreach --recursive git clean -xfdf ||:
git gc ||:
git submodule foreach --recursive git gc ||:
git prune ||:
git submodule foreach --recursive git prune ||:
cd ..
# clean git history structure and leftovers
git clean -xfdf ||:
git submodule foreach --recursive git clean -xfdf ||:
git gc
git submodule foreach --recursive git gc ||:
git prune
git submodule foreach --recursive git prune ||:
git remote remove origin || true
git remote add origin https://github.com/conp-solutions/smax.git
popd
popd

# Generate a license stub
cp code/smax/LICENSE > LICENSE .
popd # starexec

pushd build
cp -r ../starexec/code/smax .
cd smax
make clean
make smax
# finally, copy binary into starexec package
cp smax-src/smax_static "$BIN_PATH"
popd # build

# compress relevant starexec part
cd starexec
zip -r -y -9 smax-starexec.zip *

# jump back and move zip file here
popd # $TMPD
mv "$TMPD"/starexec/smax-starexec.zip .
