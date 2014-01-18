/* SIMPLIFY - misc.c
 *
 * Miscalenous functions for simplify.
 *
 * 2008 Axel Eirola
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "symbol.h"
#include "atom.h"
#include "rule.h"

#include "occurrences.h"
extern OCCTAB *occtab;

/* Reads a smodels program from file *in, and runs it through smodels -internal
 * and replaces *in with the new program
 */
FILE* smodels_internal(FILE *in) {

	FILE *out = NULL;
	pid_t nPid;

	int pipefrom[2]; /* pipe to get smodels output */
	if (pipe(pipefrom) != 0) {
		perror("pipe() from");
		return NULL;
	}

	// Fork to child and parent
	nPid = fork();
	if (nPid < 0) {
		perror("fork() 1");
		return NULL;

	} else if (nPid == 0) {
		// Child function
		dup2(fileno(in), STDIN_FILENO); // Duplicate in to stdin
		dup2(pipefrom[1], STDOUT_FILENO); // Duplicate out to stdout
		/* close unnecessary pipe descriptors for a clean environment */
		close(pipefrom[0]);
		close(pipefrom[1]);
		/* call smodels */
		execlp("smodels", "smodels", "-internal", NULL);
		perror("execlp()");
		return NULL;

	} else {
		// Parent function

		/* Close unused pipe ends. This is especially important for the
		 * pipefrom[1] write descriptor, otherwise readFromPipe will never
		 * get an EOF. */
		close(pipefrom[1]);

		// Open stream for output
		if ((out = fdopen(pipefrom[0], "r")) == NULL) {
			perror("fdopen() r");
			return NULL;
		}

		/* We don't wait for fork, so that simplify can start reading output
		 * from smodels before finished, preventing the buffer to overflow.
		 */
	}

	return out;
}

/* Frees rule data from ANY_RULE according to type in RULE
 * Why this hasn't been implemented in the library I don't know
 */
extern void free_rule_data(RULE *rule) {
	switch (rule->type) {
	case TYPE_BASIC:
		free(rule->data.basic->neg);
		free(rule->data.basic);
		break;
	case TYPE_CONSTRAINT:
		free(rule->data.constraint->neg);
		free(rule->data.constraint);
		break;
	case TYPE_CHOICE:
		free(rule->data.choice->neg);
		free(rule->data.choice->head);
		free(rule->data.choice);
		break;
	case TYPE_WEIGHT:
		free(rule->data.weight->neg);
		//free(rule->data.weight->weight);
		free(rule->data.weight);
		break;
	}
	return;
}

/* Marks rule as removed, frees memory and updates head occurrances */
extern int remove_rule(RULE *rule) {

	// Update occurrences
	remove_rule_occurrences(rule, occtab);

	free_rule_data(rule);
	rule->type *= -1;

	return 0;
}

/* qsort int comparison function */
extern int int_cmp(const void *a, const void *b) {
	const int *ia = (const int *) a; // casting pointer types
	const int *ib = (const int *) b;
	return *ia - *ib;
	/* integer comparison: returns negative if b > a
	 and positive if a > b */
}

/* Returns the sum of integer array a */
extern int sum(const int *a, const int a_cnt) {
	int sum = 0;
	int i;

	for (i = 0; i < a_cnt; i++)
		sum += a[i];

	return sum;
}
