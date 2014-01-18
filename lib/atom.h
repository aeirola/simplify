/*
 * Data structures related to atoms
 *
 * (c) 2006 Tomi Janhunen
 */

#define _ATOM_H_RCSFILE  "$RCSfile: atom.h,v $"
#define _ATOM_H_DATE     "$Date: 2007/11/02 12:22:47 $"
#define _ATOM_H_REVISION "$Revision: 1.12 $"

/* Status bits for atoms */

#define MARK_TRUE    0x001
#define MARK_FALSE   0x002
#define MARK_TRUE_OR_FALSE 0x003
#define MARK_POSOCC  0x004
#define MARK_NEGOCC  0x008
#define MARK_POSOCC_OR_NEGOCC 0x00C
#define MARK_HEADOCC 0x010
#define MARK_VISIBLE 0x020
#define MARK_INPUT   0x040
#define MARK_EXTERN  0x080
#define MARK_BORDER  0x100
#define MARK_UNIQUE  0x200

/* Atom table */

typedef struct atab {
  int count;                /* Number of atoms */
  int offset;               /* Index value = atom number - offset */
  int shift;                /* Enables global shift of atom numbers */
  char *prefix;             /* Added to symbolic names */
  char *postfix;            /* Added to symbolic names */
  struct atab *other;       /* Cross-referenced table */
  struct atab *next;        /* Next piece */
  SYMBOL **names;           /* Vector of names */
  int *statuses;            /* Vector of status bits */
  int *others;              /* Vector of cross-references */
} ATAB;

/* Atom tables may consist of several pieces; see the next field above: */

/* The following routines handle a single piece: */

extern ATAB *new_table(int count, int offset);
extern ATAB *extend_table(ATAB *table, int count, int offset);
extern ATAB *append_table(ATAB *table1, ATAB *table2);

/* The following routines deal with all/several pieces: */

extern ATAB *copy_table(ATAB *table);
extern void initialize_other_tables(ATAB *table1, ATAB *table2);
extern int compare_atom_tables(ATAB *table1, ATAB *table2, int checkinput);
extern int match_atom_tables(ATAB *table1, ATAB *table2, int checkoutput);
extern ATAB *find_atom(ATAB *table, int atom);
extern int find_atom_by_name(ATAB *table, char *name);
extern SYMBOL *find_name(ATAB *table, int atom);
#define invisible(t,a) (!find_name(t,a))
#define visible(t,a) find_name(t,a)
extern int set_status_by_name(ATAB *table, char *name, int mask);
extern int set_status(ATAB *table, int atom, int mask);
extern int set_statuses(ATAB *table, int cnt, int *atoms, int mask);
extern int clear_status(ATAB *table, int atom, int mask);
extern int get_status(ATAB *table, int atom);
extern int find_invisible(ATAB *table);
extern int table_size(ATAB* table);
extern void set_shift(ATAB* table, int shift);
extern void set_postfix(ATAB* table, char *postfix);
extern void set_prefix(ATAB* table, char *prefix);
extern int set_module(ATAB *table, int atom, int module);
extern void name_invisible_atoms(char *prefix, ATAB *table);

/* Atom stacks */

typedef struct astack {
  int atom;
  int status;
  char *name;
  struct astack *under;
} ASTACK;

extern ASTACK *push(int atom, int status, char *name, ASTACK *stack);
extern ASTACK *pop(int *atom, int *status, char **name, ASTACK *stack);

/* Atom queues that define symbolic names for atoms */

#define AQ_DEF 1
#define AQ_STR 2
#define AQ_IDX 3
#define AQ_ATM 4

typedef struct aref {
  int atom;
  ATAB *table;
} AREF;

typedef union element {
  int atom;
  char *string;
  int index;
  AREF *aref;
} ELEMENT;

typedef struct qelem {
  int etype;
  ELEMENT elem;
  struct qelem *next;
} QELEM;

typedef struct aqueue {
  QELEM *first;
  QELEM *last;
} AQUEUE;

void qdef(int atom, AQUEUE *q);
void qstr(char *name, AQUEUE *q);
void qidx(int idx, AQUEUE *q);
void qatom(int atom, ATAB *table, AQUEUE *q);
QELEM *print_elem(FILE *out, QELEM *q);
