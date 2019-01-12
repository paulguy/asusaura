/* Based on this commit: https://github.com/CalcProgrammer1/KeyboardVisualizer/commit/8889f3b133d9c6c2a53f7cc21e1dc73bd42a675c */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <i2c/smbus.h>

#include "compyglow.h"
#include "module.h"
#include "i2c.h"
#include "util.h"

typedef enum {
    CT_RAM0 = 0,
    CT_RAM1,
    CT_RAM2,
    CT_RAM3,
    CT_MAINBOARD,
    CT_LAST
} ControllerType;

#define SET_REG_CMD    (0x00)
#define ACCESS_REG_CMD      (0x01)
#define COLOR_SET_CMD  (0x03)

#define BSWAP(X) ((X << 8) | (X >> 8))

#define LEDTYPE_MAX ((ControllerType)CT_LAST)

#define LEDC_MAX (4)

typedef struct {
    Device *parent;
    ControllerType type;
    int dev;
    unsigned char addr;
} LEDController;

typedef struct {
    int maindev;
    int auxdev;
    unsigned int nledc;
    Device dev[LEDC_MAX+1];
} ModuleState;

static LEDController ledc[LEDC_MAX+1];
static ModuleState ms;
static const char *LEDControllerTypes[LEDTYPE_MAX] = {"Asus Aura RAM 0",
                                                      "Asus Aura RAM 1",
                                                      "Asus Aura RAM 2",
                                                      "Asus Aura RAM 3",
                                                      "Asus Aura Mainboard"};
/* they're all the same now but in the future they may differ */
static const int ColorCounts[LEDTYPE_MAX] = {5,
                                             5,
                                             5,
                                             5,
                                             5};

int asusaura_update(void *priv, const CompyGlow_Color *c);

int asusaura_write(int i2cdev, unsigned char addr, unsigned short reg, unsigned char val) {
    if(set_i2c_slave(i2cdev, addr) < 0) {
        return(-1);
    }
    if(i2c_smbus_write_word_data(i2cdev, SET_REG_CMD, BSWAP(reg)) < 0) {
        fprintf(stderr, "Failed to set aura register: %s\n", strerror(errno));
        return(-1);
    }
    if(i2c_smbus_write_byte_data(i2cdev, ACCESS_REG_CMD, val) < 0) {
        fprintf(stderr, "Failed to write aura register: %s\n", strerror(errno));
        return(-1);
    }

    return(0);
}

int asusaura_read(int i2cdev, unsigned char addr, unsigned short reg) {
    int retval;

    if(set_i2c_slave(i2cdev, addr) < 0) {
        return(-1);
    }
    if(i2c_smbus_write_word_data(i2cdev, SET_REG_CMD, BSWAP(reg)) < 0) {
        fprintf(stderr, "Failed to set aura register: %s\n", strerror(errno));
        return(-1);
    }
    retval = i2c_smbus_read_byte_data(i2cdev, ACCESS_REG_CMD);
    if(retval < 0) {
        fprintf(stderr, "Failed to read aura register: %s\n", strerror(errno));
        return(-1);
    }

    return(retval);
}

int asusaura_write_colors(int i2cdev, unsigned char addr, const unsigned char *colors, int count) {
    if(set_i2c_slave(i2cdev, addr) < 0) {
        return(-1);
    }

    if(i2c_smbus_write_word_data(i2cdev, SET_REG_CMD, 0x0080) < 0) {
        fprintf(stderr, "Failed to set aura register: %s\n", strerror(errno));
        return(-1);
    }

 	if(i2c_smbus_write_block_data(i2cdev,
 	                              COLOR_SET_CMD,
 	                              count * 3,
 	                              colors) < 0) {
 	    fprintf(stderr, "Failed to write color block: %s\n", strerror(errno));
 	    return(-1);
 	}

    return(0);
}

void add_ledc(ModuleState *s,
              int i2cdev,
              unsigned char addr,
              ControllerType type) {
    if(s->nledc > LEDC_MAX) {
        fprintf(stderr, "WARNING: Too many LED controllers!\n");
        return;
    }

    ledc[s->nledc].parent = &(s->dev[s->nledc]);
    ledc[s->nledc].type = type;
    ledc[s->nledc].dev = i2cdev;
    ledc[s->nledc].addr = addr;

    STATIC_STRCPY(s->dev[s->nledc].name, LEDControllerTypes[type])
    s->dev[s->nledc].priv = &(ledc[s->nledc]);
    s->dev[s->nledc].nled = ColorCounts[type];
    s->dev->update = asusaura_update;

    addDevice(&(s->dev[s->nledc]));

    s->nledc++;
}

#define WRITE_OR_ERROR(DEV, ADDR, REG, VAL) \
    if(asusaura_write(DEV, ADDR, REG, VAL) < 0) { \
        fprintf(stderr, "WARNING: Error while initializing LED controller!\n"); \
        return(-1); \
    }

int init_ledc(int i2cdev, unsigned char addr) {
    if(i2c_probe(i2cdev, addr) < 0) {
        return(-1);
    }

    WRITE_OR_ERROR(i2cdev, addr, 0x8020, 0x01)
    WRITE_OR_ERROR(i2cdev, addr, 0x8021, 0x01)
    WRITE_OR_ERROR(i2cdev, addr, 0x8025, 0xFF)

    return(0);
}

void try_init_ledc(ModuleState *s,
                   int i2cdev,
                   unsigned char addr,
                   ControllerType type) {
    if(init_ledc(i2cdev, addr) == 0) {
        add_ledc(s, i2cdev, addr, type);
    }
}

int asusaura_initialize(ModuleState *s) {
    s->nledc = 0;

    if(s->maindev >= 0) {
        /* initialize RAM LED controllers if available */
        try_init_ledc(s, s->maindev, 0x70, CT_RAM0);
        try_init_ledc(s, s->maindev, 0x71, CT_RAM1);
        try_init_ledc(s, s->maindev, 0x72, CT_RAM2);
        try_init_ledc(s, s->maindev, 0x73, CT_RAM3);
    }

    if(s->auxdev >= 0) {
        /* initialize mainboard LED controller if available */
        try_init_ledc(s, s->auxdev, 0x4e, CT_MAINBOARD);
    }

    return(s->nledc);
}

unsigned int asusaura_probe(void *priv, ArgList *args) {
    ModuleState *s = priv;
    char *mainbus = NULL;
    char *auxbus = NULL;

    while(args != NULL) {
        if(strcmp("maindev", args->key) == 0) {
            mainbus = args->val;
        } else if(strcmp("auxdev", args->key) == 0) {
            auxbus = args->val;
        }
        args = args->next;
    }

    s->maindev = -1;
    s->auxdev = -1;
    if(mainbus != NULL) {
        s->maindev = open_i2c_dev(mainbus);
        if(s->maindev < 0) {
            fprintf(stderr, "WARNING: Failed to open main device!\n");
        }
    }
    if(auxbus != NULL) {
        s->auxdev = open_i2c_dev(auxbus);
        if(s->auxdev < 0) {
            fprintf(stderr, "WARNING: Failed to open aux device!\n");
        }
    }

    return(asusaura_initialize(s));
}

unsigned int asusaura_close(void *priv) {
    ModuleState *s = priv;
    unsigned int ndevs;

    if(s->maindev >= 0) {
        close(s->maindev);
    }
    if(s->auxdev >= 0) {
        close(s->auxdev);
    }
    ndevs = s->nledc;
    s->nledc = 0;

    return(ndevs);
}

int asusaura_update(void *priv, const CompyGlow_Color *c) {
    LEDController *lc = priv;
    unsigned int i;

    unsigned int count = ColorCounts[lc->type];

    unsigned char colors[count * 3];
    for(i = 0; i < count; i++) { /* RBG color I guess? */
        colors[i * 3] = c[i].r;
        colors[i * 3 + 1] = c[i].b;
        colors[i * 3 + 2] = c[i].g;
    }

    if(asusaura_write_colors(lc->dev, lc->addr, colors, count) < 0) {
        removeDevice(lc->parent);
        return(-1);
    }
    if(asusaura_write(lc->dev, lc->addr, 0x80A0, 0x01) < 0) {
        removeDevice(lc->parent);
        return(-1);
    }

    return(0);
}

Module asusaura_module = {
    .name = "Asus Aura",
    .confname = "asusaura",
    .priv = &ms,
    .probe = asusaura_probe,
    .close = asusaura_close
};
