/* SIMPLIFY - normalize.c
 *
 * Includes functions used to normalize rules to canonical form.
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"

#include "transformations/helpers.h"
#include "misc.h"

#include "stats.h"
STAT *stats;
#include "options.h"
OPT *options;

int int_cmp(const void *a, const void *b);

/* Normalizes a basic rule */
int normalize_basic(RULE *rule) {

	BASIC_RULE *this_rule = rule->data.basic;
	qsort(this_rule->neg, this_rule->neg_cnt, sizeof(int), int_cmp);
	qsort(this_rule->pos, this_rule->pos_cnt, sizeof(int), int_cmp);

	// Remove duplicates
	this_rule->neg_cnt -= remove_duplicates(this_rule->neg, this_rule->neg_cnt);
	int p = remove_duplicates(this_rule->pos, this_rule->pos_cnt);

	this_rule->pos_cnt -= p;

	if (this_rule->pos != &(this_rule->neg[this_rule->neg_cnt]) || p) {
		memmove(&(this_rule->neg[this_rule->neg_cnt]), &(this_rule->pos[0]),
				(this_rule->pos_cnt) * sizeof(int));
		this_rule->neg = realloc(this_rule->neg, (this_rule->neg_cnt
				+ this_rule->pos_cnt) * sizeof(int));
		this_rule->pos = &(this_rule->neg[this_rule->neg_cnt]);
	}
	return 0;
}

/* Normalizes a constraint rule */
int normalize_constraint(RULE *rule) {

	CONSTRAINT_RULE *this_rule = rule->data.constraint;
	qsort(this_rule->neg, this_rule->neg_cnt, sizeof(int), int_cmp);
	qsort(this_rule->pos, this_rule->pos_cnt, sizeof(int), int_cmp);

	// Don't Remove duplicates
	/*int n = remove_duplicates(this_rule->neg, this_rule->neg_cnt);
	this_rule->neg_cnt -= n;
	int p = remove_duplicates(this_rule->pos, this_rule->pos_cnt);
	this_rule->pos_cnt -= p;

	this_rule->bound -= n + p;

	if (this_rule->pos != &(this_rule->neg[this_rule->neg_cnt]) || p) {
		memmove(&(this_rule->neg[this_rule->neg_cnt]), &(this_rule->pos[0]),
				(this_rule->pos_cnt) * sizeof(int));
		this_rule->neg = realloc(this_rule->neg, (this_rule->neg_cnt
				+ this_rule->pos_cnt) * sizeof(int));
		this_rule->pos = &(this_rule->neg[this_rule->neg_cnt]);
	}*/
	
	return 0;
}

/* Normalizes a choice rule */
int normalize_choice(RULE *rule) {

	CHOICE_RULE *this_rule = rule->data.choice;
	qsort(this_rule->head, this_rule->head_cnt, sizeof(int), int_cmp);
	qsort(this_rule->neg, this_rule->neg_cnt, sizeof(int), int_cmp);
	qsort(this_rule->pos, this_rule->pos_cnt, sizeof(int), int_cmp);

	// Remove duplicates
	int h = remove_duplicates(this_rule->head, this_rule->head_cnt);
	this_rule->head_cnt -= h;
	if (h)
		this_rule->head = realloc(this_rule->head, this_rule->head_cnt
				* sizeof(int));

	// Remove duplicates
	this_rule->neg_cnt -= remove_duplicates(this_rule->neg, this_rule->neg_cnt);
	int p = remove_duplicates(this_rule->pos, this_rule->pos_cnt);
	this_rule->pos_cnt -= p;

	if (this_rule->pos != &(this_rule->neg[this_rule->neg_cnt]) || p) {
		memmove(&(this_rule->neg[this_rule->neg_cnt]), &(this_rule->pos[0]),
				(this_rule->pos_cnt) * sizeof(int));
		this_rule->neg = realloc(this_rule->neg, (this_rule->neg_cnt
				+ this_rule->pos_cnt) * sizeof(int));
		this_rule->pos = &(this_rule->neg[this_rule->neg_cnt]);
	}

	return 0;
}

//  This public-domain C implementation by Darel Rex Finley.
//    (http://alienryderflex.com/quicksort/)
//
//  * This function assumes it is called with valid parameters.
//
//  Modified for sorting weight rules by adding a secondary array in which
//  the same swaps are made
void qsort_twin(int *arr_pri, int *arr_sec, const int elements) {

#define  MAX_LEVELS  300
	int piv_pri, piv_sec, beg[MAX_LEVELS], end[MAX_LEVELS], i = 0, L, R, swap;

	beg[0] = 0;
	end[0] = elements;
	while (i >= 0) {
		L = beg[i];
		R = end[i] - 1;
		if (L < R) {
			piv_pri = arr_pri[L];
			piv_sec = arr_sec[L];
			while (L < R) {
				while (arr_pri[R] >= piv_pri && L < R)
					R--;
				if (L < R) {
					arr_pri[L] = arr_pri[R];
					arr_sec[L++] = arr_sec[R];
				}
				while (arr_pri[L] <= piv_pri && L < R)
					L++;
				if (L < R) {
					arr_pri[R] = arr_pri[L];
					arr_sec[R--] = arr_sec[L];
				}
			}
			arr_pri[L] = piv_pri;
			arr_sec[L] = piv_sec;
			beg[i + 1] = L + 1;
			end[i + 1] = end[i];
			end[i++] = L;
			if (end[i] - beg[i] > end[i - 1] - beg[i - 1]) {
				swap = beg[i];
				beg[i] = beg[i - 1];
				beg[i - 1] = swap;
				swap = end[i];
				end[i] = end[i - 1];
				end[i - 1] = swap;
			}
		} else {
			i--;
		}
	}
}

/* Normalizes a weight rule */
int normalize_weight(RULE *rule) {
	if (rule->type != TYPE_WEIGHT)
		return 0;

	WEIGHT_RULE *this_rule = rule->data.weight;
	qsort_twin(this_rule->neg, this_rule->weight, this_rule->neg_cnt);
	qsort_twin(this_rule->pos, &this_rule->weight[this_rule->neg_cnt],
			this_rule->pos_cnt);

	// Remove duplicates
	int n = 0;
	int p = 0;
	int i, new_weight;

	for (i = 1; i < this_rule->neg_cnt; i++)
		if (this_rule->neg[i - 1] == this_rule->neg[i]) {
			// New weight is sum of weights 
			new_weight = this_rule->weight[i] + this_rule->weight[i - 1];

			memmove(&(this_rule->neg[i - 1]), &(this_rule->neg[i]),
					(this_rule->neg_cnt - i) * sizeof(int));
			memmove(&(this_rule->weight[i - 1]), &(this_rule->weight[i]),
					(this_rule->neg_cnt + this_rule->pos_cnt - i) * sizeof(int));

			this_rule->weight[i - 1] = new_weight;

			n++;
		}
	this_rule->neg_cnt -= n;

	for (i = 1; i < this_rule->pos_cnt; i++)
		if (this_rule->pos[i - 1] == this_rule->pos[i]) {
			new_weight = this_rule->weight[i + this_rule->neg_cnt]
					+ this_rule->weight[i + this_rule->neg_cnt - 1];

			memmove(&(this_rule->pos[i - 1]), &(this_rule->pos[i]),
					(this_rule->pos_cnt - i) * sizeof(int));
			memmove(&(this_rule->weight[i + this_rule->neg_cnt - 1]),
					&(this_rule->weight[i + this_rule->neg_cnt]),
					(this_rule->pos_cnt - i) * sizeof(int));

			this_rule->weight[i + this_rule->neg_cnt - 1] = new_weight;

			p++;
		}
	this_rule->pos_cnt -= p;

	if (this_rule->pos != &(this_rule->neg[this_rule->neg_cnt]) || p) {
		// Move body
		memmove(&(this_rule->neg[this_rule->neg_cnt]), &(this_rule->pos[0]),
				(this_rule->pos_cnt) * sizeof(int));
		this_rule->pos = &(this_rule->neg[this_rule->neg_cnt]);

		// Move weights
		memmove(&(this_rule->neg[this_rule->neg_cnt + this_rule->pos_cnt]),
				this_rule->weight, (this_rule->neg_cnt + this_rule->pos_cnt)
						* sizeof(int));
		this_rule->weight = &(this_rule->neg[this_rule->neg_cnt + this_rule->pos_cnt]);

		// Reallocate body
		this_rule->neg = realloc(this_rule->neg, (this_rule->neg_cnt
				+ this_rule->pos_cnt) * 2 * sizeof(int));

	}

	return 0;
}

/* Normalizes any rule */
int normalize(RULE *rule) {
	int type = rule->type;

	switch (type) {
	case TYPE_BASIC:
		return normalize_basic(rule);
	case TYPE_CONSTRAINT:
		return normalize_constraint(rule);
	case TYPE_CHOICE:
		return normalize_choice(rule);
	case TYPE_WEIGHT:
		return normalize_weight(rule);
	default:
		return -1;
	}
}
