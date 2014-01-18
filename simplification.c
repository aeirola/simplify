/* SIMPLIFY - simplification.c
 *
 * Includes handling and calling of simplification functions as well as removing
 * rules depending on returnvalues of said functions. Used simplifications are
 * are found under the transformations subfolder.
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"
#include "transformations/compute.h"

#include "simplification.h"
#include "normalize.h"
#include "transformations/trivial.h"
#include "misc.h"
#include "transformations/partev.h"
#include "transformations/taut.h"
#include "transformations/contra.h"
#include "transformations/nonmin.h"
#include "transformations/litnonmin.h"
#include "options.h"
extern OPT *options;
#include "stats.h"
extern STAT *stats;

#include "occurrences.h"
extern OCCTAB *occtab;
extern ATAB *atab;

/* Main logical function called by program main function.
 *
 * Creates required datastructures and manages executionorder of simplification-
 * algorithms found and explained in respective source-files.
 */
void simplify(RULE **program) {

	RULE *first = *program;
	RULE *r = first;
	RULE *p = first;

	int remove = 0; // Remove current rule?

	/* Normalize rules */
	while (r) {
		if (normalize(r)) {
			remove_rule(r);
			if (r == first)
				first = r->next;
			else
				p->next = r->next;
		} else
			p = r;

		r = r->next;
	}

	do {
		/* Reset stats */
		stats->any = 0;

		/* Remove computes */
		if (options->compute)
			remove_computes(atab, occtab);

		r = first;
		while (r) {
			remove = 0;

			// Remove rule if trivial
			if (options->triv)
				if (find_trivial(r))
					remove = 1;

			// Remove rule if tautological
			if (!remove && options->taut)
				if (find_taut(r))
					remove = 1;

			// Remove rule if contradictional
			if (!remove && options->contra)
				if (find_contra(r))
					remove = 1;

			// Remove rule if partially evaluatedly untrue
			if (!remove && options->partev)
				if (partev(r))
					remove = 1;

			// Remove rule if nonminimal
			if (!remove && options->nonmin)
				if (remove_nonmin(r))
					remove = 1;

			// Remove rule if literaly nonminimal
			if (!remove && options->litnonmin)
				if (remove_litnonmin(r))
					remove = 1;

			// Renormalize
			if (!remove && normalize(r))
				remove = 1;

			int head = get_head(r);
			// Remove rule, if head never used
			if (!remove && options->partev && head && atab->names[head]
					== NULL && (occtab->head)[head].rule_cnt <= 1
					&& (occtab->pbody)[head].rule_cnt < 1
					&& (occtab->nbody)[head].rule_cnt < 1) {
				stats->partev_removed[r->type]++;
				remove = 1;
			}

			// Remove rule if set to be removed
			if (!remove && r->type < 0)
				remove = 1;

			// Remove if set to be removed
			if (remove) {
				remove_rule(r);
				if (r == first)
					first = r->next;
				else
					p->next = r->next;
			} else
				p = r;

			/* Go to next */
			r = r->next;
		}

		stats->loops++;

	} while (options->loop && stats->any); // Loop again if something changed


	// Unlist removed rules
	r = first;
	while (r) {
		if (r->type < 0
		// Check that we output valid constraint rules
				|| (r->type == TYPE_CONSTRAINT && r->data.constraint->bound
						> r->data.constraint->neg_cnt
								+ r->data.constraint->pos_cnt)
		// and weight rules
				|| (r->type == TYPE_WEIGHT && r->data.weight->bound > sum(
						r->data.weight->neg, r->data.weight->neg_cnt
								+ r->data.weight->pos_cnt))) {
			if (r == first)
				first = r->next;
			else
				p->next = r->next;
		} else
			p = r;
		r = r->next;
	}

	// Pass back first rule of program
	*program = first;

	return;
}
