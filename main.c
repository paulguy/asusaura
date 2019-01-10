#include <stdio.h>
#include <stdlib.h>

#include "asusaura.h"

int main(int argc, char **argv) {
    AsusAura_Color *colors;
    unsigned int count;
    unsigned int devs;
    unsigned int i, j;

    if(argc < 3) {
        fprintf(stderr, "USAGE: %s <mainbus> <auxbus> [<r> <g> <b>]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    devs = asusaura_open(argv[1], argv[2]);
    if(argc < 6) {
        fprintf(stderr, "Found %u devices.\n", devs);

        for(i = 0; i < devs; i++) {
            fprintf(stderr, "%u: %s\n", i, asusaura_typeStr(asusaura_getType(i)));
        }
    } else {
        for(i = 0; i < devs; i++) {
            count = asusaura_getColorReq(i, &colors);

            for(j = 0; j < count; j++) {
                colors[j].r = atoi(argv[3]);
                colors[j].g = atoi(argv[4]);
                colors[j].b = atoi(argv[5]);
            }

            if(asusaura_update(i, colors) < 0) {
                fprintf(stderr, "Couldn't update colors.\n");
           }

           asusaura_freeColorReq(colors);
        }
    }

    asusaura_close();
    exit(EXIT_SUCCESS);
}
