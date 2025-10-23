#include <stdint.h>
#include "msp.h"
#include "..\inc\Bump.h"
#include "..\inc\Clock.h"
#include "..\inc\SysTick.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Motor.h"
#include"..\inc\PWM.h"
#include"..\inc\Reflectance.h"
#include "..\inc\ADC14.h"
#include "..\inc\IRDistance.h"
#include "..\inc\LPF.h"
#include "..\inc\TimerA0.h"

void TimedPause(uint32_t time){
  Clock_Delay1ms(time);          // 小车停止
  Motor_Stop();
}

uint8_t Data;
int32_t position;
void reflactance(void){

        Data = Reflectance_Read(1000);
        position = Reflectance_Position(Data);

        if((position<=-50)&&(position>=-250))
        {                                             //右转情况1
            Motor_Stop();
            TimedPause(200);
                              Motor_Forward(7000,2000);
                              TimedPause(100);
         }

         else if((position>=50)&&(position<=250)){//左转情况1
                  Motor_Stop();
                  TimedPause(200);
                  Motor_Forward(2000,7000);
                  TimedPause(100);
                      }
         else if(position>250){//左转情况2
             Motor_Stop();
             TimedPause(200);
             Motor_Left(4000,4000);
             TimedPause(300);
         }
         else if(position<-250){//右转情况2
                     Motor_Stop();
                     TimedPause(200);
                     Motor_Right(4000,4000);
                     TimedPause(300);
                 }
              else {//不偏离规矩则前进
                  Motor_Forward(4000,4000);

              }

}



uint32_t Right,Center,Left; // distance in mm

void IRrun(uint16_t nr,uint16_t nc,uint16_t nl )
{
    Right = RightConvert(nr);
          Center = CenterConvert(nc);
          Left = LeftConvert(nl);
            if(Center <= 100){
              TimedPause(100);
              Motor_Backward(4000, 4000);
                  TimedPause(200);
                 Motor_Right(5000,4000);
                  TimedPause(700);// delay
                }
            else if(Left <= 100){
                 Motor_Right(4000, 4000);
                TimedPause(500);          // delay
               }
          else if(Right <= 100){

                  Motor_Left(4000, 4000);

                 TimedPause(500);          // delay
               }
            else{
                Motor_Forward(4000,4000);
            }

}
void bumprun(void){

        uint8_t numflag = 0;
        numflag = Bump_Read();
        if(numflag == 1){
            TimedPause(200);
            Motor_Backward(4000,4000);
            TimedPause(500);
            Motor_Left(4000,6000);
            TimedPause(500);//delay
        }
        else if(numflag == 3){
            TimedPause(200);
            Motor_Backward(4000,4000);
            TimedPause(500);
            Motor_Right(4000,6000);
            TimedPause(500);// delay
        }
        else if(numflag == 2){
            TimedPause(200);
            Motor_Backward(4000,4000);
            TimedPause(500);// delay
            Motor_Right(4000,6000);
            TimedPause(700);//delay
        }
        else if(numflag == 0)
             Motor_Forward(4000,4000);

}
volatile uint32_t nr,nc,nl; // filtered ADC sample
uint32_t raw17,raw12,raw16;
  void loop(void)//采样2ms一次，32ms一个周期
  {
      int i;
      int z=16;
      uint32_t sum1=0,sum2=0,sum3=0;
      for(i=0;i<z;i++)
      {
          ADC_In17_12_16(&raw17,&raw12,&raw16);
          sum1=raw17+sum1;
          sum2=raw12+sum2;
          sum3=raw16+sum3;
          Clock_Delay1ms(2);
      }
       nr=sum1/z;
       nc=sum2/z;
       nl=sum3/z;
  }

  int main(void){
      uint8_t t;
       Clock_Init48MHz();
       LaunchPad_Init();
       ADC0_InitSWTriggerCh17_12_16();
         Bump_Init();      // bump switches

       Motor_Init();     // your function
       PWM_Init34(15000, 10000, 10000);
       Reflectance_Init();
       while(LaunchPad_Input()==0);  // wait for touch
       t = LaunchPad_Input();
       switch(t){

 case 0x01:
  while(1)
  {                 // switches are negative logic on P1.1 and P1.4
      loop();
      IRrun(nr,nc,nl);
     bumprun();
}
       break;
      case 0x02:
          while(1){
            reflactance();
            bumprun();
}
            break;
          }
  }
