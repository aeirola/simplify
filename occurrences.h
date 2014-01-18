/* SIMPLIFY - occurances.h
 *
 * 2008 Axel Eirola
 */

#ifndef OCCURRENCE_H_
#define OCCURRENCE_H_

/* Occurrences in rules bodies, holds a list of rules in wich an atom occurs.
 * Deleted rules marked as NULL pointers as constant memory copying is rather
 * slow */
typedef struct occ {
	int rule_cnt; /* Number of rules */
	int size; /* Rule table size */
	RULE **rules; /* First rule */
} OCC;

/* Occurrence table, holds three occurrance lists for each atom occurring in
 * heads, positive bodies and negative bodies */
typedef struct occtab {
	int count;
	OCC *head; /* Head occurrences */
	OCC *pbody; /* Positive occurrences in body */
	OCC *nbody; /* Negative occurrences in body */
} OCCTAB;

extern OCCTAB *initialize_occurrences(const ATAB *table);
extern void compute_occurrences(RULE *program, OCCTAB *occtab);

void add_rule_occurrences(RULE *rule, OCCTAB *occtab);
int remove_rule_occurrences(RULE *rule, OCCTAB *occtab);
void add_atom_occurrence(RULE *rule, OCC *occ);
int remove_atom_occurrence (const RULE *rule, OCC *occ);

#endif /* OCCURRENCE_H_ */
