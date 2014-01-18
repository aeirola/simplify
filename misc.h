/* SIMPLIFY - misc.h
 *
 * 2008 Axel Eirola
 */

extern FILE* smodels_internal(FILE *in);

extern void free_rule_data(RULE *rule);

extern int remove_rule(RULE *rule);

extern int int_cmp(const void *a, const void *b);

extern int sum (const int *a, const int a_cnt);
