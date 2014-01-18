/* SIMPLIFY - helpers.c
 *
 * Helper functions for logical simplification algorithms.
 *
 * Note that all functions in this file assumes the given rules are normalized as
 * done by normalize_rule()
 *
 * 2008 Axel Eirola
 */

#include <stdlib.h>
#include <string.h>

// Checks if sorted array a contains elem, returns position of elem + 1
extern int contains_element(const int *a, const int a_cnt, const int elem) {

	// Decide wich algorithm to use
	if(a_cnt < 5) {
		// Linear search
		int i;
		for (i=0; i < a_cnt && !(a[i] > elem); i++)
			if (a[i] == elem)
					return i+1;

		return 0;
	} else {
		// Binary search
		int mid;
		int low = 0;
		int high = a_cnt - 1;

		while (low <= high) {
			mid = (low + high) / 2;
			if (a[mid] > elem)
				high = mid-1;
			else if (a[mid] < elem)
				low = mid+1;
			else
				return mid+1; // found
		}
		return 0; // not found
	}
}

/* Checks if sorted arrays a and b have a common element,
 * Returns the common element if found */
extern int has_common_element(const int *a, const int a_cnt, const int *b,
		const int b_cnt) {

	int i;
	// Decide wich algorithm to use
	if (b_cnt < 10) {
		// Linear search
		int j = 0;
		for (i=0; i < a_cnt; i++)
			for (; j < b_cnt && !(b[j] > a[i]); j++)
				if (a[i] == b[j])
					return a[i];

		return 0;

	} else {
		// Binary search
		int low, high;
		int mid = 0;

		for (i=0; i < a_cnt; i++) {
			high = b_cnt-1;
			low = mid;
			while (low <= high) {
				mid = (low + high) / 2;
				if (b[mid] > a[i])
					high = mid-1;
				else if (b[mid] < a[i])
					low = mid+1;
				else
					return a[i]; // found
			}
		}
		return 0;
	}
}

// Checks if sorted array a is a subset of sorted array b, returns boolean value
extern int is_subset_of(const int *a, const int a_cnt, const int *b,
		const int b_cnt) {

	if (a_cnt > b_cnt)
		return 0;

	int i;
	// Decide wich algorithm to use
	if (b_cnt < 10) {
		// Linear search
		if (a_cnt > b_cnt)
			return 0;
		int j = 0;
		for (i = 0; i < a_cnt; i++)
			for (;; j++) // Sorted array allows us to return where we left off
				if (j == b_cnt || b[j] > a[i])
					return 0;
				else if (a[i] == b[j])
					break;

		return 1;

	} else {
		// Binary search
		int mid, high;
		int low = -1;

		for (i = 0; i < a_cnt; i++) {
			high = b_cnt-1;
			low++; // Sorted array allows us to return where we left off
			while (low < high) {
				mid = (low + high) / 2;
				if (b[mid] < a[i])
					low = mid+1;
				else
					high = mid;
			}
			if (b[low] != a[i])
				return 0;
		}
		return 1;
	}
}


/* Removes element at given position from array, does not decrement count */
extern void remove_element(int *a, const int a_cnt, const int at) {
	memmove(&(a[at]), &(a[at+1]), (a_cnt-(at+1))*sizeof(int)); // Move memory

	/* Unable to safely free unused memory due to structure of rule bodys with
	 * whole body (negative, positive and weights) allocated as one */

	return;
}

/* Remove duplicate integers from array of size a_cnt, does not reallocate mem.
 * Returns the amount of removed elements. */
extern int remove_duplicates(int *a, const int a_cnt) {
	int removed = 0;
	int i;
	int size = a_cnt;
	for (i=1; i < size; i++)
		if (a[i-1] == a[i]) {
			memmove(&(a[i-1]), &(a[i]), (size - i) * sizeof(int));
			removed++;
			size--; // So that we don't loop over the end, removing last elem
		}

	return removed;
}
