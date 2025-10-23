#include "msp.h"
#include <stdio.h>
#include "../inc/Clock.h"
#include "../inc/UART0.h"
#include "../inc/Reflectance.h"
#include "../inc/PWM.h"
#include "../inc/Motor.h"
#include "../inc/BumpInt.h"
#include "../inc/LaunchPad.h"
#include "../inc/ADC14.h"


uint32_t CollisionData, CollisionFlag;  // mailbox
uint32_t ch1,ch2,ch3;
int32_t z=1195172;
int32_t e=1058;
uint8_t data,cnt_collision=0;

void HandleCollision(uint8_t bumpSensor){
   Motor_Stop();
   CollisionData = bumpSensor;
   CollisionFlag = 1;
   if(CollisionData == 0x1f || CollisionData == 0x2f || CollisionData == 0x37)
   {
       Motor_Backward(5500,5500);
       Clock_Delay1ms(210);
       Motor_Right(5000,5000);
       Clock_Delay1ms(270);
       Motor_Stop();

   }
   else if(CollisionData == 0x3b || CollisionData == 0x3d || CollisionData == 0x3e)
   {
       Motor_Backward(5500,5500);
       Clock_Delay1ms(210);
       Motor_Left(5000,5000);
       Clock_Delay1ms(270);
       Motor_Stop();
   }

}


void main(void)
{

    uint32_t distance;
    uint32_t distance2;
    uint32_t distance3;
   UART0_Initprintf();
    CollisionFlag=0;

    Clock_Init48MHz();

    ADC0_InitSWTriggerCh17_12_16();
    LaunchPad_Init();
    Motor_Init();
    BumpInt_Init(&HandleCollision);
    LaunchPad_LED(0);
    while(1){

        ADC_In17_12_16(&ch1,&ch2,&ch3);
        distance=z/(ch2-e);
        distance2=z/(ch1-e);
        distance3=z/(ch3-e);
        if(distance!=0&&distance<150)
        {
            Motor_Backward(5500,5500);
            Clock_Delay1ms(400);
            Motor_Right(5000,5000);
            Clock_Delay1ms(1000);
            Motor_Stop();
        }
        if(distance2!=0&&distance2<50)
        {
            Motor_Backward(5500,5500);
            Clock_Delay1ms(400);
            Motor_Right(5000,5000);
            Clock_Delay1ms(1000);
            Motor_Stop();
        }
        if(distance3!=0&&distance3<50)
        {
            Motor_Backward(5500,5500);
            Clock_Delay1ms(400);
            Motor_Left(5000,5000);
            Clock_Delay1ms(1000);
            Motor_Stop();
        }
        else
            Motor_Forward(6000,6000);
     }
}


