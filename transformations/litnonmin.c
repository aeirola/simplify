/* SIMPLIFY - litnonmin.c
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

/* Checks if literal from B can be removed since A allready defines it */
static int find_litnonmin(const RULE *a, const RULE *b) {
	if (a->type != TYPE_BASIC)
		return 0;

	BASIC_RULE *this_a = a->data.basic;

	switch (b->type) {
	case TYPE_BASIC: {
		BASIC_RULE *this_b = b->data.basic;

		// Check neg
		if (!is_subset_of(this_a->neg, this_a->neg_cnt, this_b->neg,
				this_b->neg_cnt))
			return 0;

		/* Check pos */
		else if (!is_subset_of(this_a->pos, this_a->pos_cnt, this_b->pos,
				this_b->pos_cnt))
			return 0;

		// No need to check these as it's basically tautologies
		else if (contains_element(this_a->neg, this_a->neg_cnt, this_b->head)
				|| contains_element(this_a->pos, this_a->pos_cnt, this_b->head))
			return 0;

		int n;
		if ((n = contains_element(this_b->pos, this_b->pos_cnt, this_a->head))) {

			remove_atom_occurrence(b, &(occtab->pbody)[this_b->pos[n - 1]]);
			remove_element(this_b->pos, this_b->pos_cnt--, n - 1);
			stats->litnonmin_reduced[TYPE_BASIC]++;
			stats->any++;
			return 0;

		} else if (contains_element(this_b->neg, this_b->neg_cnt, this_a->head)) {
			stats->litnonmin_removed[TYPE_BASIC]++;
			stats->any++;
			return 1;
		}

		else
			return 0;
	}
	case TYPE_CHOICE: {
		CHOICE_RULE *this_b = b->data.choice;

		// Check neg
		if (!is_subset_of(this_a->neg, this_a->neg_cnt, this_b->neg,
				this_b->neg_cnt))
			return 0;

		/* Check pos */
		else if (!is_subset_of(this_a->pos, this_a->pos_cnt, this_b->pos,
				this_b->pos_cnt))
			return 0;

		// No need to check these as it's basically tautologies
		else if (is_subset_of(this_a->neg, this_a->neg_cnt, this_b->head,
				this_b->head_cnt) || is_subset_of(this_a->pos, this_a->pos_cnt,
				this_b->head, this_b->head_cnt))
			return 0;

		int n;
		if ((n = contains_element(this_b->pos, this_b->pos_cnt, this_a->head))) {
			remove_atom_occurrence(b, &(occtab->pbody)[this_b->pos[n - 1]]);
			remove_element(this_b->pos, this_b->pos_cnt--, n - 1);
			stats->litnonmin_reduced[TYPE_CHOICE]++;
			stats->any++;
			return 0;

		} else if (contains_element(this_b->neg, this_b->neg_cnt, this_a->head)) {
			stats->litnonmin_removed[TYPE_CHOICE]++;
			stats->any++;
			return 1;
		} else
			return 0;
	}
	}
	return 0;
}

/* Checks if given rule is removeable on basis of litnonmin, does this by checking
 * possible candidates from the occtab.
 */
int remove_litnonmin(RULE *rule) {

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

	// Use heads as last resort
	int head = get_head(rule);
	if (rule->type != TYPE_CHOICE) {
		if (smallest == NULL || (occtab->head)[head].rule_cnt
				< smallest->rule_cnt) {
			tables = malloc(2 * sizeof(OCC*));
			table_cnt = 2;

			tables[0] = &(occtab->pbody)[head];
			tables[1] = &(occtab->nbody)[head];
		}
	}

	// Different procedure for choice heads
	else if (smallest == NULL) {
		tables = malloc(rule->data.choice->head_cnt * 2 * sizeof(OCC*));
		table_cnt = rule->data.choice->head_cnt * 2;

		for (t = 0; t < rule->data.choice->head_cnt; t++) {
			tables[2 * t] = &(occtab->pbody)[rule->data.choice->head[t]];
			tables[2 * t + 1] = &(occtab->nbody)[rule->data.choice->head[t]];
		}
	}

	// Call actual nonmin verifyers
	OCC *occ;
	for (t = 0; t < table_cnt; t++) {
		occ = tables[t];
		// Ceck negative body occurrances
		for (i = 0; i < occ->size; i++)
			// Check if rule is removed
			if (occ->rules[i] && find_litnonmin(occ->rules[i], rule)) {
				return 1;
			}

	}

	return 0;

}
