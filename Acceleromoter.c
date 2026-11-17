#include "cybsp.h"
#include "cy_utils.h"
#include "i2c.h"
#include <stdio.h>



int _write(int file, char *ptr, int len)
{
   (void)file;
   for (int i = 0; i < len; i++){
       if (ptr[i] == '\n'){
           XMC_UART_CH_Transmit(CYBSP_DEBUG_UART_HW, '\r');
       }
       XMC_UART_CH_Transmit(CYBSP_DEBUG_UART_HW, ptr[i]);
   }
   return len;
}



#define ACCEL_ADDR   (0b0011110 << 1)   // device's 7-bit I2C address, shifted left once for R/W (ADDRESS OF 1ST ACCELEROMETER)

#define ACCEL_ADDR2  (0b0011101 << 1)  // SA0 = VDD,  sihifted left once for  R/W (ADDRESS OF 2ND ACCELEROMETER) 

#define CTRL1_REG    0x20          // CTRL1 register address
#define CTRL1_VALUE  0x67          // AODR=0110 (100 Hz), BDU=0, Z/Y/X enabled


//Only using 1 accelerometer
//Returns true if the I2C write succeeded (sensor ACK'd) and false if not. Doesn't take addresses since the address, register and value are all fixed by the defines.
bool configure_ctrl1() //Configure the first accelerometer
{
    uint8_t data[2] = { CTRL1_REG, CTRL1_VALUE };
    return i2c_write_bytes(XMC_I2C1_CH0, ACCEL_ADDR, data, 2);
}//configure_ctrl1



//combine_bytes (Not exactly sure what this does - fix later)
int16_t combine_bytes(uint8_t high_byte, uint8_t low_byte) {
    return (int16_t)((high_byte << 8) | low_byte);
}//combine_bytes

//Function to get acceleration and return it
int16_t get_acceleration(void){

    configure_ctrl1();

    //FIRST SENSOR:
    uint8_t data = 0b10101000; // 0x28 + SUB[7] increment bit
    uint8_t read_data[6];
    i2c_write_bytes(XMC_I2C1_CH0, 0x3C, &data, 1);
    i2c_read_bytes(XMC_I2C1_CH0, 0x3D, read_data, 6);

    int16_t x = combine_bytes(read_data[1], read_data[0]);
    int16_t y = combine_bytes(read_data[3], read_data[2]);
    int16_t z = combine_bytes(read_data[5], read_data[4]);

    printf("SENSOR 1: X = %d  G's = %f\n\rY = %d  G's = %f\n\rZ = %d  G's = %f\n\r\n\r\n\r",
        x, x / 16384.0, 
        y, y / 16384.0, 
        z, z / 16384.0
    );

    XMC_Delay(500);

    return combine_bytes(read_data[5], read_data[4]);
}//get_acceleration