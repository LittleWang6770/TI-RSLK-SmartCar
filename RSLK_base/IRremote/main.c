#include <stdint.h>
#include <stdio.h>
#include "msp.h"
//#include "..\inc\Clock.h"
//#include "..\inc\UART0.h"
#include "Clock.h"
#include "UART0.h"
#include "Motor.h"
#include "TA2InputCapture.h"
#include <string.h>
#include "CortexM.h"
#include "PWM.h"
#define P2_0 (*((volatile uint8_t *)(0x42098060)))
#define P1_0 (*((volatile uint8_t *)(0x42098040)))
char buf[2000]={0};

int i=0;
int t,z;
char p=0;
int n=7;
char u=0,v=0;
uint16_t Count0;
uint32_t Sum0;
uint16_t Period0;              // (1/SMCLK) units = 83.3 ns units
uint16_t First0; // Timer A3 first edge, P10.4
int Done0; // set each rising
uint32_t LeftDuty=12000,RightDuty=12000,DutyBuffer[500];  // 0 to 10000
uint32_t Time;// in 0.01 sec
int e=0;
int o=0;
int tbuf[8]={0};

void IR (void){//¶¯×÷º¯Êý
     while(1){
       p=p+(buf[o-1]<<n);
       o--;
       n--;
       if(n==-1){
       break;
       }
}
   n=7;
   printf("%x\n",p);
   if(p==0x45){
   memset(tbuf,0,32);
   tbuf[0]=1;
   Motor_Left(12200,12000);
   Clock_Delay1ms(320);
   Motor_Forward(LeftDuty,RightDuty);
   }
   if(p==0x46){
   memset(tbuf,0,32);
   tbuf[1]=1;
   Motor_Forward(LeftDuty,RightDuty);
   }
   if(p==0x47){
   memset(tbuf,0,32);
   tbuf[2]=1;
   Motor_Right(12200,12000);
   Clock_Delay1ms(320);
  Motor_Forward(LeftDuty,RightDuty);
   }
   if(p==0x44){
   memset(tbuf,0,32);
   tbuf[5]=1;
  Motor_Backward(LeftDuty,RightDuty);
   Clock_Delay1ms(150);
   Motor_Stop();
   }
   if(p==0x40){
   memset(tbuf,0,32);
   tbuf[6]=1;
   Motor_Forward(LeftDuty,RightDuty);
   Clock_Delay1ms(150);
   Motor_Stop();
   }
   if(p==0x43){
   Motor_Stop();
   memset(tbuf,0,32);
   }
   if(p==0x07){
   LeftDuty=LeftDuty-500;
   RightDuty=RightDuty-500;
      if(LeftDuty<=0||RightDuty<=0){
      LeftDuty=0;
      RightDuty=0;
      }
      if(tbuf[0]==1||tbuf[1]==1||tbuf[2]==1){
      Motor_Forward(LeftDuty,RightDuty);
      }
      if(tbuf[3]==1){
      Motor_Right(LeftDuty,RightDuty);
      }
      if(tbuf[4]==1){
      Motor_Backward(LeftDuty,RightDuty);
      }
      if(tbuf[5]==1||tbuf[6]==1||tbuf[7]==1){
      Motor_Stop();
      }
    }
   if(p==0x15){
    LeftDuty=LeftDuty+500;
    RightDuty=RightDuty+500;
     if(LeftDuty>=15000||RightDuty>=15000){
     LeftDuty=15000;
     RightDuty=15000;
     }
     if(tbuf[0]==1||tbuf[1]==1||tbuf[2]==1){
    Motor_Forward(LeftDuty,RightDuty);
     }
     if(tbuf[3]==1){
     Motor_Right(LeftDuty,RightDuty);
     }
     if(tbuf[4]==1){
    Motor_Backward(LeftDuty,RightDuty);
     }
     if(tbuf[5]==1||tbuf[6]==1){
     Motor_Stop();
     }
   }
   if(p==0x09){
   memset(tbuf,0,32);
   tbuf[3]=1;
  Motor_Right(LeftDuty,RightDuty);
    }
   if(p==0x16){
   memset(tbuf,0,32);
   tbuf[4]=1;
   Motor_Backward(LeftDuty,RightDuty);
   }
   if(p==0x19){
    LeftDuty=LeftDuty+100;
    RightDuty=RightDuty+100;
       if(LeftDuty>=15000||RightDuty>=15000){
       LeftDuty=15000;
       RightDuty=15000;
       }
       if(tbuf[0]==1||tbuf[1]==1||tbuf[2]==1){
       Motor_Forward(LeftDuty,RightDuty);
       }
       if(tbuf[3]==1){
       Motor_Right(LeftDuty,RightDuty);
       }
       if(tbuf[4]==1){
      Motor_Backward(LeftDuty,RightDuty);
       }
       if(tbuf[5]==1||tbuf[6]==1||tbuf[7]==1){
       Motor_Stop();
       }
     }
   if(p==0x0d){
     LeftDuty=LeftDuty+200;
     RightDuty=RightDuty+200;
       if(LeftDuty>=15000||RightDuty>=15000){
       LeftDuty=15000;
       RightDuty=15000;
       }
       if(tbuf[0]==1||tbuf[1]==1||tbuf[2]==1){
      Motor_Forward(LeftDuty,RightDuty);
       }
      if(tbuf[3]==1){
     Motor_Right(LeftDuty,RightDuty);
      }
      if(tbuf[4]==1){
      Motor_Backward(LeftDuty,RightDuty);
      }
      if(tbuf[5]==1||tbuf[6]==1||tbuf[7]==1){
      Motor_Stop();
      }
    }
   p=0;
 }
void PeriodMeasure0(uint16_t time){
  P2_0 = P2_0^0x01;           // thread profile, P2.0
  Period0 = (time - First0)&0xFFFF; // 16 bits, 83.3 ns resolution
  First0 = time;                   // setup for next
  if(!Done0) Done0 = 1;
  else Sum0 += Period0;
  Count0++;
}
uint16_t Count2;
uint16_t Period2;              // (1/SMCLK) units = 83.3 ns units
uint16_t First2;               // Timer A3 first edge, P8.2
int Done2;
uint32_t Sum2;
void PeriodMeasure2(uint16_t time){
  P1_0 = P1_0^0x01;           // thread profile, P1.0
  Period2 = (time - First2)&0xFFFF; // 16 bits, 83.3 ns resolution
  First2 = time;                   // setup for next
  if(!Done2) Done2 = 1;
  else Sum2 += Period2;
  Count2++;
  if((Period2>12000)&&(Period2<15000)){
  buf[i]=0;
//  printf("%d\n",Period2);
//printf("%d\n",buf[i]);
  }
  else {
//  printf("%d\n",Period2);
  buf[i]=1;
//printf("%d\n",buf[i]);
  }
 i++;
//printf("i=%d\n",i);
  if(i>=18){
    for(z=1;z<=8;z++){
    u=u+buf[i-z-8];
    v=v+buf[i-z];
   }
//printf("u=%d,v=%d\n",u,v);
 }
if(u==0&&v==8){
     o=i+8;
  }
u=0;
v=0;
if(i==o){
  IR();
  }
}
/**
 * main.c
 */

void main(void)
{
    Clock_Init48MHz();  // makes SMCLK=12 MHz
    UART0_Initprintf(); // initialize UART and printf
 // UART1_Init();
    UART0_OutString("\nTest program for UART driver\n\rUART0_OutChar examples\n");
   // P2->SEL0 &= ~0x11;
   // P2->SEL1 &= ~0x11;   // configure P2.0 and P2.4 as GPIO
   // P2->DIR |= 0x11;     // P2.0 and P2.4 outputs
    First0 = First2 = 0; // first will be wrong
    Done0 = Done2 = 0;   // set on subsequent
    Time = 0;
//  Bump_Init();
    Motor_Init();        // activate Lab 13 software
    TimerA2Capture_Init(&PeriodMeasure2);
//  TimerA1_Init(&Collect,5000); // 100 Hz
// Motor_Forward(LeftDuty,RightDuty); // 37.5%  //3750
// TExaS_Init(LOGICANALYZER_P10);
// Motor_Backward(LeftDuty,RightDuty);
    EnableInterrupts();
    while(1){
    WaitForInterrupt();
    }
}
