/* SIMPLIFY - occurances.c
 *
 * Includes functions related to initialisatoin and maintenance od occurrence
 * tables.
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"

#include "misc.h"
#include "transformations/helpers.h"
#include "occurrences.h"

/* Initializes an occurrence table */
OCCTAB *initialize_occurrences(const ATAB *table) {

	OCCTAB *occtab = (OCCTAB *) malloc(sizeof(OCCTAB));
	occtab->count = table->count;

	/* Head occurrences */
	occtab->head = (OCC *) calloc((1 + occtab->count), sizeof(OCC));

	/* Occurrences in positive bodies */
	occtab->pbody = (OCC *) calloc((1 + occtab->count), sizeof(OCC));

	/* Occurrences in negative bodies */
	occtab->nbody = (OCC *) calloc((1 + occtab->count), sizeof(OCC));

	return occtab;
}

/* Calculates, allocates, populates and sorts given occurrance table */
void compute_occurrences(RULE *program, OCCTAB *occtab) {
	RULE *rule = program;
	OCC *current;
	int i;

	// Count atom occurrences
	while (rule) {

		// Head occurrences
		if (rule->type == TYPE_CHOICE) {
			int *heads = get_heads(rule);
			for (i = 0; i < get_head_cnt(rule); i++)
				(occtab->head)[heads[i]].rule_cnt++;
		} else
			(occtab->head)[get_head(rule)].rule_cnt++;

		// Positive occurrences
		int *pos = get_pos(rule);
		for (i = 0; i < get_pos_cnt(rule); i++)
			(occtab->pbody)[pos[i]].rule_cnt++;

		// Negative occurences
		int *neg = get_neg(rule);
		for (i = 0; i < get_neg_cnt(rule); i++)
			(occtab->nbody)[neg[i]].rule_cnt++;

		rule = rule->next;
	}

	// Allocate memory for occurence arrays
	for (i = 1; i <= occtab->count; i++) {
		// Allocate head arrays
		current = &(occtab->head)[i];
		if (current->rule_cnt) {
			current->rules = (RULE **) malloc(sizeof(RULE *)
					* (current->rule_cnt));
			current->size = current->rule_cnt;
			current->rule_cnt = 0;
		}

		// Allocate positive body arrays
		current = &(occtab->pbody)[i];
		if (current->rule_cnt) {
			current->rules = (RULE **) malloc(sizeof(RULE *)
					* (current->rule_cnt));
			current->size = current->rule_cnt;
			current->rule_cnt = 0;
		}

		// Allocate negative body arrays
		current = &(occtab->nbody)[i];
		if (current->rule_cnt) {
			current->rules = (RULE **) malloc(sizeof(RULE *)
					* (current->rule_cnt));
			current->size = current->rule_cnt;
			current->rule_cnt = 0;
		}
	}

	rule = program;

	// Add atom occurrences
	while (rule) {

		// Head occurrences
		if (rule->type == TYPE_CHOICE) {
			int *heads = get_heads(rule);
			for (i = 0; i < get_head_cnt(rule); i++) {
				current = &(occtab->head)[heads[i]];
				(current->rules)[current->rule_cnt++] = rule;
			}
		} else {
			current = &(occtab->head)[get_head(rule)];
			(current->rules)[current->rule_cnt++] = rule;
		}

		// Positive occurrences
		int *pos = get_pos(rule);
		for (i = 0; i < get_pos_cnt(rule); i++) {
			current = &(occtab->pbody)[pos[i]];
			(current->rules)[current->rule_cnt++] = rule;
		}

		// Negative occurences
		int *neg = get_neg(rule);
		for (i = 0; i < get_neg_cnt(rule); i++) {
			current = &(occtab->nbody)[neg[i]];
			(current->rules)[current->rule_cnt++] = rule;
		}

		rule = rule->next;
	}

	// Sort occurence lists
	for (i = 0; i < occtab->count; i++) {
		if ((occtab->head)[i].rule_cnt)
			qsort((occtab->head)[i].rules, (occtab->head)[i].rule_cnt,
					sizeof(OCC *), int_cmp);
		if ((occtab->pbody)[i].rule_cnt)
			qsort((occtab->pbody)[i].rules, (occtab->pbody)[i].rule_cnt,
					sizeof(OCC *), int_cmp);
		if ((occtab->nbody)[i].rule_cnt)
			qsort((occtab->nbody)[i].rules, (occtab->nbody)[i].rule_cnt,
					sizeof(OCC *), int_cmp);
	}

	return;
}

/* Adds occurrances for every atom in given rule */
void add_rule_occurrences(RULE *rule, OCCTAB *occtab) {
	int i;
	// Head occurrences
	if (rule->type == TYPE_CHOICE) {
		int *heads = get_heads(rule);
		for (i = 0; i < get_head_cnt(rule); i++)
			add_atom_occurrence(rule, &(occtab->head)[heads[i]]);
	} else if (rule->type > 0)
		add_atom_occurrence(rule, &(occtab->head)[get_head(rule)]);

	// Positive occurrences
	int *pos = get_pos(rule);
	for (i = 0; i < get_pos_cnt(rule); i++)
		add_atom_occurrence(rule, &(occtab->pbody)[pos[i]]);

	// Negative occurences
	int *neg = get_neg(rule);
	for (i = 0; i < get_neg_cnt(rule); i++)
		add_atom_occurrence(rule, &(occtab->nbody)[neg[i]]);

	return;
}

/* Removes occurrances for every atom in given rule */
int remove_rule_occurrences(RULE *rule, OCCTAB *occtab) {

	int i, r = 0;
	// Head occurrences
	if (rule->type == TYPE_CHOICE) {
		int *heads = get_heads(rule);
		for (i = 0; i < get_head_cnt(rule); i++)
			r += remove_atom_occurrence(rule, &(occtab->head)[heads[i]]);
	} else if (rule->type > 0)
		r += remove_atom_occurrence(rule, &(occtab->head)[get_head(rule)]);

	// Positive occurrences
	int *pos = get_pos(rule);
	for (i = 0; i < get_pos_cnt(rule); i++)
		r += remove_atom_occurrence(rule, &(occtab->pbody)[pos[i]]);

	int *neg = get_neg(rule);
	for (i = 0; i < get_neg_cnt(rule); i++)
		r += remove_atom_occurrence(rule, &(occtab->nbody)[neg[i]]);

	return r;
}

/* Finds the appropriate place for element elem int array a, by returning
 * the index succeeding the largest element being smaller than the given one
 */
static int find_place(const int *a, const int a_cnt, const int elem) {

	// Decide on appropriate search method
	if (a_cnt < 10) {
		// Linear search
		int i;
		for (i = 0; i < a_cnt - 1 && a[i] < elem;)
			i++;
		return i;

	} else {
		// Binary search
		int mid = 0;
		int low = 0;
		int high = a_cnt - 1;
		int mid_val = 0;

		while (low < high) {
			mid = (low + high) / 2;

			// Get value of mid
			mid_val = mid;
			while (!a[mid_val] && 0 < mid_val)
				mid_val--;

			if (a[mid_val] < elem)
				low = mid + 1;
			else
				high = mid;
		}
		return low;
	}
}

// Adds an occurrence of rule to occurrence table
void add_atom_occurrence(RULE *rule, OCC *occ) {

	// Find place for rule in sorted array
	int n = find_place((int *) occ->rules, occ->size, (int) rule);

	// Check if we need to allocate more memory
	if (occ->size == 0 || occ->rules[n] != NULL) {
		// Allocate more memory for added atoms
		occ->rules = realloc(occ->rules, ((occ->size + 1) * sizeof(int)));

		// Move tail to make space for new rule
		memmove(&((occ->rules)[n + 1]), &((occ->rules)[n]), (occ->size - n)
				* sizeof(RULE *));
		occ->size++;

	}
	// Add rule to right place
	occ->rules[n] = rule;

	// Increment count
	occ->rule_cnt++;

	return;
}

/* Removes occurrence data of rule from atom occurrence table
 */
int remove_atom_occurrence(const RULE *rule, OCC *occ) {

	// Check that not allready empty, should not happen but better be sure
	if (occ->size == 0)
		return -1;

	// Find occurrence
	int n = find_place((int *) occ->rules, occ->size, (int) rule);

	// Remove occurrence
	occ->rules[n] = NULL;
	occ->rule_cnt--;
	return 0;

}
