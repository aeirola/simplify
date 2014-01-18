/* SIMPLIFY - stats.h
 *
 * Includes datastructure for storing simplification statistics.
 *
 * 2008 Axel Eirola
 */

#define TYPES 6

typedef struct struct_stat {
	int triv_reduced[TYPES];
	int triv_removed[TYPES];

	int compute_reduced[TYPES];
	int compute_removed[TYPES];

	int taut_reduced[TYPES];
	int taut_removed[TYPES];

	int contra_reduced[TYPES];
	int contra_removed[TYPES];

	int nonmin_reduced[TYPES];
	int nonmin_removed[TYPES];

	int litnonmin_reduced[TYPES];
	int litnonmin_removed[TYPES];

	int partev_reduced[TYPES];
	int partev_removed[TYPES];

	int loops;
	int any;


} STAT;
