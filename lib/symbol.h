/*
 * Symbol table to LPEQ etc
 *
 * (c) 2006 Tomi Janhunen
 */

/*
 * Definitions related to symbols (names of atoms)
 */

#define _SYMBOL_H_RCSFILE  "$RCSfile: symbol.h,v $"
#define _SYMBOL_H_DATE     "$Date: 2007/07/09 11:25:54 $"
#define _SYMBOL_H_REVISION "$Revision: 1.2 $"

typedef struct info {
  int atom;     /* Atom number */
  int module;   /* Module number */
} INFO;

typedef struct symbol {
  char *name;          /* String */
  INFO info;           /* Data associated with this symbol (if any) */
  struct symbol *next; /* Next entry */
} SYMBOL;

extern void symbol_table_init();
extern SYMBOL *make_symbol(char *);
extern void print_symbol(FILE *out, SYMBOL *);
extern SYMBOL *find_symbol(char *name);
