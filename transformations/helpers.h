/* SIMPLIFY - helpers.h
 * 
 * 2008 Axel Eirola
 */

/* Checks if array a contains elem, returns position of elem starting form 1 */
extern int contains_element(const int *a, const int a_cnt, const int elem);

/* Checks if arrays a and b have a common element, returns the element if found */
extern int has_common_element(const int *a, const int a_cnt, const int *b,
		const int b_cnt);

/* Checks if array a is a subset of array b, returns boolean */
extern int is_subset_of(const int *a, const int a_cnt, const int *b,
		const int b_cnt);

/* Removes elemnt form array */
extern int remove_element(int *a, const int a_cnt, const int at);

/* Removes duplicate elements from array */
extern int remove_duplicates(int *a, int a_cnt);
