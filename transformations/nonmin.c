/* SIMPLIFY - nonmin.c
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <stdio.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"

#include "helpers.h"
#include "../misc.h"
#include "../stats.h"
extern STAT *stats;
#include "../occurrences.h"
extern OCCTAB *occtab;

/* Checks if a basic rule is nonmin */
static int find_nonmin_basic(const RULE *a, const RULE *b) {
	BASIC_RULE *this_a = a->data.basic;

	switch (b->type) {
	// Remove rule if it has "weaker" body
	case TYPE_BASIC: {
		BASIC_RULE *this_b = b->data.basic;

		/* Check head */
		if (this_a->head != this_b->head)
			return 0;

		/* Check neg */
		if (!is_subset_of(this_a->neg, this_a->neg_cnt, this_b->neg,
				this_b->neg_cnt))
			return 0;

		/* Check pos */
		else if (!is_subset_of(this_a->pos, this_a->pos_cnt, this_b->pos,
				this_b->pos_cnt))
			return 0;

		stats->nonmin_removed[TYPE_BASIC]++;
		stats->any++;

		return 1;
	}

		// Remove rule if body is "weaker"
	case TYPE_CHOICE: {
		CHOICE_RULE *this_b = b->data.choice;

		// Check heads if more nonmins possible
		if (this_a->head < this_b->head[0])
			return 0;

		/* Check neg */
		else if (!is_subset_of(this_a->neg, this_a->neg_cnt, this_b->neg,
				this_b->neg_cnt))
			return 0;

		/* Check pos */
		else if (!is_subset_of(this_a->pos, this_a->pos_cnt, this_b->pos,
				this_b->pos_cnt))
			return 0;

		// Remove H(a) form H(b)
		int n = contains_element(this_b->head, this_b->head_cnt, this_a->head);
		if (n) {
			remove_atom_occurrence(b, &(occtab->head)[this_b->head[n - 1]]);
			remove_element(this_b->head, this_b->head_cnt, n - 1);
			this_b->head_cnt--;
			stats->any++;
		}

		if (this_b->head_cnt == 0) {
			stats->nonmin_removed[TYPE_CHOICE]++;
			return 1;
		} else {
			stats->nonmin_reduced[TYPE_CHOICE]++;
			return 0;
		}
	}
	}
	return 0;
}

/* Checks if a constraint rule is nonmin */
static int find_nonmin_constraint(const RULE *a, const RULE *b) {
	CONSTRAINT_RULE *this_a = a->data.constraint;

	switch (b->type) {

	// Remove rule if body is "weaker"
	case TYPE_BASIC: {
		BASIC_RULE *this_b = b->data.basic;

		/* Check head */
		if (this_a->head != this_b->head)
			return 0;

		// Check if the amount of common atoms is equal or grater than bound
		int i, common = 0;
		for (i = 0; i < this_a->neg_cnt; i++)
			if (contains_element(this_b->neg, this_b->neg_cnt, this_a->neg[i]))
				common++;

		for (i = 0; i < this_a->pos_cnt; i++)
			if (contains_element(this_b->pos, this_b->pos_cnt, this_a->pos[i]))
				common++;

		if (this_a->bound <= common) {
			stats->nonmin_removed[TYPE_BASIC]++;
			stats->any++;
			return 1;
		} else
			return 0;
	}
		// Remove rule if b:s bound requires more atoms than a has
	case TYPE_CONSTRAINT: {
		CONSTRAINT_RULE *this_b = b->data.constraint;

		/* Check head */
		if (this_a->head != this_b->head)
			return 0;

		/* Check bounds */
		if (this_b->bound < this_a->bound)
			return 0;

		/* Check neg */
		else if (!is_subset_of(this_a->neg, this_a->neg_cnt, this_b->neg,
				this_b->neg_cnt))
			return 0;

		/* Check pos */
		else if (!is_subset_of(this_a->pos, this_a->pos_cnt, this_b->pos,
				this_b->pos_cnt))
			return 0;

		else {
			stats->nonmin_removed[TYPE_CONSTRAINT]++;
			stats->any++;
			return 1;
		}
	}

	}
	return 0;
}

/* Checks if a choice rule is nonmin */
static int find_nonmin_choice(const RULE *a, const RULE *b) {

	if (b->type != TYPE_CHOICE)
		return 0;

	CHOICE_RULE *this_a = a->data.choice;
	CHOICE_RULE *this_b = b->data.choice;

	// Check heads if more nonmins possible
	if (this_a->head[this_a->head_cnt - 1] < this_b->head[0])
		return 0;

	/* Check neg */
	else if (!is_subset_of(this_a->neg, this_a->neg_cnt, this_b->neg,
			this_b->neg_cnt))
		return 0;

	/* Check pos */
	else if (!is_subset_of(this_a->pos, this_a->pos_cnt, this_b->pos,
			this_b->pos_cnt))
		return 0;

	// Remove all a.head from b.head ( H_b = H_b \ H_a )
	int i, n = 0;
	for (i = 0; i < this_a->head_cnt; i++) {
		if ((n = contains_element(this_b->head, this_b->head_cnt,
				this_a->head[i]))) {
			remove_atom_occurrence(b, &(occtab->head)[this_b->head[n - 1]]);
			remove_element(this_b->head, this_b->head_cnt, n - 1);
			this_b->head_cnt--;
			stats->any++;
		}
	}

	if (this_b->head_cnt == 0) {
		stats->nonmin_removed[TYPE_CHOICE]++;
		return 1;
	} else if (n)
		stats->nonmin_reduced[TYPE_CHOICE]++;
	return 0;

}

/* Checks if a weight rule is nonmin */
static int find_nonmin_weight(const RULE *a, const RULE *b) {
	WEIGHT_RULE *this_a = a->data.weight;

	switch (b->type) {

	// Remove rule if weight of atoms in b is higher than bound in a
	case TYPE_BASIC: {
		BASIC_RULE *this_b = b->data.basic;

		/* Check head */
		if (this_a->head != this_b->head)
			return 0;

		// Check if the weight of common atoms is equal or grater than bound
		int n, i, total_weight = 0;
		for (i = 0; i < this_a->neg_cnt; i++)
			if ((n = contains_element(this_b->neg, this_b->neg_cnt,
					this_a->neg[i])))
				total_weight += this_a->weight[n - 1];

		for (i = 0; i < this_a->pos_cnt; i++)
			if ((n = contains_element(this_b->pos, this_b->pos_cnt,
					this_a->pos[i])))
				total_weight += this_a->weight[this_a->neg_cnt + n - 1];

		if (this_a->bound <= total_weight) {
			stats->nonmin_removed[TYPE_BASIC]++;
			stats->any++;
			return 1;
		} else
			return 0;
	}
		// Remove rule if higher bound and smaller weights
	case TYPE_WEIGHT: {
		WEIGHT_RULE *this_b = b->data.weight;

		/* Check head */
		if (this_a->head != this_b->head)
			return 0;

		// Check bounds
		if (this_b->bound < this_a->bound)
			return 0;

		/* Check neg */
		else if (!is_subset_of(this_a->neg, this_a->neg_cnt, this_b->neg,
				this_b->neg_cnt))
			return 0;

		/* Check pos */
		else if (!is_subset_of(this_a->pos, this_a->pos_cnt, this_b->pos,
				this_b->pos_cnt))
			return 0;

		int i, ok = 1;
		// Check neg weights
		for (i = 0; i < this_b->neg_cnt; i++)
			if (this_b->weight[i] != this_a->weight[contains_element(
					this_a->neg, this_a->neg_cnt, this_b->neg[i]) - 1]) {
				ok = 0;
				break;
			}
		// Check pos weights
		if (ok)
			for (i = 0; i < this_b->pos_cnt; i++)
				if (this_b->weight[i + this_b->neg_cnt]
						!= this_a->weight[contains_element(this_a->pos,
								this_a->pos_cnt, this_b->pos[i]) - 1
								+ this_a->neg_cnt]) {
					ok = 0;
					break;
				}

		if (ok) {
			stats->nonmin_removed[TYPE_WEIGHT]++;
			stats->any++;
			return 1;
		}
	}
	}
	return 0;
}

/* Checks if rule b is redundant because of rule a, on basis of NONMIN */
static int find_nonmin(const RULE *a, const RULE *b) {

	// Don't do nonmin on same rules
	if (a == b)
		return 0;

	switch (a->type) {
	case TYPE_BASIC:
		return find_nonmin_basic(a, b);
	case TYPE_CONSTRAINT:
		return find_nonmin_constraint(a, b);
	case TYPE_CHOICE:
		return find_nonmin_choice(a, b);
	case TYPE_WEIGHT:
		return find_nonmin_weight(a, b);
	default:
		return 0;
	}
}

/* Checks if given rule is removeable on basis of nonmin, does this by checking
 * possible candidates from the occtab.
 */
int remove_nonmin(RULE *rule) {

	int pos_cnt = get_pos_cnt(rule);
	int neg_cnt = get_neg_cnt(rule);
	int *pos = get_pos(rule);
	int *neg = get_neg(rule);

	OCC *smallest = NULL;
	OCC **tables = &smallest;
	int table_cnt = 1;

	// Find smallest table to check
	int i, t;
	// Check pos
	for (i = 0; i < pos_cnt; i++)
		if (smallest == NULL || (occtab->pbody)[pos[i]].rule_cnt
				< smallest->rule_cnt)
			smallest = &(occtab->pbody)[pos[i]];

	// Check neg
	for (i = 0; i < neg_cnt; i++)
		if (smallest == NULL || (occtab->nbody)[neg[i]].rule_cnt
				< smallest->rule_cnt)
			smallest = &(occtab->nbody)[neg[i]];

	// Check head
	if (rule->type != TYPE_CHOICE) {
		if (smallest == NULL || (occtab->head)[get_head(rule)].rule_cnt
				< smallest->rule_cnt)
			smallest = &(occtab->head)[get_head(rule)];
	}
	// Take choice heads if nothing else available
	else if (smallest == NULL) {
		tables = malloc(rule->data.choice->head_cnt * sizeof(OCC*));
		table_cnt = rule->data.choice->head_cnt;

		for (t = 0; t < rule->data.choice->head_cnt; t++)
			tables[t] = &(occtab->head)[rule->data.choice->head[t]];
	}

	// Call actual nonmin verifyers
	OCC *occ;
	for (t = 0; t < table_cnt; t++) {
		occ = tables[t];
			for (i = 0; i < occ->size; i++)
				// Check if rule is removed
				if (occ->rules[i] && find_nonmin(occ->rules[i], rule))
					return 1;
	}

	return 0;
}
