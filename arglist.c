#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arglist.h"
#include "util.h"

ArgList *CompyGlow_appendArgList(ArgList *args, char *key, char *val) {
    ArgList *last;

    if(args == NULL) {
        args = malloc(sizeof(ArgList));
        if(args == NULL) {
            return(NULL);
        }
        args->next = NULL;
        last = args;
    } else {
        last = args;
        while(last->next != NULL) {
            last = last->next;
        }
        last->next = malloc(sizeof(ArgList));
        if(last->next == NULL) {
            return(NULL);
        }
        last = last->next;
        last->next = NULL;
    }

    STATIC_STRCPY(last->key, key)
    if(val == NULL) {
        last->val[0] = '\0';
    } else {
        STATIC_STRCPY(last->val, val)
    }

    return(args);
}

void CompyGlow_freeArgList(ArgList *args) {
    ArgList *next;

    while(args != NULL) {
        next = args->next;
        free(args);
        args = next;
    }
}
