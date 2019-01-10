/* Based on this commit: https://github.com/CalcProgrammer1/KeyboardVisualizer/commit/8889f3b133d9c6c2a53f7cc21e1dc73bd42a675c */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include "asusaura.h"

#define SET_REG_CMD    (0x00)
#define ACCESS_REG_CMD      (0x01)
#define COLOR_SET_CMD  (0x03)

#define BSWAP(X) ((X << 8) | (X >> 8))

#define LEDTYPE_MAX ((AsusAura_ControllerType)ASUSAURA_NONE)

typedef struct {
    AsusAura_ControllerType type;
    int dev;
    unsigned char addr;
} LEDController;

static int maindev = -1;
static int auxdev = -1;
#define LEDC_MAX (4)
static unsigned int nledc;
static LEDController ledc[LEDC_MAX+1];
static const char *LEDControllerTypes[LEDTYPE_MAX+1] = {"RAM 0",
                                                        "RAM 1",
                                                        "RAM 2",
                                                        "RAM 3",
                                                        "Mainboard",
                                                        "None"};
/* they're all the same now but in the future they may differ */
static const int ColorCounts[LEDTYPE_MAX+1] = {5,
                                               5,
                                               5,
                                               5,
                                               5,
                                               -1};

int open_i2c_dev(const char *devpath) {
    int i2cdev;
    unsigned long funcs;

    i2cdev = open(devpath, O_RDWR);
    if (i2cdev < 0) {
        fprintf(stderr, "Couldn't open device %s: %s\n", devpath, strerror(errno));
        return(-1);
    }

    if(ioctl(i2cdev, I2C_FUNCS, &funcs) < 0) {
        fprintf(stderr, "Couldn't get i2c functions: %s\n", strerror(errno));
        close(i2cdev);
        return(-1);
    }

    if(!(funcs & I2C_FUNC_SMBUS_WRITE_WORD_DATA) ||
       !(funcs & I2C_FUNC_SMBUS_WRITE_BYTE_DATA) ||
       !(funcs & I2C_FUNC_SMBUS_WRITE_BLOCK_DATA) ||
       !(funcs & I2C_FUNC_SMBUS_READ_BYTE_DATA)) {
        fprintf(stderr, "Missing functions for this i2c device.\n");
        close(i2cdev);
        return(-1);
    }

    return(i2cdev);
}

int set_i2c_slave(int i2cdev, unsigned char addr) {
    if(ioctl(i2cdev, I2C_SLAVE, addr) < 0) {
        fprintf(stderr, "Couldn't set i2c slave addr %hhu: %s\n", addr, strerror(errno));
        return(-1);
    }

    return(0);
}

int i2c_probe(int i2cdev, unsigned char addr) {
    /* probe based on i2cdetect method */
    if(set_i2c_slave(i2cdev, addr) < 0) {
        return(-1);
    }

    return(i2c_smbus_write_quick(i2cdev, I2C_SMBUS_WRITE));
}

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

void add_ledc(int i2cdev, unsigned char addr, AsusAura_ControllerType type) {
    if(nledc > LEDC_MAX) {
        fprintf(stderr, "WARNING: Too many LED controllers!\n");
        return;
    }

    ledc[nledc].type = type;
    ledc[nledc].dev = i2cdev;
    ledc[nledc].addr = addr;

    nledc++;
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

void try_init_ledc(int i2cdev, unsigned char addr, AsusAura_ControllerType type) {
    if(init_ledc(i2cdev, addr) == 0) {
        add_ledc(i2cdev, addr, type);
    }
}

int asusaura_initialize(int maindev, int auxdev) {
    nledc = 0;
    unsigned int i;

    for(i = 0; i < LEDC_MAX; i++) {
        ledc[nledc].type = ASUSAURA_NONE;
        ledc[nledc].dev = -1;
        ledc[nledc].addr = 0;
    }

    if(maindev >= 0) {
        /* initialize RAM LED controllers if available */
        try_init_ledc(maindev, 0x70, ASUSAURA_RAM0);
        try_init_ledc(maindev, 0x71, ASUSAURA_RAM1);
        try_init_ledc(maindev, 0x72, ASUSAURA_RAM2);
        try_init_ledc(maindev, 0x73, ASUSAURA_RAM3);
    }

    if(auxdev >= 0) {
        /* initialize mainboard LED controller if available */
        try_init_ledc(auxdev, 0x4e, ASUSAURA_MAINBOARD);
    }

    return(nledc);
}

int asusaura_open(const char *mainbus, const char *auxbus) {
    if(mainbus != NULL) {
        maindev = open_i2c_dev(mainbus);
        if(maindev < 0) {
            fprintf(stderr, "WARNING: Failed to open main device!\n");
        }
    }
    if(auxbus != NULL) {
        auxdev = open_i2c_dev(auxbus);
        if(auxdev < 0) {
            fprintf(stderr, "WARNING: Failed to open aux device!\n");
        }
    }

    return(asusaura_initialize(maindev, auxdev));
}

void asusaura_close() {
    if(maindev >= 0) {
        close(maindev);
    }
    if(auxdev >= 0) {
        close(auxdev);
    }
}

int asusaura_dev_count() {
    return(nledc);
}

AsusAura_ControllerType asusaura_getType(unsigned int idx) {
    if(idx > LEDC_MAX) {
        return(ASUSAURA_NONE);
    }

    return(ledc[idx].type);
}

const char *asusaura_typeStr(AsusAura_ControllerType type) {
    if(type < 0 || type > LEDTYPE_MAX)
        return(LEDControllerTypes[ASUSAURA_NONE]);

    return(LEDControllerTypes[type]);
}

unsigned int asusaura_getColorReq(unsigned int idx, AsusAura_Color **c) {
    int count = ColorCounts[asusaura_getType(idx)];
    if(count == -1)
        return(-1);

    *c = malloc(sizeof(AsusAura_Color) * count);
    if(*c == NULL) {
        return(-1);
    }

    return(count);
}

void asusaura_freeColorReq(AsusAura_Color *c) {
    free(c);
}

int asusaura_update(unsigned int idx, const AsusAura_Color *c) {
    int i;

    int count = ColorCounts[asusaura_getType(idx)];
    if(count == -1) {
        return(-1);
    }

    unsigned char colors[count * 3];
    for(i = 0; i < count; i++) { /* RBG color I guess? */
        colors[i * 3] = c[i].r;
        colors[i * 3 + 1] = c[i].b;
        colors[i * 3 + 2] = c[i].g;
    }

    if(asusaura_write_colors(ledc[idx].dev, ledc[idx].addr, colors, count) < 0) {
        return(-1);
    }
    WRITE_OR_ERROR(ledc[idx].dev, ledc[idx].addr, 0x80A0, 0x01)

    return(0);
}
