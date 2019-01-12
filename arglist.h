#ifndef ARGLIST_H
#define ARGLIST_H

#define ARGLIST_KEYLEN_MAX (64)
#define ARGLIST_VALLEN_MAX (192)

/*
 * Usage Summary
 *
 * Initially, appendArgList should be called with NULL as the args to create a
 * new argument list with the first key and value passed in.  Additional
 * arguments may appended by calling the function additional times.  This is
 * based on a linked list so while you may pass any member of the list in, you
 * should avoid losing the head.  The function will return NULL on failures so
 * the return value should be assigned to a temporary pointer if appending
 * otherwise the head may be lost if it's just assigned back to the original
 * pointer.  When the list is no longer needed, it should be freed by passing
 * the head to freeArgList.  Any key or value pointers will be invalid after
 * freeing.
 */

typedef struct ArgList {
    char key[ARGLIST_KEYLEN_MAX];
    char val[ARGLIST_VALLEN_MAX];
    struct ArgList *next;
} ArgList;

/*
 * Append an argument to a list or create an initial list.
 *
 * args
 *   List to append to or NULL to create a new one.
 *
 * key
 *   Key of argument to append.
 *
 * val
 *   Value of argument to append.  If NULL, the value will just be an empty
 *   string.
 *
 * return
 *   New argument list or NULL if failed.  This should be assigned to a
 *   temporary pointer and checked for NULL to avoid a memory leak on error.
 */
ArgList *CompyGlow_appendArgList(ArgList *args, char *key, char *val);

/*
 * Free an argument list.
 *
 * args
 *   List to free.  NULL will do nothing.
 */
void CompyGlow_freeArgList(ArgList *args);

#endif
