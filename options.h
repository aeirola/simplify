/* SIMPLIFY - options.h
 *
 * Options datastructure, stores boolen values of input parameters.
 *
 * 2008 Axel Eirola
 */

typedef struct OPT_struct {
	int triv;
	int compute;
	int taut;
	int contra;

	int nonmin;
	int litnonmin;

	int partev;
	int partev_full;

	int smodels;

	int verbose;
	int stats;
	int loop;

} OPT;
