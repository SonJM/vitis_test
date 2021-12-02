#include <stdio.h>

#include "xil_printf.h"
#include "xil_io.h"
#include "sleep.h"
#include "peri_iic_ctrl.h"
#include "xparameters.h"
#include <math.h>


#define  BASE_ADDR                  0x44A00000

#define  DOUBLE_BUFFER_ADDR_1       (UINTPTR)((BASE_ADDR+0x04  ))
#define  DOUBLE_BUFFER_ADDR_2       (UINTPTR)((BASE_ADDR+0x08  ))
#define  OVR1              	      (UINTPTR)((BASE_ADDR+0x0c  ))
#define  OVR2				            (UINTPTR)((BASE_ADDR+0x10  ))
#define  STATUS				         (UINTPTR)((BASE_ADDR+28    ))
#define  COORD 					      (UINTPTR)((BASE_ADDR       ))
#define  COLOR_KEY 		            (UINTPTR)((BASE_ADDR+(4*20)))
#define  PERI_SETUP                 (UINTPTR)((BASE_ADDR+0x14  ))


#define  CAM_rst_seq_done           0
#define  odd_even_write             1
#define  HSYNC_lock                 2
#define  Frame_lock                 3
#define  IDELAY_RDY       		      4


#define  DDR_CAL                    31
#define  CAM_rst_req                14
#define  overlay_enable             13
#define  LCD_en                     11
#define  HDMI_en                    10
#define  CAM_en                     9
#define  Touch_rstb                 8
#define  LCD_rstb                   7
#define  LCD_mode                   6
#define  LCD_shlr                   5
#define  LCD_updn                   4
#define  LCD_dith                   3
#define  LCD_W_LED_en               2
#define  LCD_W_LED_ctrl             1
#define  LCD_DRV_ctrl               0
#define  Touch_intr_cont            0x44A10000

typedef struct _TPOINT{
   int x;
   int y;
}TPOINT;

void Bresenham(UINTPTR address, int x1, int y1, int x2, int y2);
void Midpoint(UINTPTR address, int x1, int y1, int x2, int y2);
double TP_Length(TPOINT TP1, TPOINT TP2);

int chk_status(UINTPTR addr, int bit_shift);

void bit_on(UINTPTR addr, int bit_shift);

void bit_off(UINTPTR addr, int bit_shift);

void draw_line_a(UINTPTR address, int x, int y, int r);
void draw_line_b(UINTPTR address, int x, int y, int r);
int main()
{
   printf("Boot..\n");
   for(volatile int a = 0; a<1000; a++)
	   usleep(1000);

	Xil_Out32(DOUBLE_BUFFER_ADDR_1, (uint)0x80000000);
	Xil_Out32(DOUBLE_BUFFER_ADDR_2, (uint)0x80400000);
  // Set camera double buffer address


	Xil_Out32(Touch_intr_cont + 0x00, 0x0);
	Xil_Out32(Touch_intr_cont + 0x08, 0x0);
	Xil_Out32(Touch_intr_cont + 0x0C, 0x2);


	Xil_Out32(COORD, (uint)0<<12 | (uint)0);
  // Set frame coordinate

	while(!chk_status(STATUS, DDR_CAL)) // wait for DDR cali
	{
		printf("Wait DDR_CAL\n");
	}


	Xil_Out32(OVR1, (uint)0x80C00000);
	Xil_Out32(OVR2, (uint)0x81000000);
  // Set camera double buffer address
	memset(0x80C00000, 0x0, 4147200);
	memset(0x81000000, 0x0, 4147200);
	// clear overlay buffer
	memset(0x80000000, 0x0, 4147200);
	memset(0x80400000, 0x0, 4147200);
	// clear camera buffer
	Xil_Out32(COLOR_KEY, (uint)0x00000000);
	// Set color key value


	Xil_Out32(PERI_SETUP, 0x0000); // all reset state
	printf("Allreset\n");




   //bit_off(PERI_SETUP, LCD_mode      ); // sync mode
   bit_on(PERI_SETUP, LCD_mode      ); // DE mode

   bit_on(PERI_SETUP, LCD_dith      );
   bit_on(PERI_SETUP, LCD_W_LED_en  );
   bit_on(PERI_SETUP, LCD_W_LED_ctrl);
   bit_on(PERI_SETUP, LCD_DRV_ctrl  );
   bit_on(PERI_SETUP, LCD_shlr      );
   bit_on(PERI_SETUP, overlay_enable);

	printf("LCD_MODE_SETTING\n");

   bit_on(PERI_SETUP, CAM_rst_req   );
	printf("CAM reset request\n");


   bit_on(PERI_SETUP, LCD_rstb       );
   bit_off(PERI_SETUP, CAM_rst_req   );
	printf("LCD, Camera reset escape\n");

   for(volatile int a = 0; a<1000; a++)
      usleep(1000);


	while(!chk_status(STATUS, CAM_rst_seq_done)) // wait for cam reset
	{
		printf("Wait cam rst\n");
	}

	img_zn220_initialize();
	printf("cam setting done\n");

   for(volatile int a = 0; a<1000; a++)
      usleep(1000);

   bit_on(PERI_SETUP, CAM_en    );
   bit_on(PERI_SETUP, Touch_rstb);


   bit_on(PERI_SETUP, LCD_en);
   bit_off(PERI_SETUP, LCD_W_LED_en);


//   {
//	   u8 temp;
//	   for(int i = 0; i<186; i++)
//	   {
//	 	  IIC_ReadByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, GT9147_ConfigMsgReg + i, &temp, 1, 2);
//		   printf("%d\n",temp);
//	   }
//   }

//   printf("=====================\n");
//   int Status;
//   Status = Touch_init(0x01);
//   if (Status != XST_SUCCESS) {
//      return XST_FAILURE;
//   }
//
//   {
//	   u8 temp;
//	   for(int i = 0; i<186; i++)
//	   {
//	 	  IIC_ReadByte(TOUCH_IIC_BASE_ADDR, TOUCH_IIC_SLAVE_ADDR, GT9147_ConfigMsgReg + i, &temp, 1, 2);
//		   printf("%d\n",temp);
//	   }
//   }


    while(1)
    {
    	volatile u32 a;
    	char c;

//      if(chk_status(STATUS, odd_even_write))
//      {
//         printf("Even frame is in writting...\n");
//      }
//      else
//      {
//         printf("Odd frame is in writting...\n");
//      }
      if(!chk_status(STATUS, HSYNC_lock    ))
      {
         printf("HSYNC lock is break!!!\n");
      }
      if(!chk_status(STATUS, Frame_lock    ))
      {
         printf("Frame lock is break!!!\n");
      }
      TouchPointRefTypeDef TPR_Structure;
      TPR_Structure = Touch_Read();
      if(TPR_Structure.TouchSta != 0)
      {
          printf("STA = %d\n\r",TPR_Structure.TouchSta);

          for (int i = 0; i < 5 ; i++)
          {
//             printf("[%d] x = %d Y = %d\n\r",i, TPR_Structure.x[i],TPR_Structure.y[i]);
        	  printf("[%d] x = %d Y = %d\n\r",i, (int)((double)TPR_Structure.y[0]/(double)1011*(double)800), (int)(480 - (double)(TPR_Structure.x[0]-200)/(double)580*(double)480));
          }
      }
      if(TPR_Structure.TouchSta == 1)
      {
    	  //draw_line(0x80C00000,(double)TPR_Structure.x[0]/(double)660*(double)800, 480 - (TPR_Structure.y[0] - 150), 5);
    	  //draw_line(0x81000000,(double)TPR_Structure.x[0]/(double)660*(double)800, 480 - (TPR_Structure.y[0] - 150), 5);

    	  draw_line_a(0x80C00000,(int)((double)TPR_Structure.y[0]/(double)1011*(double)800), (int)(480 - (double)(TPR_Structure.x[0]-200)/(double)580*(double)480),  10);
    	  draw_line_b(0x81000000,(int)((double)TPR_Structure.y[0]/(double)1011*(double)800), (int)(480 - (double)(TPR_Structure.x[0]-200)/(double)580*(double)480),  10);
      }else if(TPR_Structure.TouchSta == 0b11)
      {
    		// clear overlay buffer
    		memset(0x80C00000, 0x0, 4147200);
    		memset(0x81000000, 0x0, 4147200);
      }

    }

    return 0;
}

int chk_status(UINTPTR addr, int bit_shift)
{
   u32 status = Xil_In32((UINTPTR)addr);
   if(status & ((u32)1 << bit_shift))
   {
      return 1;
   }
   else
   {
      return 0;
   }
}

void bit_on(UINTPTR addr, int bit_shift)
{
   u32 value = *(u32 *)addr;
   value = value | (1<<bit_shift);
   *(u32 *)addr = value;
}

void bit_off(UINTPTR addr, int bit_shift)
{
   u32 value = *(u32 *)addr;
   value = value & ~((u32)1<<bit_shift);
   *(u32 *)addr = value;
}
int point_to_address(UINTPTR base, int cols, int rows, int px, int py, int pixel_size)
{
	return base + ((cols*py) + px) * pixel_size;
}

void draw_line_a(UINTPTR address, int x, int y, int r)
{
	int fx;
	int fy;
   
   static TPOINT last_valid_tp;
   TPOINT current_point;
   current_point.x = x;
   current_point.y = y;

   if((TP_Length(last_valid_tp, current_point) > 100 )|| (last_valid_tp.x == 0 && last_valid_tp.y==0))
   {
      Xil_Out16(point_to_address(address, 1920, 1080, x, y, 2), 0xFFFF);
      last_valid_tp = current_point;
   }
   else
   {
	   //Bresenham(address, last_valid_tp.x, last_valid_tp.y, current_point.x, current_point.y);
	   Midpoint(address, last_valid_tp.x, last_valid_tp.y, current_point.x, current_point.y);
	  last_valid_tp = current_point;
   }
//	for(fx = x-r;fx<x+r;fx++)
//	{
//		if(fx < 0)
//			fx = 0;
//		for(fy = y-r;fy<y+r;fy++)
//		{
//			if(fy < 0)
//				fy = 0;
//			Xil_Out32(point_to_address(address, 1920, 1080, fx, fy, 2), 0xFFFFFFFF);
//		}
//	}
}


void draw_line_b(UINTPTR address, int x, int y, int r)
{
	int fx;
	int fy;

   static TPOINT last_valid_tp;
   TPOINT current_point;
   current_point.x = x;
   current_point.y = y;

   if((TP_Length(last_valid_tp, current_point) > 100) || (last_valid_tp.x == 0 && last_valid_tp.y==0))
   {
      Xil_Out16(point_to_address(address, 1920, 1080, x, y, 2), 0xFFFF);
      last_valid_tp = current_point;
   }
   else
   {
	   //Bresenham(address, last_valid_tp.x, last_valid_tp.y, current_point.x, current_point.y);
	   Midpoint(address, last_valid_tp.x, last_valid_tp.y, current_point.x, current_point.y);
	  last_valid_tp = current_point;
   }
//	for(fx = x-r;fx<x+r;fx++)
//	{
//		if(fx < 0)
//			fx = 0;
//		for(fy = y-r;fy<y+r;fy++)
//		{
//			if(fy < 0)
//				fy = 0;
//			Xil_Out32(point_to_address(address, 1920, 1080, fx, fy, 2), 0xFFFFFFFF);
//		}
//	}
}

void Bresenham(UINTPTR address, int x1, int y1, int x2, int y2)
{
   int dx, dy, incrE, incrNE, D, x, y;
   dx = x2 - x1;
   dy = y2 - y1;

   D = 2*dy - dx;
   incrE = 2*dy;
   incrNE = 2*dy - 2*dx;

   x = x1;
   y = y1;
   Xil_Out16(point_to_address(address, 1920, 1080, x, y, 2), 0xFFFF);
   while (x < x2)
   {
      if (D < 0)
      {
         D += incrE;
         x++;
      }
      else
      {
         D += incrNE;
         x++; y++;
      }
      Xil_Out16(point_to_address(address, 1920, 1080, x, y, 2), 0xFFFF);
   }
}

void Midpoint(UINTPTR address, int x1, int y1, int x2, int y2)
{
	TPOINT a;
	TPOINT b;
	a.x = x1;
	a.y = y1;
	b.x = x2;
	b.y = y2;



	Xil_Out16(point_to_address(address, 1920, 1080, (x1 + x2) / 2,  (y1 + y2) / 2,2), 0xFFFF);
	if(TP_Length(a,b)<1.5)
		return;
	Midpoint(address,(x1 + x2) / 2,  (y1 + y2) / 2, x2, y2);
	Midpoint(address, x1, y1, (x1 + x2) / 2,  (y1 + y2) / 2);
}




double TP_Length(TPOINT TP1, TPOINT TP2)
{
   double x = (TP1.x - TP2.x) * (TP1.x - TP2.x);
   double y = (TP1.y - TP2.y) * (TP1.y - TP2.y);
   return sqrt(x+y);
}
