
#define ADXL345_ADDR         0x53
#define ADXL345_DEVID        0x00
#define ADXL345_POWER_CTL    0x2D
#define ADXL345_DATA_FORMAT  0x31
#define ADXL345_DATAX0       0x32
#define ADXL345_DATAY0       0x34
#define ADXL345_DATAZ0       0x36
#define ADXL345_BW_RATE      0x2C
#define ADXL345_FIFO_CTL     0x38


unsigned char i2c_read_byte(unsigned char dev_addr, unsigned char reg_addr);
void i2c_write_byte(unsigned char dev_addr, unsigned char reg_addr, unsigned char data);

struct accel_data {
    short x;
    short y;
    short z;
};

void adxl345_init(void) {
    unsigned char devid = i2c_read_byte(ADXL345_ADDR, ADXL345_DEVID);
    i2c_write_byte(ADXL345_ADDR, ADXL345_DATA_FORMAT, 0x01);
    i2c_write_byte(ADXL345_ADDR, ADXL345_BW_RATE, 0x0A);
    i2c_write_byte(ADXL345_ADDR, ADXL345_FIFO_CTL, 0x80);
    i2c_write_byte(ADXL345_ADDR, ADXL345_POWER_CTL, 0x08);

    for(volatile int i = 0; i < 1000; i++);
}

void adxl345_read(struct accel_data *data) {
    unsigned char buffer[6];


    for(int i = 0; i < 6; i++) {
        buffer[i] = i2c_read_byte(ADXL345_ADDR, ADXL345_DATAX0 + i);
    }


    data->x = (short)((buffer[1] << 8) | buffer[0]);
    data->y = (short)((buffer[3] << 8) | buffer[2]);
    data->z = (short)((buffer[5] << 8) | buffer[3]);
}


void adxl345_to_g(struct accel_data *raw, float *g) {
    const float scale = 0.0625f;

    g[0] = raw->x * scale;
    g[1] = raw->y * scale;
    g[2] = raw->z * scale;
}


void adxl345_calibrate(struct accel_data *offset) {
    struct accel_data sum = {0, 0, 0};
    struct accel_data sample;
    int samples = 100;

    for(int i = 0; i < samples; i++) {
        adxl345_read(&sample);
        sum.x += sample.x;
        sum.y += sample.y;
        sum.z += sample.z;

        for(volatile int j = 0; j < 100; j++);
    }

    offset->x = sum.x / samples;
    offset->y = sum.y / samples;
    offset->z = (sum.z / samples) - 256;
}

void adxl345_apply_calibration(struct accel_data *data, struct accel_data *offset) {
    data->x -= offset->x;
    data->y -= offset->y;
    data->z -= offset->z;
}

int adxl345_detect_freefall(struct accel_data *data, short threshold) {
    short magnitude = data->x * data->x +
                     data->y * data->y +
                     data->z * data->z;

    return (magnitude < threshold * threshold);
}

/*
void example_usage(void) {
    struct accel_data raw, offset;
    float g[3];

    adxl345_init();
    adxl345_calibrate(&offset);


    while(1) {
        adxl345_read(&raw);
        adxl345_apply_calibration(&raw, &offset);
        adxl345_to_g(&raw, g);

        if(adxl345_detect_freefall(&raw, 50)) {
        }
        for(volatile int i = 0; i < 10000; i++);
    }
}

*/
