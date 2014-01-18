/* SIMPLIFY - TAUT.c
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

/* Checks if a basic rule is tautological */
static int find_taut_basic(const RULE *rule) {
	BASIC_RULE *this_rule = rule->data.basic;

	// R'= R \ { h <- B }			if h \in B+
	if (contains_element(this_rule->pos, this_rule->pos_cnt, this_rule->head)) {
		stats->taut_removed[TYPE_BASIC]++;
		stats->any++;
		return 1;
	} else
		return 0;
}

/* Checks if a constraint rule is tautological */
static int find_taut_constraint(const RULE *rule) {
	CONSTRAINT_RULE *this_rule = rule->data.constraint;
	int n;

	// R'= R \ { h <- l{h,B} } U {h <-l{B} }
	if ((n = contains_element(this_rule->pos, this_rule->pos_cnt,
			this_rule->head))) {
		remove_atom_occurrence(rule, &(occtab->pbody)[this_rule->pos[n-1]]);
		remove_element(this_rule->pos, this_rule->pos_cnt, n-1);
		this_rule->pos_cnt--;
		//this_rule->bound--; Bound stays same
		stats->taut_reduced[TYPE_CONSTRAINT]++;
		stats->any++;
	}

	return 0;
}

/* Checks if a choice rule is tautological */
static int find_taut_choice(const RULE *rule) {
	CHOICE_RULE *this_rule = rule->data.choice;
	int i, n=0;

	// R'= R \ { h <- B } U {H\B <- B}			if H \cup B+ != ø
	for (i = 0; i < this_rule->head_cnt; i++) {
		if ((n = contains_element(this_rule->pos, this_rule->pos_cnt,
				this_rule->head[i]))) {
			remove_atom_occurrence(rule, &(occtab->head)[this_rule->head[n-1]]);
			remove_element(this_rule->head, this_rule->head_cnt, n-1);
			this_rule->head_cnt--;
			stats->any++;
		}
	}

	if (this_rule->head_cnt == 0) {
		stats->taut_removed[TYPE_CHOICE]++;
		return 1;
	} else if (n)
		stats->taut_reduced[TYPE_CHOICE]++;
	return 0;

}

/* Checks if a weight rule is tautological */
static int find_taut_weight(const RULE *rule) {
	WEIGHT_RULE *this_rule = rule->data.weight;
	int n;

	// R'= R \ { h <- l[h,B] } U {h <-l[B] }
	if ((n = contains_element(this_rule->pos, this_rule->pos_cnt,
			this_rule->head))) {
		n--; // Decrement to actual position
		//this_rule->bound -= this_rule->weight[n + this_rule->neg_cnt];
		remove_atom_occurrence(rule, &(occtab->pbody)[this_rule->pos[n-1]]);
		remove_element(this_rule->pos, this_rule->pos_cnt, n);
		remove_element(&this_rule->weight[this_rule->neg_cnt],
				this_rule->pos_cnt, n);
		this_rule->pos_cnt--;
		stats->taut_reduced[TYPE_WEIGHT]++;
		stats->any++;
	}

	return 0;
}

/* Checks if a rule is tautological,
 * returns 1 if to be removed, 0 otherwise */
int find_taut(const RULE *rule) {
	int type = rule->type;

	switch (type) {
	case TYPE_BASIC:
		return find_taut_basic(rule);
	case TYPE_CONSTRAINT:
		return find_taut_constraint(rule);
	case TYPE_CHOICE:
		return find_taut_choice(rule);
	case TYPE_WEIGHT:
		return find_taut_weight(rule);
	default:
		return 0;
	}
}
