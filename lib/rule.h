/*
 * Data structures related to rules used in the smodels system
 *
 * (c) 2005 Tomi Janhunen
 */

#define _RULE_H_RCSFILE  "$RCSfile: rule.h,v $"
#define _RULE_H_DATE     "$Date: 2007/07/09 11:23:35 $"
#define _RULE_H_REVISION "$Revision: 1.7 $"

/* Rule types supported by smodels */

#define TYPE_BASIC       1
#define TYPE_CONSTRAINT  2
#define TYPE_CHOICE      3
#define TYPE_INTEGRITY   4
#define TYPE_WEIGHT      5
#define TYPE_OPTIMIZE    6   /* Not supported */

/* Other expressions of interest */

#define TYPE_ORDERED     7   /* Not supported */
#define TYPE_DISJUNCTIVE 8
#define TYPE_CLAUSE      9

/* Various kinds of rules */

typedef struct basic_rule {
  int head;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} BASIC_RULE;

typedef struct constraint_rule {
  int head;
  int bound;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} CONSTRAINT_RULE;

typedef struct choice_rule {
  int head_cnt;
  int *head;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} CHOICE_RULE;

typedef struct integrity_rule {
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} INTEGRITY_RULE;

typedef struct weight_rule {
  int head;
  int bound;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
  int *weight;
} WEIGHT_RULE;

typedef struct disjunctive_rule {
  int head_cnt;
  int *head;
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} DISJUNCTIVE_RULE;

typedef struct clause {
  int pos_cnt;
  int *pos;
  int neg_cnt;
  int *neg;
} CLAUSE;

typedef union any_rule {
  struct basic_rule *basic;
  struct constraint_rule *constraint;
  struct choice_rule *choice;
  struct integrity_rule *integrity;
  struct weight_rule *weight;
  struct disjunctive_rule *disjunctive;
  struct clause *clause;
} ANY_RULE;

typedef struct rule {
  int type;
  ANY_RULE data;
  struct rule *next;
} RULE;

extern void _version_rule_c();

extern int get_head(RULE *r);
extern int *get_heads(RULE *r);
extern int get_head_cnt(RULE *r);
extern int *get_pos(RULE *r);
extern int get_pos_cnt(RULE *r);
extern int *get_neg(RULE *r);
extern int get_neg_cnt(RULE *r);

extern int check_negative_invisible(RULE *program, ATAB* table);
extern void mark_io_atoms(RULE *program, ATAB *table, int module);
extern void mark_visible(ATAB *table);
extern void mark_occurrences(RULE *program, ATAB *table);
extern int non_basic(RULE *program);
extern int check_rule_types(RULE *program, int *types);
extern int non_atomic(RULE *program);
extern int len(RULE *program);
extern int compute_statement_len(ATAB *table);
extern int number_of_rules(RULE *program);
extern RULE *append_rules(RULE *program, RULE *rules);
