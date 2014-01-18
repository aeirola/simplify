/* SIMPLIFY - trivial.c
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"

#include "../stats.h"
extern STAT *stats;
#include "../occurrences.h"
extern OCCTAB *occtab;

/* Reduces a trivial constraint rule */
static int find_trivial_constraint(RULE *rule) {
	CONSTRAINT_RULE *this_rule = rule->data.constraint;

	// R'= R \ {h <- l{B} } U {h}				if l = 0
	if (this_rule->bound == 0) {
		rule->data.basic = malloc(sizeof(BASIC_RULE));
		rule->data.basic->head = this_rule->head;
		rule->data.basic->pos = NULL;
		rule->data.basic->pos_cnt = 0;
		rule->data.basic->neg = NULL;
		rule->data.basic->neg_cnt = 0;
		rule->type = 1;

		int i;
		for (i = 0; i < this_rule->neg_cnt; i++)
			remove_atom_occurrence(rule, &(occtab->nbody)[this_rule->neg[i]]);
		for (i = 0; i < this_rule->pos_cnt; i++)
			remove_atom_occurrence(rule, &(occtab->pbody)[this_rule->pos[i]]);

		free(this_rule->neg);
		free(this_rule);
		stats->triv_reduced[TYPE_CONSTRAINT]++;
		stats->any++;
		return 0;

		// R'= R \ {h <- l{B} } U {h <- B}		if l = |B|
	} else if (this_rule->bound == this_rule->neg_cnt + this_rule->pos_cnt) {
		rule->data.basic = malloc(sizeof(BASIC_RULE));
		rule->data.basic->head = this_rule->head;
		rule->data.basic->pos = this_rule->pos;
		rule->data.basic->pos_cnt = this_rule->pos_cnt;
		rule->data.basic->neg = this_rule->neg;
		rule->data.basic->neg_cnt = this_rule->neg_cnt;
		rule->type = 1;
		free(this_rule);
		stats->triv_reduced[TYPE_CONSTRAINT]++;
		stats->any++;
		return 0;

		// R'= R \ {h <- l{B} }					if l > |B|
	} else if (this_rule->bound > this_rule->neg_cnt + this_rule->pos_cnt) {
		stats->triv_removed[TYPE_CONSTRAINT]++;
		stats->any++;
		return 1;
	}

	return 0;
}

/* Reduces a trivial weight rule */
static int find_trivial_weight(RULE *rule) {
	WEIGHT_RULE *this_rule = rule->data.weight;

	// R'= R \ {h <- l[B] } U {h}				if l = 0
	if (this_rule->bound == 0) {
		rule->data.basic = malloc(sizeof(BASIC_RULE));
		rule->data.basic->head = this_rule->head;
		rule->data.basic->pos = NULL;
		rule->data.basic->pos_cnt = 0;
		rule->data.basic->neg = NULL;
		rule->data.basic->neg_cnt = 0;
		rule->type = 1;

		int i;
		for (i = 0; i < this_rule->neg_cnt; i++)
			remove_atom_occurrence(rule, &(occtab->nbody)[this_rule->neg[i]]);
		for (i = 0; i < this_rule->pos_cnt; i++)
			remove_atom_occurrence(rule, &(occtab->pbody)[this_rule->pos[i]]);

		free(this_rule->neg);
		free(this_rule);
		stats->triv_reduced[TYPE_WEIGHT]++;
		stats->any++;
		return 0;
	}

	// Get total weight and smallest weight
	int i, total_weight = 0;
	int lightest = this_rule->weight[0];
	for (i = 0; i < this_rule->neg_cnt + this_rule->pos_cnt; i++) {
		total_weight += this_rule->weight[i];
		if (this_rule->weight[i] < lightest)
			lightest = this_rule->weight[i];
	}

	// R'= R \ { h <- l[B] }						if l > Sum(W(B))
	if (this_rule->bound > total_weight) {
		stats->triv_removed[TYPE_WEIGHT]++;
		stats->any++;
		return 1;
	}

	// Reduce to basic rules if clear that all body literals need be satisfied
	// R'= R \ { h <- l[B] } U { h <- B }			if l > Sum(W(B))-lightest
	if (this_rule->bound > total_weight - lightest) {
		rule->data.basic = malloc(sizeof(BASIC_RULE));
		rule->data.basic->head = this_rule->head;
		rule->data.basic->pos = this_rule->pos;
		rule->data.basic->pos_cnt = this_rule->pos_cnt;
		rule->data.basic->neg = this_rule->neg;
		rule->data.basic->neg_cnt = this_rule->neg_cnt;
		rule->type = TYPE_BASIC;
		free(this_rule);
		stats->triv_reduced[TYPE_WEIGHT]++;
		stats->any++;
		return 0;
		// Reduce to constraint rule if any literal will satisfy bound
	} else if (this_rule->bound < lightest) {
		rule->data.constraint = malloc(sizeof(CONSTRAINT_RULE));
		rule->data.constraint->head = this_rule->head;
		rule->data.constraint->pos = this_rule->pos;
		rule->data.constraint->pos_cnt = this_rule->pos_cnt;
		rule->data.constraint->neg = this_rule->neg;
		rule->data.constraint->neg_cnt = this_rule->neg_cnt;
		rule->data.constraint->bound = 1;

		//free(this_rule->weight);
		free(this_rule);
		rule->type = TYPE_CONSTRAINT;

		stats->triv_reduced[TYPE_WEIGHT]++;
		stats->any++;

		return 0;
	}

	// Check if weights uniform
	int uniform = 1;
	int w = this_rule->weight[0];
	for (i = 1; i < this_rule->neg_cnt + this_rule->pos_cnt; i++)
		if (w != this_rule->weight[i]) {
			uniform = 0;
			break;
		}

	// R'= R \ { h <- l[B] } U { h <- l/w{B} }		if uniform weights
	if (uniform) {
		rule->data.constraint = malloc(sizeof(CONSTRAINT_RULE));
		rule->data.constraint->head = this_rule->head;
		rule->data.constraint->pos = this_rule->pos;
		rule->data.constraint->pos_cnt = this_rule->pos_cnt;
		rule->data.constraint->neg = this_rule->neg;
		rule->data.constraint->neg_cnt = this_rule->neg_cnt;
		rule->data.constraint->bound = (this_rule->bound / w);
		if (this_rule->bound % w > 0)
			rule->data.constraint->bound++;
		rule->type = TYPE_CONSTRAINT;

		// Weight tables are for some obscure reason not dynamically allocated
		// So we can't free them either.
		//free(rule->data.weight->weight);
		free(this_rule);

		stats->triv_reduced[TYPE_WEIGHT]++;
		stats->any++;

		return 0;
	}

	return 0;
}

/* Reduces any trivial rule,
 * returns 1 if to be removed, 0 otherwise */
int find_trivial(RULE *rule) {
	int type = rule->type;

	switch (type) {
	case TYPE_BASIC:
		return 0; // Basic rules can't be reduced
	case TYPE_CONSTRAINT:
		return find_trivial_constraint(rule);
	case TYPE_CHOICE:
		/* Choise rules can't be trivialized because zero heads can also be chosen */
		return 0;
	case TYPE_WEIGHT:
		return find_trivial_weight(rule);
	default:
		return 0;
	}
}
