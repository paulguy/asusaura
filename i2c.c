#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

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
