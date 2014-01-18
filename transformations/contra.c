/* SIMPLIFY - CONTRA.c
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"

#include "helpers.h"
#include "../stats.h"
extern STAT *stats;
#include "../occurrences.h"
extern OCCTAB *occtab;

/* Checks if a basic rule is contradictory */
static int find_contra_basic(const RULE *rule) {
	BASIC_RULE *this_rule = rule->data.basic;

	// R'= R \ { h <- B }			if B+ \cap B- != ø
	if (has_common_element(this_rule->neg, this_rule->neg_cnt, this_rule->pos,
			this_rule->pos_cnt)) {
		stats->contra_removed[TYPE_BASIC]++;
		stats->any++;
		return 1;
	} else
		return 0;
}

/* Checks if a constraint rule is contradictory */
/*static int find_contra_constraint(RULE *rule) {
	CONSTRAINT_RULE *this_rule = rule->data.constraint;

	int a, n, p;
	if ((a = has_common_element(this_rule->neg, this_rule->neg_cnt,
			this_rule->pos, this_rule->pos_cnt)) && (n = contains_element(
			this_rule->neg, this_rule->neg_cnt, a)) && (p = contains_element(
			this_rule->pos, this_rule->pos_cnt, a))) {

		remove_atom_occurrence(rule, &(occtab->nbody)[this_rule->neg[n-1]]);
		remove_element(this_rule->neg, this_rule->neg_cnt-- + this_rule->pos_cnt, n-1);
		this_rule->pos -= sizeof(int);

		remove_atom_occurrence(rule, &(occtab->pbody)[this_rule->pos[p-1]]);
		remove_element(this_rule->pos, this_rule->pos_cnt--, p-1);

		this_rule->bound--;
		stats->contra_reduced[TYPE_CONSTRAINT]++;
		stats->any++;
	}

	return 0;
}*/

/* Checks if a choice rule is contradictory */
static int find_contra_choice(const RULE *rule) {
	CHOICE_RULE *this_rule = rule->data.choice;

	// R'= R \ { H <- B }			if B+ \cap B- != ø
	if (has_common_element(this_rule->neg, this_rule->neg_cnt, this_rule->pos,
			this_rule->pos_cnt)) {
		stats->contra_removed[TYPE_CHOICE]++;
		stats->any++;
		return 1;
	} else
		return 0;
}

/* Checks if a weight rule is contradictory */
/*static int find_contra_weight(RULE *rule) {
	WEIGHT_RULE *this_rule = rule->data.weight;

	int a, n, p;
	if ((a = has_common_element(this_rule->neg, this_rule->neg_cnt,
			this_rule->pos, this_rule->pos_cnt)) && (n = contains_element(
			this_rule->neg, this_rule->neg_cnt, a)) && (p = contains_element(
			this_rule->pos, this_rule->pos_cnt, a))) {
		int wn = this_rule->weight[n-1];
		int wp = this_rule->weight[n-1 + this_rule->neg_cnt];

		//Remove rule with smaller weight and decrease limit and weight of
		//remaining rule by difference in weight
		if (wn < wp) {
			remove_element(this_rule->weight, this_rule->neg_cnt
					+this_rule->pos_cnt, n-1);

			remove_atom_occurrence(rule, &(occtab->nbody)[this_rule->neg[n-1]]);
			remove_element(this_rule->neg, this_rule->neg_cnt-- + this_rule->pos_cnt, n-1);
			this_rule->pos -= sizeof(int);
			this_rule->weight[this_rule->neg_cnt + p-1] -= wn;
			this_rule->bound -= wn;
		} else if (wn > wp) {
			remove_element(&this_rule->weight[this_rule->neg_cnt],
					this_rule->pos_cnt, p-1);

			remove_atom_occurrence(rule, &(occtab->pbody)[this_rule->pos[n-1]]);
			remove_element(this_rule->pos, this_rule->pos_cnt--, p-1);
			this_rule->weight[n-1] -= wp;
			this_rule->bound -= wp;
		} else if (wn == wp) {
			remove_element(this_rule->weight, this_rule->neg_cnt, n-1);
			remove_element(&this_rule->weight[this_rule->neg_cnt],
					this_rule->pos_cnt, p-1);

			remove_atom_occurrence(rule, &(occtab->nbody)[this_rule->neg[n-1]]);
			remove_element(this_rule->neg, this_rule->neg_cnt-- + this_rule->pos_cnt, n-1);
			this_rule->pos -= sizeof(int);
			remove_atom_occurrence(rule, &(occtab->pbody)[this_rule->pos[n-1]]);
			remove_element(this_rule->pos, this_rule->pos_cnt--, p-1);
			this_rule->bound -= wn;
		}

		stats->contra_reduced[TYPE_WEIGHT]++;
		stats->any++;
	}

	return 0;
}*/

/* Checks if a rule is contradictory,
 * returns 1 if to be removed, 0 otherwise */
int find_contra(RULE *rule) {
	int type = rule->type;

	switch (type) {
	case TYPE_BASIC:
		return find_contra_basic(rule);
		// Constraint and weight transformations removed because not proven true
//	case TYPE_CONSTRAINT:
//		return find_contra_constraint(rule);
	case TYPE_CHOICE:
		return find_contra_choice(rule);
//	case TYPE_WEIGHT:
//		return find_contra_weight(rule);
	default:
		return 0;
	}
}
