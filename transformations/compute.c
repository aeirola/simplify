/* SIMPLIFY - compute.c
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"

#include "compute.h"
#include "helpers.h"
#include "../misc.h"
#include "../stats.h"
extern STAT *stats;

#include "../occurrences.h"
extern OCCTAB *occtab;
extern ATAB *atab;

/* Positive atoms not removed because transformation not proven to be true in
 * all cases */

/* Check negative compute atoms against positive body */
static int remove_neg_pos(RULE *rule, int atom) {
	switch (rule->type) {
	case TYPE_BASIC:
		stats->compute_removed[TYPE_BASIC]++;
		stats->any++;
		return 1;

	case TYPE_CONSTRAINT:
		/*remove_element(this_rule->pos, this_rule->pos_cnt, n-1);
		 this_rule->pos_cnt--;
		 stats->compute_atoms_removed++;*/
		return 0;

	case TYPE_CHOICE:
		stats->compute_removed[TYPE_CHOICE]++;
		stats->any++;
		return 1;

	case TYPE_WEIGHT:
		/*remove_element(this_rule->pos, this_rule->pos_cnt, n-1);
		 remove_element(this_rule->weight, this_rule->neg_cnt
		 +this_rule->pos_cnt, n-1);
		 this_rule->pos_cnt--;
		 stats->compute_atoms_removed++;*/
		return 0;

	default:
		return 0;
	}
}

/* Check negative compute atoms against negative body */
static int remove_neg_neg(RULE *rule, int atom) {
	switch (rule->type) {
	case TYPE_BASIC: {
		BASIC_RULE *r = rule->data.basic;
		int n = contains_element(r->neg, r->neg_cnt, atom);
		if (n) {
			remove_atom_occurrence(rule, &(occtab->nbody)[r->neg[n - 1]]);
			remove_element(r->neg, r->neg_cnt--, n - 1);

			stats->compute_reduced[TYPE_BASIC]++;
			stats->any++;
		}
		return 0;

	}
	case TYPE_CONSTRAINT: {
		CONSTRAINT_RULE *r = rule->data.constraint;
		int n = contains_element(r->neg, r->neg_cnt, atom);
		if (n) {
			remove_atom_occurrence(rule, &(occtab->nbody)[r->neg[n - 1]]);
			remove_element(r->neg, r->neg_cnt--, n - 1);

			// Decrement constraint bound
			r->bound--;

			stats->compute_reduced[TYPE_CONSTRAINT]++;
			stats->any++;
		}
		return 0;

	}
	case TYPE_CHOICE: {
		CHOICE_RULE *r = rule->data.choice;
		int n = contains_element(r->neg, r->neg_cnt, atom);
		if (n) {
			remove_atom_occurrence(rule, &(occtab->nbody)[r->neg[n - 1]]);
			remove_element(r->neg, r->neg_cnt--, n - 1);

			stats->compute_reduced[TYPE_CHOICE]++;
			stats->any++;
		}
		return 0;

	}
	case TYPE_WEIGHT: {
		WEIGHT_RULE *r = rule->data.weight;
		int n = contains_element(r->neg, r->neg_cnt, atom);
		if (n) {
			remove_atom_occurrence(rule, &(occtab->nbody)[r->neg[n - 1]]);
			remove_element(r->neg, r->neg_cnt, n - 1);
			r->bound -= r->weight[n - 1];

			// Remove corrseponding weight
			remove_element(r->weight, r->neg_cnt + r->pos_cnt, n - 1);

			r->neg_cnt--;
			stats->compute_reduced[TYPE_WEIGHT]++;
			stats->any++;
		}
		return 0;

	}
	default:
		return 0;
	}
}

/* Check positive compute atoms against positive body */
static int remove_pos_pos(RULE *rule, int atom) {
	switch (rule->type) {
	case TYPE_BASIC:
		/*remove_element(this_rule->pos, this_rule->pos_cnt, n-1);
		 this_rule->pos_cnt--;
		 stats->compute_atoms_removed++;*/
		return 0;

	case TYPE_CONSTRAINT:
		/*	 remove_element(this_rule->pos, this_rule->pos_cnt, n-1);
		 this_rule->pos_cnt--;
		 this_rule->bound--;
		 stats->compute_atoms_removed++;*/
		return 0;

	case TYPE_CHOICE:
		/*remove_element(this_rule->pos, this_rule->pos_cnt, n-1);
		 this_rule->pos_cnt--;
		 stats->compute_atoms_removed++;*/
		return 0;

	case TYPE_WEIGHT:
		/*remove_element(this_rule->pos, this_rule->pos_cnt, n-1);
		 this_rule->bound -= this_rule->weight[n-1+this_rule->neg_cnt];
		 remove_element(&this_rule->weight[this_rule->neg_cnt],
		 this_rule->pos_cnt, n-1);
		 this_rule->pos_cnt--;
		 stats->compute_atoms_removed++;*/
		return 0;

	default:
		return 0;
	}
}

/* Check positive compute atoms against negative body */
static int remove_pos_neg(RULE *rule, int atom) {
	switch (rule->type) {
	case TYPE_BASIC:
		stats->compute_removed[TYPE_BASIC]++;
		stats->any++;
		return 1;

	case TYPE_CONSTRAINT: {
		CONSTRAINT_RULE *r = rule->data.constraint;
		int n = contains_element(r->neg, r->neg_cnt, atom);
		if (n) {
			remove_atom_occurrence(rule, &(occtab->nbody)[r->neg[n - 1]]);
			remove_element(r->neg, r->neg_cnt--, n - 1);

			stats->compute_reduced[TYPE_CONSTRAINT]++;
			stats->any++;
		}
		return 0;

	}
	case TYPE_CHOICE:
		stats->compute_removed[TYPE_CHOICE]++;
		stats->any++;
		return 1;

	case TYPE_WEIGHT: {
		WEIGHT_RULE *r = rule->data.weight;
		int n = contains_element(r->neg, r->neg_cnt, atom);
		if (n) {
			remove_atom_occurrence(rule, &(occtab->nbody)[r->neg[n - 1]]);
			remove_element(r->neg, r->neg_cnt, n - 1);
			// Remove corrseponding weight
			remove_element(r->weight, r->neg_cnt + r->pos_cnt, n - 1);
			r->neg_cnt--;

			stats->compute_reduced[TYPE_WEIGHT]++;
			stats->any++;
		}
		return 0;

	}
	default:
		return 0;
	}
}

// Removes removable computes from
void remove_computes(ATAB *atab, OCCTAB *occtab) {

	int i, j;
	// Get computes
	for (i = 0; i < atab->count; i++)
		// Negative compute
		if (get_status(atab, i) == MARK_FALSE) {
			// Positive body
			for (j = 0; j < (occtab->pbody)[i].size; j++)
				if ((occtab->pbody)[i].rules[j] && remove_neg_pos(
						(occtab->pbody)[i].rules[j], i))
					remove_rule((occtab->pbody)[i].rules[j]);
			// Negative body
			for (j = 0; j < (occtab->nbody)[i].size; j++)
				if ((occtab->nbody)[i].rules[j] && remove_neg_neg(
						(occtab->nbody)[i].rules[j], i))
					remove_rule((occtab->nbody)[i].rules[j]);
		}
		// Positive compute
		else if (get_status(atab, i) == MARK_TRUE) {
			// Positive body
			for (j = 0; j < (occtab->pbody)[i].size; j++)
				if ((occtab->pbody)[i].rules[j] && remove_pos_pos(
						(occtab->pbody)[i].rules[j], i))
					remove_rule((occtab->pbody)[i].rules[j]);
			// Negative body
			for (j = 0; j < (occtab->nbody)[i].size; j++)
				if ((occtab->nbody)[i].rules[j] && remove_pos_neg(
						(occtab->nbody)[i].rules[j], i))
					remove_rule((occtab->nbody)[i].rules[j]);
		}

}

