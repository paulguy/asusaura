#include <stdlib.h>

#include "module.h"

//modules
#include "asusaura.h"

const Module *modules[] = { &asusaura_module };

Device *devices = NULL;

void addDevice(Device *d) {
    Device *dl;

    if(devices == NULL) {
        devices = d;
        return;
    }

    dl = devices;
    while(dl->next != NULL) {
        dl = dl->next;
    }

    dl->next = d;
    dl->next->next = NULL;

    return;
}

int removeDevice(Device *d) {
    Device *dl;
    int num = 0;

    if(devices == NULL) {
        return(-1);
    }

    if(devices == d) {
        devices = NULL;
        return(0);
    }

    dl = devices;
    while(dl->next != NULL) {
        num++;
        if(dl->next == d) {
            dl->next = dl->next->next;
            return(num);
        }
        dl = dl->next;
    }

    return(-1);
}
