/*
 * peri_iic_ctrl.c
 *
 *  Created on: 2019. 11. 13.
 *      Author: LK
 */


#include "peri_iic_ctrl.h"
#include "zn220_initialize_list.h"

unsigned IIC_WriteByte(UINTPTR IIC_BASE_ADDR, AddressType IIC_ADDR, AddressType SUB_ADDR, u8 *BufferPtr, u16 ByteCount, int addr_size)
{
	volatile unsigned SentByteCount;
	volatile unsigned AckByteCount;
	u8 WriteBuffer[200];
	int Index;
	u32 CntlReg;

	if (addr_size == 2) {
		WriteBuffer[0] = (u8)(SUB_ADDR >> 8);
		WriteBuffer[1] = (u8)(SUB_ADDR);
	} else if (addr_size == 1) {
		WriteBuffer[0] = (u8)(SUB_ADDR);
		IIC_ADDR |= (128 >> 8) & 0x7;
	}

	for (Index = 0; Index < ByteCount; Index++) {
		WriteBuffer[addr_size + Index] = BufferPtr[Index];
	}


	SentByteCount = XIic_Send(IIC_BASE_ADDR, IIC_ADDR,
				  WriteBuffer, ByteCount + addr_size,
				  XIIC_STOP);


	return SentByteCount - sizeof(SUB_ADDR);
}


unsigned IIC_ReadByte(UINTPTR IIC_BASE_ADDR, AddressType IIC_ADDR, AddressType SUB_ADDR, u8 *BufferPtr, u16 ByteCount, int addr_size)
{
	volatile unsigned ReceivedByteCount;
	u16 StatusReg;
	u32 CntlReg;

	u8 addr_buf[2];

	if (addr_size == 2) {
		addr_buf[0] = (u8)(SUB_ADDR >> 8);
		addr_buf[1] = (u8)(SUB_ADDR);
	} else if (addr_size == 1) {
		addr_buf[0] = (u8)(SUB_ADDR);
	}
	do {
		StatusReg = XIic_ReadReg(IIC_BASE_ADDR, XIIC_SR_REG_OFFSET);
		if(!(StatusReg & XIIC_SR_BUS_BUSY_MASK)) {
			ReceivedByteCount = XIic_Send(IIC_BASE_ADDR,
							IIC_ADDR,
							addr_buf,
							addr_size,
							XIIC_STOP);

			if (ReceivedByteCount != sizeof(SUB_ADDR)) {

				/* Send is aborted so reset Tx FIFO */
				CntlReg = XIic_ReadReg(IIC_BASE_ADDR,
							XIIC_CR_REG_OFFSET);
				XIic_WriteReg(IIC_BASE_ADDR, XIIC_CR_REG_OFFSET,
						CntlReg | XIIC_CR_TX_FIFO_RESET_MASK);
				XIic_WriteReg(IIC_BASE_ADDR,
						XIIC_CR_REG_OFFSET,
						XIIC_CR_ENABLE_DEVICE_MASK);
			}
		}

	} while (ReceivedByteCount != sizeof(SUB_ADDR));

	/*
	 * Read the number of bytes at the specified address from the EEPROM.
	 */
	ReceivedByteCount = XIic_Recv(IIC_BASE_ADDR, IIC_ADDR,
					BufferPtr, ByteCount, XIIC_STOP);

	/*
	 * Return the number of bytes read from the EEPROM.
	 */
	return ReceivedByteCount;
}


s32 img_zn220_initialize ()
{
	s32 Status = 0;
	u32 i = 0;
	u32 d = 0;
	u8 img_iic_value;

	int err_cnt = 0;

	int temp = sizeof(zn220reg);


	// YCbCr 10bit, Full HD
	for (i=0; i < sizeof(zn220reg)/sizeof(zn220reg_info); i++)
	{
      IIC_WriteByte (CAM_IIC_BASE_ADDR, ZN220_IIC_SLAVE_ADDR, zn220reg[i].addr, &(zn220reg[i].initvalue), 1, 1);

      usleep(1000);
	}
	printf ("IIC Initialize done.\r\n");

	return err_cnt;
}



int Touch_IIC_Setup()
{
	u32 Index;
	int Status;
	XIic_Config *ConfigPtr;	/* Pointer to configuration data */

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIic_LookupConfig(XPAR_IIC_1_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XIic_CfgInitialize(&Touch_IicInstance, ConfigPtr,
			ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Set the Slave address.
	 */
	Status = XIic_SetAddress(&Touch_IicInstance, XII_ADDR_TO_SEND_TYPE,
				 TOUCH_IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}




int TouchWriteData(u16 register_addr, u8 * Buffer, u16 ByteCount)
{
	int Status;
   u8 * new_buffer;
   u16 new_addr;

   new_addr = ((register_addr&0xff)<<8)|((register_addr&0xff00)>>8);
   new_buffer = (void *)malloc(ByteCount + 2);

   if(new_buffer == NULL) {
		u32 a;

		xil_printf("fail at write mem allocation\n\r");
		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x20);
		xil_printf("ISR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x104);  // SR
		xil_printf("SR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x100);  // CR
		xil_printf("CR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x120);  // RX_FIFO_PIRQ
		xil_printf("RX_FIFO_PIRQ is %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x118);  // RX_FIFO_OCY
		xil_printf("RX_FIFO_OCY is %d\n\r",a);

      free(new_buffer);
		return XST_FAILURE;
	}

   memcpy(new_buffer,&new_addr, 2);
   memcpy(new_buffer+2,Buffer, ByteCount);

	/*
	 * Set the defaults.
	 */

	Touch_IicInstance.Stats.TxErrors = 0;

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&Touch_IicInstance);
	if (Status != XST_SUCCESS) {
		u32 a;

		xil_printf("fail at write XIic_Start\n\r");
		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x20);
		xil_printf("ISR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x104);  // SR
		xil_printf("SR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x100);  // CR
		xil_printf("CR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x120);  // RX_FIFO_PIRQ
		xil_printf("RX_FIFO_PIRQ is %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x118);  // RX_FIFO_OCY
		xil_printf("RX_FIFO_OCY is %d\n\r",a);

		free(new_buffer);
      return XST_FAILURE;
	}

	/*
	 * Send the Data.
	 */
   if(Buffer == 0)
   {
      Status = XIic_MasterSend(&Touch_IicInstance, &new_addr, 2);
      if (Status != XST_SUCCESS) {
         free(new_buffer);
         return XST_FAILURE;
      }
   }else
   {
      	Status = XIic_MasterSend(&Touch_IicInstance, new_buffer, ByteCount+2);
         if (Status != XST_SUCCESS) {
            free(new_buffer);
            return XST_FAILURE;
         }
   }



	/*
	 * Wait till the transmission is completed.
	 */
	while ((XIic_IsIicBusy(&Touch_IicInstance) == TRUE)) {
		/*
		 * This condition is required to be checked in the case where we
		 * are writing two consecutive buffers of data to the Touch.
		 * The Touch takes about 2 milliseconds time to update the data
		 * internally after a STOP has been sent on the bus.
		 * A NACK will be generated in the case of a second write before
		 * the Touch updates the data internally resulting in a
		 * Transmission Error.
		 */
		if (Touch_IicInstance.Stats.TxErrors != 0) {


			/*
			 * Enable the IIC device.
			 */
			Status = XIic_Start(&Touch_IicInstance);
			if (Status != XST_SUCCESS) {
				free(new_buffer);
            return XST_FAILURE;
			}


			if (!XIic_IsIicBusy(&Touch_IicInstance)) {
				/*
				 * Send the Data.
				 */
				Status = XIic_MasterSend(&Touch_IicInstance,
							 Buffer,
							 ByteCount);
				if (Status == XST_SUCCESS) {
					Touch_IicInstance.Stats.TxErrors = 0;
				}
				else {
				}
			}
		}
	}

	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&Touch_IicInstance);
	if (Status != XST_SUCCESS) {
		free(new_buffer);
      return XST_FAILURE;
	}

	free(new_buffer);
   return XST_SUCCESS;
}



int TouchReadData(u16 register_addr, u8 *BufferPtr, u16 ByteCount)
{
	int Status;
	AddressType Address = register_addr;
	u32 a;

   u16 Addr;
	/*
	 * Set the Defaults.
	 */


	/*
	 * Position the Pointer in Touch.
	 */



    Addr= Address;


	Status = TouchWriteData(Addr,0, sizeof(Address));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the IIC device.
	 */
	Status = XIic_Start(&Touch_IicInstance);
	if (Status != XST_SUCCESS) {
		u32 a;

		xil_printf("fail at read XIic_Start\n\r");
		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x20);
		xil_printf("ISR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x104);  // SR
		xil_printf("SR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x100);  // CR
		xil_printf("CR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x120);  // RX_FIFO_PIRQ
		xil_printf("RX_FIFO_PIRQ is %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x118);  // RX_FIFO_OCY
		xil_printf("RX_FIFO_OCY is %d\n\r",a);


		return XST_FAILURE;
	}

	/*
	 * Receive the Data.
	 */
	XIic_Recv(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR,BufferPtr,ByteCount,XIIC_STOP);

	//Status = XIic_MasterRecv(&Touch_IicInstance, BufferPtr, ByteCount);
	if (Status != XST_SUCCESS) {

		u32 a;

		xil_printf("XIic_MasterRecv\n\r");
		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x20);
		xil_printf("ISR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x104);  // SR
		xil_printf("SR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x100);  // CR
		xil_printf("CR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x120);  // RX_FIFO_PIRQ
		xil_printf("RX_FIFO_PIRQ is %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x118);  // RX_FIFO_OCY
		xil_printf("RX_FIFO_OCY is %d\n\r",a);


		return XST_FAILURE;
	}


	/*
	 * Wait till all the data is received.
	 */
	while ((XIic_IsIicBusy(&Touch_IicInstance) == TRUE)) {
		xil_printf("In HOLD on!!!\n\r");
		u32 a;

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x20);  // ISR
		xil_printf("ISR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x104);  // SR
		xil_printf("SR = %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x100);  // CR
		xil_printf("CR = %d\n\r",a);

//		xil_printf("Try read FIFO...\n\r");
//		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x108);  // Rx_FIFO
//		xil_printf("FIFO is %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x120);  // RX_FIFO_PIRQ
		xil_printf("RX_FIFO_PIRQ is %d\n\r",a);

		a = Xil_In32(TOUCH_IIC_BASE_ADDR + 0x118);  // RX_FIFO_OCY
		xil_printf("RX_FIFO_OCY is %d\n\r",a);
	}


	/*
	 * Stop the IIC device.
	 */
	Status = XIic_Stop(&Touch_IicInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}



int Touch_init(uint8_t cmd)
{
	   uint8_t GT9147_ConfigMSGTBL[186] =
	   {
	      0x41,
	      0x0,
	      0x4,
	      0x58,
	      0x2,
	      0xA,
	      0x3D,
	      0x0,
	      0x1,
	      0x8,
	      0x28,
	      0x8,
	      0x5A,
	      0x46,
	      0x3,
	      0x3,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x10,
	      0x11,
	      0x14,
	      0xF,
	      0x89,
	      0x2A,
	      0x9,
	      0x4B,
	      0x50,
	      0xB5,
	      0x6,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x2,
	      0x1D,
	      0x0,
	      0x1,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x3C,
	      0x78,
	      0x94,
	      0xC5,
	      0x2,
	      0x7,
	      0x0,
	      0x0,
	      0x4,
	      0x90,
	      0x40,
	      0x0,
	      0x81,
	      0x4A,
	      0x0,
	      0x75,
	      0x55,
	      0x0,
	      0x6B,
	      0x61,
	      0x0,
	      0x62,
	      0x70,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x2,
	      0x4,
	      0x6,
	      0x8,
	      0xA,
	      0xC,
	      0x10,
	      0x12,
	      0x14,
	      0xFF,
	      0xFF,
	      0xFF,
	      0xFF,
	      0xFF,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x2,
	      0x4,
	      0x6,
	      0x8,
	      0xA,
	      0xF,
	      0x10,
	      0x12,
	      0x16,
	      0x18,
	      0x1C,
	      0x1D,
	      0x1E,
	      0x1F,
	      0x20,
	      0x21,
	      0x22,
	      0x24,
	      0xFF,
	      0xFF,
	      0xFF,
	      0xFF,
	      0xFF,
	      0xFF,
	      0xFF,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0,
	      0x0
	   };




   uint8_t i;
   GT9147_ConfigMSGTBL[184] = 0;
   uint16_t temp = 0;

   uint8_t tempt = 0x22;

   tempt = 0xAA;
   IIC_WriteByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, 0x8041, &tempt, 1, 2);
   tempt = 0x22;
   IIC_WriteByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, 0x8040, &tempt, 1, 2);
   IIC_ReadByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, GT9147_ConfigMsgReg, &temp, 1, 2);
   tempt = 0x00;

//	if((temp&0xFF) < 0x60)
	{
	   IIC_WriteByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, GT9147_ConfigMsgReg, &tempt, 1, 2);
	   usleep(1000);
      for(i = 0;i < 184;i++)
      {
         GT9147_ConfigMSGTBL[184] += GT9147_ConfigMSGTBL[i];
      }
      GT9147_ConfigMSGTBL[184] = ~GT9147_ConfigMSGTBL[184]  + 1;
      GT9147_ConfigMSGTBL[185] = cmd;

      for(i = 0; i<186; i++)
      {
    	  IIC_WriteByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, GT9147_ConfigMsgReg + i, &GT9147_ConfigMSGTBL[i], 1, 2);
      }


	}
	return XST_SUCCESS;
}


TouchPointRefTypeDef Touch_Read()
{
	uint8_t i;
	uint16_t sta = 0;
    uint8_t dat[4] = {0};

    TouchPointRefTypeDef TPR_Structure = {0,{0},{0}};

   const uint16_t GT9147_TPR_TBL[5] = {GT9147_TouchPoint1Reg,GT9147_TouchPoint2Reg,GT9147_TouchPoint3Reg,GT9147_TouchPoint4Reg,GT9147_TouchPoint5Reg};
	IIC_ReadByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, GT9147_TouchStateReg,&sta,1,2);

	if(sta & 0x80)
	{
		if(sta & 0x0F)
		{
			TPR_Structure.TouchSta = ~(0xFF << (sta & 0x0F));
			for(i = 0;i < 5;i++)
			{
				if (TPR_Structure.TouchSta & (1<<i))
				{
					IIC_ReadByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, GT9147_TPR_TBL[i],dat,4,2);
//					TPR_Structure.x[i] = ((uint16_t)(dat[1]) << 8) + dat[0];
//					TPR_Structure.y[i] = ((uint16_t)(dat[3]) << 8) + dat[2];
					TPR_Structure.x[i] = 800 - ((uint16_t)((uint16_t)(dat[3]) << 8) + dat[2]);
					TPR_Structure.y[i] = (uint16_t)((uint16_t)(dat[1]) << 8) + dat[0];
				}
			}
		}
		sta = 0;
		IIC_WriteByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, GT9147_TouchStateReg,&sta,1,2);
	}
	else
	{
		TPR_Structure.TouchSta &= 0xE0;
	}
	return TPR_Structure;
}



