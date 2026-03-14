#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

#define BMP280_ADDR 0x76
#define CALIBRATION_DATA 0x88
#define TEMP_DATA 0xFA
#define PRESS_DATA 0xF7
#define CTRL_MEAS 0xF4
#define CONFIG 0xF5
#define RESET 0xE0

#define T1 27504
#define T2 26435
#define T3 50


enum state{
  temp,
  press
};

typedef struct {
  uint32_t temp;
  uint32_t press;
  uint8_t reg_address;
} chip_state_t;

bool on_i2c_connect(void *user_data, uint32_t address, bool read) {
  chip_state_t *chip = (chip_state_t *)user_data;
  if (address == BMP280_ADDR) {
    return true;
  }
  return false;
}

uint8_t on_i2c_read(void *user_data) {
  chip_state_t *chip = (chip_state_t *)user_data;
  static uint8_t base_addr=0xFA;
  static int counter=0;
  int init=0;
 // printf("BASEADDR : %x\n",base_addr);
if(chip->reg_address!=0xD0)
{
  if(chip->reg_address==base_addr)
  {
    ((init+=1)!=0)?counter++:counter;
  }
  else
  {
    counter=0;
    base_addr=chip->reg_address;
  }
}
uint8_t modified_reg_addr= chip->reg_address +counter;

 
    if(modified_reg_addr>0xFC && base_addr==0xFA)
    {
      modified_reg_addr-=3;
    }
  if(modified_reg_addr>0xF9 && base_addr==0xF7)
    {
      modified_reg_addr-=3;
    }
 
  

 // printf("CURRENT ADDR: %x COUNTER : %d  ADDRESS : %x\n",chip->reg_address,counter,modified_reg_addr);
 
  switch (modified_reg_addr) {
    
    case 0xD0: 
    return 0x58;  
    case CALIBRATION_DATA: 
        return 0xB8;        // LSB of T1 (27504, 0x6B00), 0xB8 is lower byte
    case CALIBRATION_DATA + 1:
        return 0x6B;   // MSB of T1 (27504, 0x6B00), 0x6B is upper byte
    case CALIBRATION_DATA + 2:
        return 0xD1;        // LSB of T2 (26435, 0x6733)
    case CALIBRATION_DATA + 3:
        return 0x67;   // MSB of T2 (26435, 0x6733)
    case CALIBRATION_DATA + 4:
        return 0xF0;        // LSB of T3 (-1000, 0xFFFFFC18)
    case CALIBRATION_DATA + 5:
        return 0xFF;   // MSB of T3 (-1000, 0xFFFFFC18)
    case CALIBRATION_DATA + 6:
        return 0xC1;        // LSB of P1 (36477, 0x8F45)
    case CALIBRATION_DATA + 7:
        return 0x8F;   // MSB of P1 (36477, 0x8F45)
    case CALIBRATION_DATA + 8:
        return 0xAB;        // LSB of P2 (-10685, 0xD169)
    case CALIBRATION_DATA + 9:
        return 0xD1;   // MSB of P2 (-10685, 0xD169)
    case CALIBRATION_DATA + 10:
        return 0x0C;        // LSB of P3 (3024, 0x0BDC)
    case CALIBRATION_DATA + 11:
        return 0x0B;   // MSB of P3 (3024, 0x0BDC)
    case CALIBRATION_DATA + 12:
        return 0x11;        // LSB of P4 (2855, 0x0B33)
    case CALIBRATION_DATA + 13:
        return 0x0B;   // MSB of P4 (2855, 0x0B33)
    case CALIBRATION_DATA + 14:
        return 0x8C;        // LSB of P5 (140, 0x8C)
    case CALIBRATION_DATA + 15:
        return 0x00;   // MSB of P5 (140, 0x8C)
    case CALIBRATION_DATA + 16:
        return 0xFF;        // LSB of P6 (-7, 0xFFFD)
    case CALIBRATION_DATA + 17:
        return 0xFF;   // MSB of P6 (-7, 0xFFFD)
    case CALIBRATION_DATA + 18:
        return 0xA0;        // LSB of P7 (15500, 0x3C74)
    case CALIBRATION_DATA + 19:
        return 0x3C;   // MSB of P7 (15500, 0x3C74)
    case CALIBRATION_DATA + 20:
        return 0x28;        // LSB of P8 (-14600, 0xD68C)
    case CALIBRATION_DATA + 21:
        return 0xD6;   // MSB of P8 (-14600, 0xD68C)
    case CALIBRATION_DATA + 22:
        return 0x70;        // LSB of P9 (6000, 0x17F0)
    case CALIBRATION_DATA + 23:
        return 0x17;   // MSB of P9 (6000, 0x17F0)
    case TEMP_DATA:
      return (uint8_t)((uint8_t)(attr_read(chip->temp) >> 16) & 0xFF);
    case TEMP_DATA + 1:
      return (uint8_t)((uint8_t)(attr_read(chip->temp) >> 8) & 0xFF);
    case TEMP_DATA + 2:
      return (uint8_t)((uint8_t)attr_read(chip->temp) & 0xFF);
    case PRESS_DATA:
      return ( attr_read(chip->press) >> 16) & 0xFF;
    case PRESS_DATA + 1:
      return ( attr_read(chip->press) >> 8) & 0xFF;
    case PRESS_DATA + 2:
      return attr_read(chip->press) & 0xFF;
    default:
      return 0xFF;
  }
  //chip->reg_address++;
}

bool on_i2c_write(void *user_data, uint8_t data) {
  chip_state_t *chip = (chip_state_t *)user_data;
 // printf("ADDRESS: %x\n",data);
  chip->reg_address = data;
  return true;
}

void on_i2c_disconnect(void *user_data) {
}

/*void update_temperature(chip_state_t *chip) {
  // Fetch the temperature value from Wokwi control
  chip->temp = attr_read(temp_attr_id);
}

void update_pressure(chip_state_t *chip) {
  // Fetch the pressure value from Wokwi control
  chip->press = attr_read(press_attr_id);
}*/


void chip_init() {
  static chip_state_t chip;
  
  const i2c_config_t i2c_config = {
    .address = BMP280_ADDR,
    .scl = pin_init("SCL", INPUT_PULLUP),
    .sda = pin_init("SDA", INPUT_PULLUP),
    .connect = on_i2c_connect,
    .read = on_i2c_read,
    .write = on_i2c_write,
    .disconnect = on_i2c_disconnect,
    .user_data = &chip,
  };

  i2c_init(&i2c_config);
  chip.reg_address = 0x00;
  printf("BMP280 initialized!\n");
  chip.temp = attr_init("temp",100);

  printf("True");
  chip.press = attr_init("press",20);

  

}

/*void chip_loop() {
  static chip_state_t chip; // Use the global or passed chip state

  printf("Reached chip_loop\n");

  // Update temperature and pressure values
  update_temperature(&chip);
  update_pressure(&chip);

  // Print the updated values
  printf("Updated Temperature: %u\n", chip.temp);
  printf("Updated Pressure: %u\n", chip.press);
}*/

