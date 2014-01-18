/* SIMPLIFY - readwrite.h
 *
 * 2008 Axel Eirola
 */

#define USAGE_STRING "Usage: \
simplify [options] [<file>|-]\n\
Options:\n\
  -h, --help    print help message\n\
  -V, --version print version information\n\
  -v, --verbose verbose mode (\"human\" readable including statistics)\n\
  -S, --stats   print transformation statistics to stderr\n\
  \n\
  -t, --taut    remove tautologies (d)\n\
  -c, --contra  remove contradictions (d)\n\
  -T, --trivial reduce trivial rules (d)\n\
  -C, --compute remove compute atoms (d)\n\
  -n, --nonmin  remove non-minimal (d)\n\
  -l, --litnon  remove literal non-minimal (d)\n\
  -p, --partev  partially evaluate simple unique heads\n\
  -P, --partev-full  partially evaluate all unique heads\n\
  -L, --loop    loop transformations until exhausted\n\
  -s, --smodels run program through smodels -internal\n\
  \n\
  -A, --all     turn on all of above\n\
  -N, --none    turn off all of above\n\
  \n\
(d) denotes that the flag is turned on by default, the default argument would\
thus be -NtcTCnl\n\
Arguments parsed in order, so that simplify -cNt would only remove tautologies.\n\
"
