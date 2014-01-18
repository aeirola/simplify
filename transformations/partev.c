/* SIMPLIFY - partev.c
 *
 * Includes functions related to partial evaluation.
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"

#include "assert.h"

#include "helpers.h"
#include "../misc.h"
#include "../normalize.h"
#include "../stats.h"
extern STAT *stats;
#include "../options.h"
extern OPT *options;

#include "../occurrences.h"
extern OCCTAB *occtab;

// TODO: Replace singleatom bodys with corresponding any rule (?)

/* Remove occurences of fact heads */
static int remove_occurrences(RULE *a, RULE *b) {

	if (a->type != TYPE_BASIC)
		return 0;

	BASIC_RULE *rule = a->data.basic;

	if (rule->neg_cnt != 0 || rule->pos_cnt != 0)
		return 0;

	int n;

	if ((n = contains_element(get_neg(b), get_neg_cnt(b), rule->head))) {
		// Found in negative body, never
		stats->any++;

		switch (b->type) {
		case TYPE_BASIC:
		case TYPE_CHOICE:
			// Body never true, remove rule
			stats->partev_removed[b->type]++;
			return 2;
		case TYPE_CONSTRAINT:
			remove_atom_occurrence(b,
					&(occtab->nbody)[b->data.constraint->neg[n - 1]]);
			remove_element(b->data.constraint->neg,
					b->data.constraint->neg_cnt--, n - 1);
			stats->partev_reduced[b->type]++;
			return 1;
		case TYPE_WEIGHT:
			remove_atom_occurrence(b, &(occtab->nbody)[b->data.weight->neg[n
					- 1]]);
			remove_element(b->data.weight->neg, b->data.weight->neg_cnt, n - 1);
			remove_element(b->data.weight->weight, b->data.weight->neg_cnt--
					+ b->data.weight->pos_cnt, n - 1);
			stats->partev_reduced[b->type]++;
			return 1;
		}

	} else if ((n = contains_element(get_pos(b), get_pos_cnt(b), rule->head))) {
		// Found in positive body, always true
		stats->partev_reduced[b->type]++;
		stats->any++;

		switch (b->type) {
		case TYPE_BASIC:
			remove_atom_occurrence(b,
					&(occtab->pbody)[b->data.basic->pos[n - 1]]);
			remove_element(b->data.basic->pos, b->data.basic->pos_cnt--, n - 1);
			return 1;
		case TYPE_CHOICE:
			remove_atom_occurrence(b, &(occtab->pbody)[b->data.choice->pos[n
					- 1]]);
			remove_element(b->data.choice->pos, b->data.choice->pos_cnt--, n
					- 1);
			return 1;
		case TYPE_CONSTRAINT:
			remove_atom_occurrence(b,
					&(occtab->pbody)[b->data.constraint->pos[n - 1]]);
			remove_element(b->data.constraint->pos,
					b->data.constraint->pos_cnt--, n - 1);
			// Decrease bound
			b->data.constraint->bound--;
			return 1;
		case TYPE_WEIGHT:
			remove_atom_occurrence(b, &(occtab->pbody)[b->data.weight->pos[n
					- 1]]);
			remove_element(b->data.weight->pos, b->data.weight->pos_cnt, n - 1);
			// Decrease bound
			b->data.weight->bound
					-= b->data.weight->weight[b->data.weight->neg_cnt + n - 1];
			// Remove weight
			remove_element(b->data.weight->weight, b->data.weight->neg_cnt
					+ b->data.weight->pos_cnt--, b->data.weight->neg_cnt + n
					- 1);
			return 1;
		}
	}
	return 0;
}

/* Replace occurences of head with positive one-atom body */
static int replace_occurrences_pos(RULE *a, RULE *b) {

	if (a->type != TYPE_BASIC)
		return 0;

	BASIC_RULE *rule = a->data.basic;

	if (rule->neg_cnt != 0 || rule->pos_cnt != 1)
		return 0;

	int n;
	if ((n = contains_element(get_neg(b), get_neg_cnt(b), rule->head))) {

		get_neg(b)[n - 1] = rule->pos[0]; // Replace occurence
		normalize(b); // Normalize
		stats->partev_removed[b->type]++;
		stats->any++;

		// Update occtab
		remove_atom_occurrence(b, &(occtab->nbody)[rule->head]);
		add_atom_occurrence(b, &(occtab->nbody)[rule->pos[0]]);
		return 1;

	} else if ((n = contains_element(get_pos(b), get_pos_cnt(b), rule->head))) {
		get_pos(b)[n - 1] = rule->pos[0]; // Replace occurence
		normalize(b); // Normalize
		stats->partev_reduced[b->type]++;
		stats->any++;

		// Update occtab
		remove_atom_occurrence(b, &(occtab->pbody)[rule->head]);
		add_atom_occurrence(b, &(occtab->pbody)[rule->pos[0]]);
		return 1;
	}
	return 0;
}

/* Replace occurences of head with negative one-atom body */
static int replace_occurrences_neg(RULE *a, RULE *b) {

	if (a->type != TYPE_BASIC)
		return 0;

	BASIC_RULE *rule = a->data.basic;

	if (rule->neg_cnt != 1 || rule->pos_cnt != 0)
		return 0;

	int n;
	// Double negation bad thing if unsure abount SCC
	/*
	if ((n = contains_element(get_neg(b), get_neg_cnt(b), rule->head))) {

		switch (b->type) {
		 case TYPE_BASIC:
		 // Remove from negative body
		 remove_element(b->data.basic->neg,
		 b->data.basic->neg_cnt--, n - 1);
		 // Move positive pointer
		 b->data.basic->pos
		 = &(b->data.basic->neg[b->data.basic->neg_cnt]);
		 b->data.basic->pos[0] = rule->neg[0];
		 // Update counts
		 b->data.basic->pos_cnt++;

		 case TYPE_CONSTRAINT:
		 // Remove from negative body
		 remove_element(b->data.constraint->neg,
		 b->data.constraint->neg_cnt--, n - 1);
		 // Move positive pointer
		 b->data.constraint->pos
		 = &(b->data.constraint->neg[b->data.constraint->neg_cnt]);
		 b->data.constraint->pos[0] = rule->neg[0];
		 // Update counts
		 b->data.constraint->pos_cnt++;

		 case TYPE_CHOICE:
		 // Remove from negative body
		 remove_element(b->data.choice->neg,
		 b->data.choice->neg_cnt--, n - 1);
		 // Move positive pointer
		 b->data.choice->pos
		 = &(b->data.choice->neg[b->data.choice->neg_cnt]);
		 b->data.choice->pos[0] = rule->neg[0];
		 // Update counts
		 b->data.choice->pos_cnt++;

		 case TYPE_WEIGHT: {
		 // Remove from negative body
		 remove_element(b->data.weight->neg,
		 b->data.weight->neg_cnt--, n - 1);

		 // Move weight
		 int weight = b->data.weight->weight[n - 1];
		 remove_element(b->data.weight->weight,
		 b->data.weight->neg_cnt--, n - 1);
		 b->data.weight->weight[b->data.weight->neg_cnt
		 + 1] = weight;

		 // Move positive pointer
		 b->data.weight->pos
		 = &(b->data.weight->neg[b->data.weight->neg_cnt]);
		 b->data.weight->pos[0] = rule->neg[0];
		 b->data.weight->pos_cnt++;
		 }
		 }
		 normalize(b); // Normalize
		 stats->partev_simp[b->type]++;
		 stats->any++;

	} else
	*/ 
	
	if ((n = contains_element(get_pos(b), get_pos_cnt(b), rule->head))) {

		switch (b->type) {
		case TYPE_BASIC:

			// Remove from positive body
			memmove(&(b->data.basic->pos[1]), &(b->data.basic->pos[0]), (n - 1)
					* sizeof(int));
			// Add negated body
			b->data.basic->pos[0] = rule->neg[0];
			// Move positive pointer
			b->data.basic->pos = &(b->data.basic->pos[1]);
			// Update counts
			b->data.basic->pos_cnt--;
			b->data.basic->neg_cnt++;
			break;

		case TYPE_CONSTRAINT:

			// Remove from positive body
			memmove(&(b->data.constraint->pos[1]),
					&(b->data.constraint->pos[0]), (n - 1) * sizeof(int));
			// Add negated body
			b->data.constraint->pos[0] = rule->neg[0];
			// Move positive pointer
			b->data.constraint->pos = &(b->data.constraint->pos[1]);
			// Update counts
			b->data.constraint->pos_cnt--;
			b->data.constraint->neg_cnt++;
			break;

		case TYPE_CHOICE:

			// Remove from positive body
			memmove(&(b->data.choice->pos[1]), &(b->data.choice->pos[0]), (n
					- 1) * sizeof(int));
			// Add negated body
			b->data.choice->pos[0] = rule->neg[0];
			// Move positive pointer
			b->data.choice->pos = &(b->data.choice->pos[1]);
			// Update counts
			b->data.choice->pos_cnt--;
			b->data.choice->neg_cnt++;
			break;

		case TYPE_WEIGHT: {

			// Remove from positive body
			memmove(&(b->data.weight->pos[1]), &(b->data.weight->pos[0]), (n
					- 1) * sizeof(int));
			// Add negated body
			b->data.weight->pos[0] = rule->neg[0];

			// Move weight
			int weight =
					b->data.weight->weight[b->data.weight->pos_cnt + n - 1];
			memmove(&(b->data.weight->weight[b->data.weight->neg_cnt + 1]),
					&(b->data.weight->weight[b->data.weight->neg_cnt]), (n - 1)
							* sizeof(int));
			b->data.weight->weight[b->data.weight->neg_cnt] = weight;

			// Move positive pointer
			b->data.weight->pos = &(b->data.weight->pos[1]);
			// Update counts
			b->data.weight->pos_cnt--;
			b->data.weight->neg_cnt++;
			break;

		}
		}

		// Update occurrences
		remove_atom_occurrence(b, &(occtab->pbody)[rule->neg[0]]);
		add_atom_occurrence(b, &(occtab->nbody)[rule->neg[0]]);

		normalize(b); // Normalize
		stats->partev_reduced[b->type]++;
		stats->any++;
		return 1;
	}
	return 0;
}

/* Evaluates all occurances of head in rule bodies of type BASIC and CHOICE
 * returns 1 if body has been changed, 0 otherwise */
static int evaluate_head(RULE *a, RULE *b) {
	if (a->type != TYPE_BASIC)
		return 0;

	BASIC_RULE *rule = a->data.basic;

	int head = rule->head;

	// Check for compatible type
	if ((b->type == TYPE_BASIC) || (b->type == TYPE_CHOICE)) {
		int *body = get_neg(b);
		int neg_cnt = get_neg_cnt(b);
		int pos_cnt = get_pos_cnt(b);

		int n;
		if ((n = contains_element(&body[neg_cnt], pos_cnt, head))) {
			int neg_rem = 0;
			int pos_rem = 0;
			// "Remove" head from body, removed atoms marked by 0
			body[(n - 1) + neg_cnt] = -1;
			pos_rem++;
			remove_atom_occurrence(b, &(occtab->pbody)[head]);

			// Check for duplicate atoms in bodies, and "remove" them
			int j;
			for (j = 0; j < rule->neg_cnt; j++)
				if ((n = contains_element(body, neg_cnt, rule->neg[j]))) {
					remove_atom_occurrence(b, &(occtab->nbody)[body[n - 1]]);
					body[n - 1] = -1;
					neg_rem++;
				}
			for (j = 0; j < rule->pos_cnt; j++)
				if ((n
						= contains_element(&body[neg_cnt], pos_cnt,
								rule->pos[j]))) {
					remove_atom_occurrence(b, &(occtab->pbody)[body[(n - 1)
							+ neg_cnt]]);
					body[(n - 1) + neg_cnt] = -1;
					pos_rem++;
				}

			// Allocate new memory for body
			int new_pos_cnt = pos_cnt - pos_rem + rule->pos_cnt;
			int new_neg_cnt = neg_cnt - neg_rem + rule->neg_cnt;
			int *new_body = malloc((new_pos_cnt + new_neg_cnt) * sizeof(int));

			// Copy negative atoms to new body
			memcpy(new_body, rule->neg, (rule->neg_cnt) * sizeof(int));
			n = rule->neg_cnt;

			// Add occurrences of copied atoms
			for (j = 0; j < rule->neg_cnt; j++)
				add_atom_occurrence(b, &(occtab->nbody)[rule->neg[j]]);

			for (j = 0; j < neg_cnt; j++)
				if (body[j] != -1)
					new_body[n++] = body[j];

			// Copy positive atoms to new body
			memcpy(&new_body[n], rule->pos, (rule->pos_cnt) * sizeof(int));
			n += rule->pos_cnt;

			// Add occurrences of copied atoms
			for (j = 0; j < rule->pos_cnt; j++)
				add_atom_occurrence(b, &(occtab->pbody)[rule->pos[j]]);

			for (j = 0; j < pos_cnt; j++)
				if (body[j + neg_cnt] != -1)
					new_body[n++] = body[j + neg_cnt];

			// Move new data to rule
			if (b->type == TYPE_BASIC) {
				b->data.basic->neg_cnt = new_neg_cnt;
				b->data.basic->pos_cnt = new_pos_cnt;
				b->data.basic->neg = new_body;
				b->data.basic->pos = &new_body[new_neg_cnt];
			} else if (b->type == TYPE_CHOICE) {
				b->data.choice->neg_cnt = new_neg_cnt;
				b->data.choice->pos_cnt = new_pos_cnt;
				b->data.choice->neg = new_body;
				b->data.choice->pos = &new_body[new_neg_cnt];
			}

			// Free old body
			free(body);

			// Normalize
			normalize(b);

			stats->partev_reduced[b->type]++;
			stats->any++;

			return 1;

		}

	} // Rule type end

	return 0;
}

/* Partialy evaluates given and linked rules */
int partev(RULE *rule) {
	RULE *unique_rule = NULL;
	extern OCCTAB *occtab;

	int i, j;
	int cnt[2];
	cnt[0] = get_pos_cnt(rule);
	cnt[1] = get_neg_cnt(rule);

	int *body[2];
	body[0] = get_pos(rule);
	body[1] = get_neg(rule);

	// Loop through both positive and negative body
	for (j = 0; j < 2; j++)
		for (i = 0; i < cnt[j]; i++)
			// Check that head has only one defining rule
			if ((occtab->head)[body[j][i]].rule_cnt == 1) {

				// Get right rule, in case list begins with removed ones
				int n;
				for (n = 0; n < (occtab->head)[body[j][i]].size; n++)
					if ((unique_rule = (occtab->head)[body[j][i]].rules[n]))
						break;

				// Check for existance and type
				if (unique_rule != NULL && unique_rule->type == TYPE_BASIC) {
					if (unique_rule->data.basic->neg_cnt == 0
							&& unique_rule->data.basic->pos_cnt == 0) {
						// Special case (facts), remove all occurences
						switch (remove_occurrences(unique_rule, rule)) {
						case 2:
							return 1;
						case 1:
							// Update data if rule altered
							cnt[0] = get_pos_cnt(rule);
							cnt[1] = get_neg_cnt(rule);
							body[0] = get_pos(rule);
							body[1] = get_neg(rule);
						}

					} else if (unique_rule->data.basic->neg_cnt == 0
							&& unique_rule->data.basic->pos_cnt == 1) {
						// Special case, replace all occurences with bodyatom
						if (replace_occurrences_pos(unique_rule, rule)) {
							// Update data if rule altered
							cnt[0] = get_pos_cnt(rule);
							cnt[1] = get_neg_cnt(rule);
							body[0] = get_pos(rule);
							body[1] = get_neg(rule);
						}

					} else if (unique_rule->data.basic->neg_cnt == 1
							&& unique_rule->data.basic->pos_cnt == 0) {
						// Special case, replace all occurences with negated bodyatom
						if (replace_occurrences_neg(unique_rule, rule)) {
							// Update data if rule altered
							cnt[0] = get_pos_cnt(rule);
							cnt[1] = get_neg_cnt(rule);
							body[0] = get_pos(rule);
							body[1] = get_neg(rule);
						}

					}
					// General case for basic and choice rules
					else if (options->partev_full && evaluate_head(unique_rule, rule)) {
						// Update data if rule altered
						cnt[0] = get_pos_cnt(rule);
						cnt[1] = get_neg_cnt(rule);
						body[0] = get_pos(rule);
						body[1] = get_neg(rule);
					}

				}

			}

	return 0;
}
