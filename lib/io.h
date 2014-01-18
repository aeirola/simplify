/*
 * Definitions related with input and output routines
 *
 * (c) 2005 Tomi Janhunen
 */

#define _IO_H_RCSFILE  "$RCSfile: io.h,v $"
#define _IO_H_DATE     "$Date: 2007/11/19 14:33:32 $"
#define _IO_H_REVISION "$Revision: 1.3 $"

extern void _version(char *rcs_file, char *rcs_date, char *rcs_revision);

/* input.c */

extern void _version_input_c();

extern char *program_name;
extern void error(char *msg);
extern char *read_string(FILE *in);

extern RULE *read_program(FILE *in);
extern ATAB *read_symbols(FILE *in);
extern int read_compute_statement(FILE *in, ATAB *table);

extern RULE *read_cnf(FILE *in, ATAB **table);

/* output.c */

#define STYLE_READABLE 1  /* Symbolic smodels/dimacs format */
#define STYLE_SMODELS  2  /* Internal smodels format */
#define STYLE_DIMACS   3  /* Internal dimacs format */
#define STYLE_GNT      4  /* Symbolic gnt format */
#define STYLE_DLV      5  /* Symbolic dlv format */

extern void _version_output_c();

extern void write_atom(int style, FILE *out, int atom, ATAB *table);
extern void write_other_atom(int style, FILE *out, int atom, ATAB *table);
extern void write_program(int style, FILE *out, RULE *program, ATAB *table);

extern void write_symbols(int style, FILE *out, ATAB *table);

extern void write_compute_statement(int style, FILE *out, ATAB *table,
				    int mask);

extern void write_classical_atom(int style, FILE *out, int atom,
				 ATAB *table);
extern void write_other_classical_atom(int style, FILE *out, int atom,
				       ATAB *table);
extern void write_cnf(int style, FILE *out, RULE *cnf, ATAB *table);
