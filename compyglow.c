#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "compyglow.h"
#include "module.h"
#include "arglist.h"

ArgList *findArgsByPrefix(ArgList *args, const char *prefix) {
    ArgList *filteredArgs = NULL;
    ArgList *newArgs;
    char *dot;

    while(args != NULL) {
        dot = strchr(args->key, '.');
        if(dot == NULL) { /* global argument */
            newArgs = CompyGlow_appendArgList(filteredArgs,
                                              args->key,
                                              args->val);
            if(newArgs == NULL) {
                CompyGlow_freeArgList(filteredArgs);
                return(NULL);
            }
            filteredArgs = newArgs;
        } else { /* module argument */            
            *dot = '\0';
            if(strcmp(prefix, args->key) == 0) {
                newArgs = CompyGlow_appendArgList(filteredArgs,
                                                  &(dot[1]),
                                                  args->val);
                if(newArgs == NULL) {
                    CompyGlow_freeArgList(filteredArgs);
                    *dot = '.';
                    return(NULL);
                }
                filteredArgs = newArgs;
            }
            *dot = '.';
        }
        args = args->next;
    }

    return(filteredArgs);
}

unsigned int CompyGlow_open(ArgList *args) {
    unsigned int i;
    unsigned int count = 0;
    ArgList *moduleArgs;

    for(i = 0; i < NMODULES; i++) {
        moduleArgs = findArgsByPrefix(args, modules[i]->confname);
        count = modules[i]->probe(modules[i]->priv, moduleArgs);
        CompyGlow_freeArgList(moduleArgs);
    }

    return(count);
}

unsigned int CompyGlow_close() {
    unsigned int i;
    unsigned int count = 0;

    for(i = 0; i < NMODULES; i++) {
        count += modules[i]->close(modules[i]->priv);
    }

    devices = NULL;

    return(count);
}

unsigned int CompyGlow_getDevCount() {
    Device *d = devices;
    unsigned int count = 0;

    while(d != NULL) {
        d = d->next;
        count++;
    }

    return(count);
}

Device *getDevIndex(unsigned int idx) {
    Device *d = devices;
    unsigned int i;

    for(i = 0; i < idx; i++) {
        if(d == NULL) {
            return(NULL);
        }
        d = d->next;
    }

    return(d);
}

char *CompyGlow_getDevName(unsigned int idx) {
    Device *d = getDevIndex(idx);
    if(d == NULL) {
        return(NULL);
    }

    return(d->name);
}

CompyGlow_Color *getColorReq(unsigned int count) {
    return(malloc(sizeof(CompyGlow_Color) * count));
}


unsigned int CompyGlow_getColorReq(unsigned int idx, CompyGlow_Color **c) {
    Device *d = getDevIndex(idx);
    if(d == NULL) {
        return(0);
    }

    *c = getColorReq(d->nled);
    if(*c == NULL) {
        return(0);
    }

    return(d->nled);
}

int CompyGlow_update(unsigned int idx, CompyGlow_Color *c) {
    Device *d = getDevIndex(idx);
    if(d == NULL) {
        return(-1);
    }

    return(d->update(d->priv, c));
}
