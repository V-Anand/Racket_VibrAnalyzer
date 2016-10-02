#ifndef _HEXIWEAR_FXOS8700CQ_H_
#define _HEXIWEAR_FXOS8700CQ_H_

#include "mbed.h"

#define I2C_SLAVE_ADDR1  (0x1E<<1)
#define I2C_SUCCESS  0x00
#define I2C_ERROR    0xff

#define UINT14_MAX 16383

#define FXOS8700CQ_STATUS  0x00
#define FXOS8700CQ_OUT_X_MSB  0x01
#define FXOS8700CQ_WHOAMI  0x0D

#define FXOS8700CQ_CTRL_REG1  0x2A
#define FXOS8700CQ_CTRL_REG2  0x2B
#define FXOS8700CQ_CTRL_REG3  0x2C
#define FXOS8700CQ_CTRL_REG4  0x2D
#define FXOS8700CQ_CTRL_REG5  0x2E

#define FXOS8700CQ_TRANSIENT_CFG  0x1D
#define FXOS8700CQ_TRANSIENT_SRC  0x1E
#define FXOS8700CQ_TRANSIENT_THS  0x1F
#define FXOS8700CQ_TRANSIENT_CNT  0x20

#define FXOS8700CQ_M_THS_COUNT  0x5A
#define FXOS8700CQ_M_CTRL_REG1  0x5B
#define FXOS8700CQ_M_CTRL_REG2  0x5C

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} SRAWDATA;

class FXOS8700CQ
{
    I2C i2cAcc;

    void write_regs(char* d, uint8_t len)
    {
       i2cAcc.write(I2C_SLAVE_ADDR1, d, 2);
    }
    void read_regs(uint8_t reg_addr, char* data, int len)
    {
       char t[1] = {reg_addr};
       i2cAcc.write(I2C_SLAVE_ADDR1, t, 1, true);
       i2cAcc.read(I2C_SLAVE_ADDR1, data, len);
    }

    void soft_reset()
    {
       /* soft reset */
       char d[2] = {FXOS8700CQ_CTRL_REG2, 0x40};
       write_regs(d, 2);
       wait_ms(100);
    }
    void set_trans_ths()
    {
       /* TRANSIENT_THS: debounce = clear when cond not true; resolution = 63mg x 5*/
       char d[2] = {FXOS8700CQ_TRANSIENT_THS, 0x85};
       write_regs(d, 2);
    }
    void set_trans_count()
    {
       /* TRANSIENT_COUNT: min num of debounce counts = 80ms*/
       char d[2] = {FXOS8700CQ_TRANSIENT_CNT, 0x02};
       write_regs(d, 2);
    }
    void init_tran_msb()
    {
       /* A_TRAN_INIT_MSB: init ref to 0g for all axes */
       char d1[2] = {0x79, 0x00};
       write_regs(d1, 2);

       char d2[2] = {0x7A, 0x00};
       write_regs(d2, 2);

       char d3[2] = {0x7B, 0x00};
       write_regs(d3, 2);

       char d4[2] = {0x7C, 0x00};
       write_regs(d4, 2);
    }
    void set_trans_cfg()
    {
       /* TRANSIENT_CFG: evt latch, no Z-axis, Y-axis, X-axis, HPF */
       char d[2] = {FXOS8700CQ_TRANSIENT_CFG, 0x16};
       write_regs(d, 2);
    }
    void enable_int()
    {
       /* Enable interrupts using CTRL_REG4 */
       char d[2] = {FXOS8700CQ_CTRL_REG4, 0x20};
       write_regs(d, 2);
    }
    void route_int()
    {
       /* Route interrupts to INT1 using CTRL_REG5 */
       char d[2] = {FXOS8700CQ_CTRL_REG5, 0x20};
       write_regs(d, 2);
    }
    void set_hybrid()
    {
       /* Setup device for hybrid mode, enable hybrid mode, auto-inc, ODR=50Hz OSR=32 */
       char d1[2] = {FXOS8700CQ_M_CTRL_REG1, 0x1F};
       write_regs(d1, 2);

       char d2[2] = {FXOS8700CQ_M_CTRL_REG2, 0x20};
       write_regs(d2, 2);

       char d3[2] = {FXOS8700CQ_CTRL_REG1, 0x19};
       write_regs(d3, 2);
    }

public:

    FXOS8700CQ(PinName sda, PinName scl)
      : i2cAcc(sda, scl)
    {
    }

    /* AN4461 */
    void enable_trans_accel()
    {
       soft_reset();
       set_trans_ths();
       set_trans_count();
       init_tran_msb();
       set_trans_cfg();
       enable_int();
       route_int();
       set_hybrid();
    }
    int read_accel(SRAWDATA* accelData)
    {
        uint8_t d[1];
        read_regs(FXOS8700CQ_TRANSIENT_SRC, (char*)d, 1);
        if (0x40 == (d[0] & 0x40)) {
          uint8_t acc[6];
          read_regs(FXOS8700CQ_OUT_X_MSB, (char*)acc, 6);
          accelData->x = ((acc[0]<<8)|(acc[1]))>>2; 
          accelData->y = ((acc[2]<<8)|(acc[3]))>>2; 
          accelData->z = ((acc[4]<<8)|(acc[5]))>>2; 
          return I2C_SUCCESS;
        }
        return I2C_ERROR;
    }
    void display_status(Serial* pc)
    {
        pc->printf( "\r\n\nFXOS8700CQ Who Am I= %X Status = %X\r\n", 
            who_am_i(),
            status());
    }
    uint8_t status()
    {
        return 0;
    }
    uint8_t who_am_i()
    {
        return 0;
    }
};

#endif /* _HEXIWEAR_FXOS8700CQ_H_ */
