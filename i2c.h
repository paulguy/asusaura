int open_i2c_dev(const char *devpath);
int set_i2c_slave(int i2cdev, unsigned char addr);
int i2c_probe(int i2cdev, unsigned char addr);
