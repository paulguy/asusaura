#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compyglow.h"
#include "arglist.h"

int main(int argc, char **argv) {
    CompyGlow_Color *colors;
    unsigned int count;
    unsigned int devs;
    int arg;
    unsigned int i, j;
    ArgList *args = NULL;
    ArgList *tempArgs;
    char key[ARGLIST_KEYLEN_MAX];
    char val[ARGLIST_VALLEN_MAX];
    char *equals;
    unsigned int minval;
    int r = -1;
    int g = -1;
    int b = -1;;

    for(arg = 1; arg < argc; arg++) {
        equals = strchr(argv[arg], '=');
        if(equals == NULL) { /* flag */
            tempArgs = CompyGlow_appendArgList(args, argv[arg], NULL);
            if(tempArgs == NULL) {
                CompyGlow_freeArgList(args);
                fprintf(stderr, "Error adding argument to list.\n");
                exit(EXIT_FAILURE);
            }
            args = tempArgs;
        } else {
            minval = equals - argv[arg];
            if(minval >= sizeof(key)) {
                minval = sizeof(key) - 1;
            }
            strncpy(key, argv[arg], minval);
            key[minval] = '\0';

            minval = strlen(argv[arg]) - ((equals + 1) - argv[arg]);
            if(minval >= sizeof(val)) {
                minval = sizeof(val) - 1;
            }
            strncpy(val, &(equals[1]), minval);
            val[minval] = '\0';
            
            tempArgs = CompyGlow_appendArgList(args, key, val);
            if(tempArgs == NULL) {
                CompyGlow_freeArgList(args);
                fprintf(stderr, "Error adding argument to list.\n");
                exit(EXIT_FAILURE);
            }
            args = tempArgs;
        }
    }

    tempArgs = args;
    while(tempArgs != NULL) {
        //fprintf(stderr, "%s = %s\n", tempArgs->key, tempArgs->val);

        if(strcmp("r", tempArgs->key) == 0) {
            r = atoi(tempArgs->val);
        } else if(strcmp("g", tempArgs->key) == 0) {
            g = atoi(tempArgs->val);
        } else if(strcmp("b", tempArgs->key) == 0) {
            b = atoi(tempArgs->val);
        }

        tempArgs = tempArgs->next;
    }

    devs = CompyGlow_open(args);
    CompyGlow_freeArgList(args);
    fprintf(stderr, "Opened %u devices.\n", devs);

    if(r == -1 || g == -1 || b == -1) {
        for(i = 0; i < devs; i++) {
            fprintf(stderr, "%u: %s\n", i, CompyGlow_getDevName(i));
        }
    } else {
        for(i = 0; i < devs; i++) {
            count = CompyGlow_getColorReq(i, &colors);

            for(j = 0; j < count; j++) {
                colors[j].r = (unsigned char)r;
                colors[j].g = (unsigned char)g;
                colors[j].b = (unsigned char)b;
            }

            if(CompyGlow_update(i, colors) < 0) {
                fprintf(stderr, "Couldn't update colors.\n");
           }

           free(colors);
        }
    }

    fprintf(stderr, "%u devices closed.\n", CompyGlow_close());
    exit(EXIT_SUCCESS);
}
