/* SIMPLIFY - readwrite.c
 *
 * Program shell that takes care of IO, argument parsing and data structure
 * initialization. Essentialy just makes everyting ready for simplification.c
 *
 * 2008 Axel Eirola
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"
#include "io.h"

#include "readwrite.h"
#include "misc.h"
#include "simplification.h"
#include "transformations/compute.h"

#include "stats.h"
STAT *stats;
#include "options.h"
OPT *options;

#include "occurrences.h"
OCCTAB *occtab;
ATAB *atab;

/* Print version information */
void _version_simplify_c() {
	fprintf(stdout, "%s: v0.3 [2008/27/8] \n", program_name);
	/*fprintf(stdout, "%s: version information: \n", program_name);
	 _version("$RCSfile: simplify.c,v $", "$Date: 2008/22/07 10:41:35 $",
	 "$Revision: 0.1.1 $");
	fprintf(stdout, "\nused libraries:\n");
	_version_atom_c();
	_version_symbol_c();
	_version_rule_c();
	_version_input_c();
	_version_output_c();*/
}

/* Print usage */
void usage() {
	fprintf(stdout, USAGE_STRING);
	return;
}

/* Main program */
int main(int argc, char **argv) {

	char *file = NULL;
	char *arg;

	FILE *in = NULL;
	FILE *out = stdout;

	FILE *prog = NULL;
	RULE *program = NULL;
	atab = NULL;
	int models;

	/* Initialize default options */
	options = malloc(sizeof(OPT));
	options->triv = 1;
	options->compute = 1;
	options->taut = 1;
	options->contra = 1;

	options->nonmin = 1;
	options->litnonmin = 1;

	options->partev = 0;
	options->partev_full = 0;

	options->smodels = 0;

	options->stats = 0;
	options->verbose = 0;
	options->loop = 0;

	/* Read arguments */
	program_name = argv[0];

	int n;
	int m;
	for (n = 1; n < argc; n++) {
		arg = argv[n];

		// Shorthand arguments
		if (arg[0] == '-' && isalpha(arg[1])) {
			m = 1;
			while (arg[m] != 0) {
				switch (arg[m]) {
				case 'h':
					usage();
					exit(0);
				case 'V':
					_version_simplify_c();
					exit(0);
				case 'v':
					options->verbose = 1;
					options->stats = 1;
					break;
				case 'S':
					options->stats = 1;
					break;

				case 'A':
					options->triv = 1;
					options->compute = 1;
					options->taut = 1;
					options->contra = 1;
					options->nonmin = 1;
					options->litnonmin = 1;
					options->partev = 1;
					options->partev_full = 1;
					options->loop = 1;
					break;
				case 'N':
					options->triv = 0;
					options->compute = 0;
					options->taut = 0;
					options->contra = 0;
					options->nonmin = 0;
					options->litnonmin = 0;
					options->partev = 0;
					options->partev_full = 0;
					options->loop = 0;
					break;
				case 'T':
					options->triv = 1;
					break;
				case 'C':
					options->compute = 1;
					break;
				case 's':
					options->smodels = 1;
					break;
				case 't':
					options->taut = 1;
					break;
				case 'c':
					options->contra = 1;
					break;
				case 'n':
					options->nonmin = 1;
					break;
				case 'l':
					options->litnonmin = 1;
					break;
				case 'p':
					options->partev = 1;
					break;
				case 'P':
					options->partev = 1;
					options->partev_full = 1;
					break;
				case 'L':
					options->loop = 1;
					break;
				default:
					usage();
					exit(-1);
				}
				m++;
			}
		}

		// Long arguments
		else if (strcmp(arg, "--help") == 0) {
			usage();
			exit(0);
		} else if (strcmp(arg, "--version") == 0) {
			_version_simplify_c();
			exit(0);
		} else if (strcmp(arg, "--verbose") == 0) {
			options->verbose = 1;
			options->stats = 1;
		} else if (strcmp(arg, "--stats") == 0)
			options->stats = 1;

		else if (strcmp(arg, "--all") == 0) {
			options->triv = 1;
			options->compute = 1;
			options->taut = 1;
			options->contra = 1;
			options->nonmin = 1;
			options->litnonmin = 1;
			options->partev = 1;
			options->loop = 1;
		} else if (strcmp(arg, "--none") == 0) {
			options->triv = 0;
			options->compute = 0;
			options->taut = 0;
			options->contra = 0;
			options->nonmin = 0;
			options->litnonmin = 0;
			options->partev = 0;
			options->loop = 0;

		} else if (strcmp(arg, "--trival") == 0)
			options->triv = 1;
		else if (strcmp(arg, "--compute") == 0)
			options->compute = 1;
		else if (strcmp(arg, "--taut") == 0)
			options->taut = 1;
		else if (strcmp(arg, "--contra") == 0)
			options->contra = 1;
		else if (strcmp(arg, "--nonmin") == 0)
			options->nonmin = 1;
		else if (strcmp(arg, "--litnon") == 0)
			options->litnonmin = 1;
		else if (strcmp(arg, "--partev") == 0)
			options->partev = 1;
		else if (strcmp(arg, "--partev_full") == 0) {
			options->partev = 1;
			options->partev_full = 1;
		} else if (strcmp(arg, "--smodels") == 0)
			options->smodels = 1;
		else if (strcmp(arg, "--loop") == 0)
			options->loop = 1;

		else if (file == NULL)
			file = arg;

		else {
			fprintf(stderr, "%s: unknown argument %s\n", program_name, arg);
			usage();
			exit(-1);
		}
	}

	/* Open file */
	if (file == NULL || strcmp("-", file) == 0) {
		in = stdin;
	} else {
		if ((in = fopen(file, "r")) == NULL) {
			fprintf(stderr, "%s: cannot open file %s\n", program_name, file);
			exit(-1);
		}
	}

	/* Call smodels -internal */
	if (options->smodels) {
		if (!system("which smodels > /dev/null")) {
			if ((prog = smodels_internal(in)) == NULL) {
				fputs("smodels -internal fork failed!", stderr);
				exit(-1);
			}
		} else
			fputs("smodels not found, skipping -S\n", stderr);
	} else
		prog = in;

	/* Read program */
	program = read_program(prog);
	atab = read_symbols(prog);
	models = read_compute_statement(prog, atab);

	/* Close files */
	if (prog != in)
		fclose(in);
	fclose(prog);

	/* Initialize statistics */
	if ((stats = calloc(1, sizeof(STAT))) == NULL) {
		fputs("Could not initialize statistics!", stderr);
		exit(-1);
	}

	// Create occurrences table
	occtab = initialize_occurrences(atab);
	compute_occurrences(program, occtab);

	/* Call actual logic */
	simplify(&program);

	/* Output program */
	if (options->verbose) {
		fprintf(out, "%% simplify %s\n", file);
		write_program(STYLE_READABLE, out, program, atab);
		fprintf(out, "\n");
		fprintf(out, "compute { ");
		write_compute_statement(STYLE_READABLE, out, atab, MARK_TRUE
				| MARK_FALSE);
		fprintf(out, " }.\n\n");

	} else { /* !verbose_mode */
		write_program(STYLE_SMODELS, out, program, atab);
		fprintf(out, "0\n");

		write_symbols(STYLE_SMODELS, out, atab);
		fprintf(out, "0\n");

		fprintf(out, "B+\n");
		write_compute_statement(STYLE_SMODELS, out, atab, MARK_TRUE);
		fprintf(out, "0\n");

		fprintf(out, "B-\n");
		write_compute_statement(STYLE_SMODELS, out, atab, MARK_FALSE);
		fprintf(out, "0\n");

		fprintf(out, "%i\n", models);
	}

	/* Output statistics */
	if (options->stats) {

		// Dump to stderr if not in verbose mode
		if (!options->verbose)
			out = stderr;

		// Start printing
		fprintf(out, "%% Simplification statistics (reduced / removed):\n");
		fprintf(out, "%%  Trans  \tBasic  \tConstr\tChoice\tWeight\tTotal");

		int red_sub_tot = 0, rem_sub_tot = 0;
		int red_tot = 0, rem_tot = 0;

		int i;
		if (options->taut) {
			fprintf(out, "\n%%  Taut  ");
			for (i = 1; i < 6; i++)
				if (i != 4) {
					red_sub_tot += stats->taut_reduced[i];
					rem_sub_tot += stats->taut_removed[i];
					fprintf(out, "\t%i/%i", stats->taut_reduced[i],
							stats->taut_removed[i]);
				}
			fprintf(out, "\t%i/%i", red_sub_tot, rem_sub_tot);
			red_tot += red_sub_tot;
			rem_tot += rem_sub_tot;
			red_sub_tot = 0;
			rem_sub_tot = 0;
		}

		if (options->contra) {
			fprintf(out, "\n%%  Contra ");
			for (i = 1; i < 6; i++)
				if (i != 4) {
					red_sub_tot += stats->contra_reduced[i];
					rem_sub_tot += stats->contra_removed[i];
					fprintf(out, "\t%i/%i", stats->contra_reduced[i],
							stats->contra_removed[i]);
				}
			fprintf(out, "\t%i/%i", red_sub_tot, rem_sub_tot);
			red_tot += red_sub_tot;
			rem_tot += rem_sub_tot;
			red_sub_tot = 0;
			rem_sub_tot = 0;
		}

		if (options->triv) {
			fprintf(out, "\n%%  Triv  ");
			for (i = 1; i < 6; i++)
				if (i == 1 || i == 3)
					fprintf(out, "\t -");
				else if (i != 4) {
					red_sub_tot += stats->triv_reduced[i];
					rem_sub_tot += stats->triv_removed[i];
					fprintf(out, "\t%i/%i", stats->triv_reduced[i],
							stats->triv_removed[i]);
				}
			fprintf(out, "\t%i/%i", red_sub_tot, rem_sub_tot);
			red_tot += red_sub_tot;
			rem_tot += rem_sub_tot;
			red_sub_tot = 0;
			rem_sub_tot = 0;
		}
		if (options->compute) {
			fprintf(out, "\n%%  Comp  ");
			for (i = 1; i < 6; i++)
				if (i != 4) {
					red_sub_tot += stats->compute_reduced[i];
					rem_sub_tot += stats->compute_removed[i];
					fprintf(out, "\t%i/%i", stats->compute_reduced[i],
							stats->compute_removed[i]);
				}
			fprintf(out, "\t%i/%i", red_sub_tot, rem_sub_tot);
			red_tot += red_sub_tot;
			rem_tot += rem_sub_tot;
			red_sub_tot = 0;
			rem_sub_tot = 0;
		}
		if (options->nonmin) {
			fprintf(out, "\n%%  Nonmin ");
			for (i = 1; i < 6; i++)
				if (i != 4) {
					red_sub_tot += stats->nonmin_reduced[i];
					rem_sub_tot += stats->nonmin_removed[i];
					fprintf(out, "\t%i/%i", stats->nonmin_reduced[i],
							stats->nonmin_removed[i]);
				}
			fprintf(out, "\t%i/%i", red_sub_tot, rem_sub_tot);
			red_tot += red_sub_tot;
			rem_tot += rem_sub_tot;
			red_sub_tot = 0;
			rem_sub_tot = 0;
		}
		if (options->litnonmin) {
			fprintf(out, "\n%%  LitNon ");
			for (i = 1; i < 6; i++)
				if (i != 4) {
					red_sub_tot += stats->litnonmin_reduced[i];
					rem_sub_tot += stats->litnonmin_removed[i];
					fprintf(out, "\t%i/%i", stats->litnonmin_reduced[i],
							stats->litnonmin_removed[i]);
				}
			fprintf(out, "\t%i/%i", red_sub_tot, rem_sub_tot);
			red_tot += red_sub_tot;
			rem_tot += rem_sub_tot;
			red_sub_tot = 0;
			rem_sub_tot = 0;
		}

		if (options->partev) {
			fprintf(out, "\n%%  PartEv");
			for (i = 1; i < 6; i++)
				if (i != 4) {
					red_sub_tot += stats->partev_reduced[i];
					rem_sub_tot += stats->partev_removed[i];
					fprintf(out, "\t%i/%i", stats->partev_reduced[i],
							stats->partev_removed[i]);
				}
			fprintf(out, "\t%i/%i", red_sub_tot, rem_sub_tot);
			red_tot += red_sub_tot;
			rem_tot += rem_sub_tot;
		}

		fprintf(out, "\n%%\n%%  Total ");
		for (i = 1; i < 6; i++)
			if (i != 4)
				fprintf(out, "\t%i/%i", stats->taut_reduced[i]
						+ stats->contra_reduced[i] + stats->triv_reduced[i]
						+ stats->compute_reduced[i] + stats->nonmin_reduced[i]
						+ stats->litnonmin_reduced[i]
						+ stats->partev_reduced[i], stats->taut_removed[i]
						+ stats->contra_removed[i] + stats->triv_removed[i]
						+ stats->compute_removed[i] + stats->nonmin_removed[i]
						+ stats->litnonmin_removed[i]
						+ stats->partev_removed[i]);
		fprintf(out, "\t%i/%i", red_tot, rem_tot);

		fprintf(out, "\n%%");
		if (options->loop)
			fprintf(out, "\n%%  Loops: %i", stats->loops);
		fprintf(out, "\n%%  Total transformations: %i\n", red_tot + rem_tot);
	}

	/* Free stuff */
	free(stats);
	free(options);

	exit(0);
}

