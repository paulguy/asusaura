#include <stdio.h>
#include <stdlib.h>

#include "asusaura.h"

int main(int argc, char **argv) {
    unsigned char colors[ASUSAURA_COLORS_SIZE];
    unsigned int devs;
    unsigned int i;

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
        colors[0] = colors[3] = colors[6] = colors[9] = colors[12] = atoi(argv[3]);
        colors[1] = colors[4] = colors[7] = colors[10] = colors[13] = atoi(argv[4]);
        colors[2] = colors[5] = colors[8] = colors[11] = colors[14] = atoi(argv[5]);

        for(i = 0; i < devs; i++) {
            if(asusaura_update(i, colors) < 0) {
                fprintf(stderr, "Couldn't update colors.\n");
           }
        }
    }

    asusaura_close();
    exit(EXIT_SUCCESS);
}
