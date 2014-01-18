#!/bin/bash
##
#  verify.sh
#
#   verify.sh [argument] <file>
#     OR
#   lparse >program> | verify.sh [argument]
#
# Verifies the work of simplify by checking for equivalence between the
# original program and the simplified one.
#
# Reads the smodels formated program either form stdin or from a file
# given as the first parameter if no argument is given, otherwise the
# second parameter is assumed as input file.
#
# One argument can be given to verify before the file, as only one argument
# can be give it is recomended to use the shorthand syntax (eg. -NCr)
#
# Requires lpeq and either clasp or smodels in PATH to function properly.
#
# 
#  2008, Axel Eirola
#

# Clean up remporary files
i_cleanup() {
 rm -rf $simp_tempfile
 rm -rf $prog_tempfile 
 echo -ne \\a	# Uncomment this line if the beeper is killing you
 exit $1
}


# Create temporary files
simp_tempfile=`mktemp -t verify_simp.XXXXXX` || {
 echo "Could not create temp file, exiting."
 exit 1
}
prog_tempfile=`mktemp -t verify_org.XXXXXX`  || {
 echo "Could not create temp file, exiting."
 exit 1
}

# Check program availability
if which clasp > /dev/null
then
 echo "Clasp found, using clasp."
 solver="clasp 1"

elif which smodels > /dev/null
then
 echo "Smodels found, using smodels."
 solver="smodels 1"

else
 echo "No solvers found, exiting."
 i_cleanup 1
fi

# Check from where to input
if [ $# = 0 ]
then
 echo "Reading from stdin."
 prog=$prog_tempfile
 while read data; do
  echo $data >> $prog
 done
elif [ $# = 1 ]
then
 if [ ${1:0:1} == '-' ]
 then
  echo "Reading from stdin."
  prog=$prog_tempfile
  while read data; do
   echo $data >> $prog
  done
  arg=$1
 else
  echo "Reading form file $1"
  prog=$1
  arg=""
 fi
elif [ $# = 2 ]
 then
 echo "Reading form file $2"
 prog=$2
 arg=$1
fi

# Check that file exists
if [ ! -f $prog ]
then 
 echo "File not found, exiting."
 i_cleanup
fi

# Do simplification
simp=$simp_tempfile
simplify $arg $prog > $simp

# Check that simplification succeeded
if [ ! -f $simp ] || [ ! $simp ]
then
 echo "Simplify failed, exiting."
 i_cleanup
fi
  

models=1

# Run tests
echo -n "Testing new models against original: "
models=`lpeq $prog $simp | $solver | grep -c "^Answer: "`
if [ $models ] && [ $models == 0 ]
 then
  echo -e "\E[32mSuccessful!\E[0m"
 else
  echo -e "\E[1;31mFailed!\E[0m"
  i_cleanup 1
fi

echo -n "Testing original models against new: "
models=`lpeq $simp $prog | $solver | grep -c "^Answer: "`
 if [ $models ] && [ $models == 0 ]
 then
  echo -e "\E[32mSuccessful!\E[0m"
 else
  echo -e "\E[1;31mFailed!\E[0m"
  i_cleanup 1
fi

echo -e "\nAll tests \E[1;32mpassed\E[0m, programs are equivalent"

i_cleanup 0

