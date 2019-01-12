#ifndef MODULE_H
#define MODULE_H

#include "compyglow.h"
#include "arglist.h"

#define NMODULES (1)

typedef struct Device {
    struct Device *next;
    char name[32];
    void *priv;
    unsigned int nled;
    int (*update)(void *priv, const CompyGlow_Color *c);
} Device;

typedef struct Module {
    struct Module *next;
    char name[64];
    char confname[32];
    void *priv;
    unsigned int (*probe)(void *priv, ArgList *args);
    unsigned int (*close)(void *priv);
} Module;

extern const Module *modules[];
extern Device *devices;

void addDevice(Device *d);
int removeDevice(Device *d);

#endif
