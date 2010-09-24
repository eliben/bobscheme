#ifndef CUTILS_H
#define CUTILS_H


#define TRUE 1
#define FALSE 0


/* Do not pass arguments with side effects (like ++i) to these
** macros.
*/
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))


#endif /* CUTILS_H */
