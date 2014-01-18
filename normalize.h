/* SIMPLIFY - normalize.c
 *
 * 2008 Axel Eirola
 */

/* Normalizes a given rule according to canonical form as described in
 * doc/canonical_form, also calls reduce_trivial() to convert trivial instances
 * of constraint, choice and weight rules to basic rules.
 */
extern int normalize(RULE *rule);

