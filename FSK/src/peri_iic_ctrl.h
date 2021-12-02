/*
 * peri_iic_ctrl.h
 *
 *  Created on: 2019. 11. 13.
 *      Author: LK
 */

#ifndef SRC_PERI_IIC_CTRL_H_
#define SRC_PERI_IIC_CTRL_H_

#include "stdio.h"
//#include "xiicps.h"
#include "xiic.h"
#include "xil_exception.h"
#include "xil_printf.h"

#define CAM_IIC_BASE_ADDR 0x40800000
#define TOUCH_IIC_BASE_ADDR 0x40810000
#define ZN220_IIC_SLAVE_ADDR	(0x80>>1)
#define TOUCH_IIC_SLAVE_ADDR	(0xBA>>1)

#define GT9147_ConfigMsgReg 	0x8047
#define GT9147_ProductIDReg     0x8140
#define GT9147_TouchStateReg 	0X814E
#define GT9147_TouchPoint1Reg	0X8150
#define GT9147_TouchPoint2Reg	0X8158
#define GT9147_TouchPoint3Reg	0X8160
#define GT9147_TouchPoint4Reg	0X8168
#define GT9147_TouchPoint5Reg	0X8170

XIic Touch_IicInstance;

typedef struct
{
	uint8_t TouchSta;
	uint16_t x[5];
	uint16_t y[5];

}TouchPointRefTypeDef;

typedef u16 AddressType;

unsigned IIC_WriteByte(UINTPTR IIC_BASE_ADDR, AddressType IIC_ADDR, AddressType SUB_ADDR, u8 *BufferPtr, u16 ByteCount, int addr_size);
unsigned IIC_ReadByte (UINTPTR IIC_BASE_ADDR, AddressType IIC_ADDR, AddressType SUB_ADDR, u8 *BufferPtr, u16 ByteCount, int addr_size);
s32 img_zn220_initialize ();

int Touch_init(uint8_t cmd);

TouchPointRefTypeDef Touch_Read();






#endif /* SRC_PERI_IIC_CTRL_H_ */
