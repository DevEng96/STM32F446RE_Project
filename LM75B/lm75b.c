/*
 *
 *
 * author: Leoni Etter
 *
 *
 * */
#include "lm75b.h"
#include "i2c.h"
#include "stdio.h"
//#define LM75B_ADR 0x90

// assume LM75B_7BIT = 0b1001ABC  (A,B,C = your A2..A0 pin levels)
// For STM32 HAL, pass 8-bit address: (LM75B_7BIT << 1)
#define LM75B_I2C_ADDR   0x90
#define LM75B_PTR_TEMP   0x00

float readTemp(void) {

	uint8_t tBuf[2];
//	if (HAL_I2C_Master_Receive(&hi2c1, LM75B_ADR, tBuf, sizeof(tBuf),
//			HAL_MAX_DELAY) == HAL_OK) {
	if (HAL_I2C_Mem_Read(&hi2c1, LM75B_I2C_ADDR,
	LM75B_PTR_TEMP, I2C_MEMADD_SIZE_8BIT, tBuf, 2, HAL_MAX_DELAY) != HAL_OK) {
		puts("LM75B: I2C read failed");
		return -1;
	}
	int16_t rawTemp = (int16_t) ((tBuf[0] << 8) | tBuf[1]); // D10 ends up in bit15

	if ((rawTemp & 0b1000000000000000) == 0b1000000000000000) {/* if D10==1 on bit 15, then Negative Temperature*/
		rawTemp = (rawTemp >> 5) & (0b0000011111111111);
		/*Twos Complement*/
//		rawTemp = (~rawTemp)+1;
		rawTemp = (~rawTemp & 0x03FF) + 1;
		return (float) -rawTemp * 0.125f;
	} else { /*Positive Temperature*/
		//int16_t rawTemp = (int16_t) ((tBuf[0] << 8) | tBuf[1]); // D10 ends up in bit15
		//rawTemp >>= 5;
		rawTemp = (rawTemp >> 5) & (0b0000011111111111);
		return (float) rawTemp * 0.125f;
	}

	/*
	 POS:
	 If the Temp data MSByte bit D10=0, then the temperature is positive and Tempvalue
	 (C)=+(Temp data)*0.125C.
	 NEG:
	 If the Temp data MSByte bit D10=1, then the temperature is negative and
	 Tempvalue(C)=-(two’s complement of Temp data)*0.125C.
	 */

}
