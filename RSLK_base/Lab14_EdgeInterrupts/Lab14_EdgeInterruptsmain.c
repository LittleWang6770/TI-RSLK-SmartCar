// Lab14_EdgeInterruptsmain.c
// Runs on MSP432, interrupt version, student start file
// Main test program for interrupt driven bump switches the robot.
// Daniel Valvano and Jonathan Valvano
// October 29, 2017

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
       ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// Negative logic bump sensors
// P4.7 Bump5, left side of robot
// P4.6 Bump4
// P4.5 Bump3
// P4.3 Bump2
// P4.2 Bump1
// P4.0 Bump0, right side of robot

#include <stdint.h>
#include "msp.h"
#include "../inc/Clock.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"
#include "../inc/Motor.h"
#include "../inc/BumpInt.h"
#include "../inc/TExaS.h"
#include "../inc/TimerA1.h"
#include "../inc/FlashProgram.h"
#include "../inc/UART0.h"

#include "..\inc\MotorSimple.h"
#include "..\inc\bump.h"
#include "..\inc\SysTick.h"



// Test of Periodic interrupt
//#define REDLED (*((volatile uint8_t *)(0x42098060)))    // Bit banding
//#define BLUELED (*((volatile uint8_t *)(0x42098068)))

uint32_t Time=0,Time_For_Forward=0;
uint32_t CollisionData, CollisionFlag;  // mailbox
int centerI = 0;
void HandleCollision(uint8_t bumpSensor){
   Motor_Stop();
   CollisionData = bumpSensor;
   CollisionFlag = 1;
   if(CollisionData == 0x1f || CollisionData == 0x2f || CollisionData == 0x37){
       P2->OUT ^= 0x01;
   }
   else if(CollisionData == 0x3b || CollisionData == 0x3d || CollisionData == 0x3e)
      // P2->OUT &= ~0x07;
       P2->OUT ^= 0x04;

   Time = 0;
   Time_For_Forward = 0;
}
#if 0
void TimerA1_Task(void){
  Time++;
  if(Time==1) Motor_Backward(5500,5000);    // Back up slowly for 1s
  else if(Time==30){
      switch (CollisionData){
          case 0x3E:Motor_Left(5800,5000); Time_For_Forward = 65; break;
          case 0x3D:Motor_Left(5800,5000); Time_For_Forward = 65; break;
          case 0x3B:Motor_Left(5800,5000); Time_For_Forward = 65; break;
          case 0x37:Motor_Right(5800,5000); Time_For_Forward = 65; break;
          case 0x2F:Motor_Right(5800,5000); Time_For_Forward = 65; break;
          case 0x1F:Motor_Right(5800,5000); Time_For_Forward = 65; break;
          default:Motor_Right(3000,3000); Time_For_Forward = 65; break;
      }
  }
  else if(Time==Time_For_Forward) Motor_Forward(9000,8000);
}
#endif
#if 1
void TimerA1_Task(void){
  int LeftSpeed = 9800, RightSpeed = 9000;
  Time++;
  if(Time==1){
      Motor_Backward(LeftSpeed,RightSpeed);    // Back up slowly for 1s
  }
  else if(Time==2){
      printf("%02x\n",CollisionData);
      switch (CollisionData){  //CollisionData
          case 0x3E:
              Motor_Left(LeftSpeed+500,RightSpeed+500);
              Clock_Delay1ms(400);
              Motor_Forward(LeftSpeed,RightSpeed);
              Clock_Delay1ms(600);
              Motor_Left(LeftSpeed+500,RightSpeed+500);
              Time_For_Forward = 30;break;
          case 0x3C:
              Motor_Left(LeftSpeed+500,RightSpeed+500);
              Clock_Delay1ms(400);
              Motor_Forward(LeftSpeed,RightSpeed);
              Clock_Delay1ms(600);
              Motor_Left(LeftSpeed+500,RightSpeed+500);
              Time_For_Forward = 20;break;
          case 0x38:
              Motor_Left(LeftSpeed,RightSpeed);Time_For_Forward = 10;break;
          case 0x3D:
          case 0x39:
              Motor_Left(LeftSpeed,0); Time_For_Forward = 11;break;
          case 0x30:
              Motor_Left(LeftSpeed,RightSpeed);Time_For_Forward = 6;break;
          case 0x31:
              Motor_Left(LeftSpeed,RightSpeed);Time_For_Forward = 4;break;
          case 0x3B:
          case 0x33:
          case 0x37:
                  Motor_Left(LeftSpeed,RightSpeed);
                  Clock_Delay1ms(150);
                  Motor_Right(LeftSpeed,RightSpeed);
                  Clock_Delay1ms(300);
                  Motor_Left(LeftSpeed,RightSpeed);
                  Clock_Delay1ms(450);
                  Motor_Left(LeftSpeed,RightSpeed);
                  Clock_Delay1ms(600);
                  Motor_Left(LeftSpeed,RightSpeed);

                  if(centerI%2==0)
                      Time_For_Forward = 3;
                  else
                      Time_For_Forward = 6;
                  centerI++;
              break;
          case 0x1e:  //对应最边两个 1 和 6
              Motor_Right(LeftSpeed,RightSpeed);Time_For_Forward = 5;break;
          case 0x23:
              Motor_Right(LeftSpeed,RightSpeed);Time_For_Forward = 4;break;
          case 0x03:
              Motor_Right(LeftSpeed,RightSpeed);Time_For_Forward = 6;break;
          case 0x2F:
          case 0x27:
              Motor_Right(0,RightSpeed); Time_For_Forward = 13;break; //Right
          case 0x07:
              Motor_Right(LeftSpeed,RightSpeed);Time_For_Forward = 8;break;
          case 0x1F:
              Motor_Right(LeftSpeed+500,RightSpeed+500);
              Clock_Delay1ms(400);
              Motor_Forward(LeftSpeed,RightSpeed);
              Clock_Delay1ms(600);
              Motor_Right(LeftSpeed+500,RightSpeed+500);
              Time_For_Forward = 30;break;
          case 0x0F:
              Motor_Right(LeftSpeed+500,RightSpeed+500);
              Clock_Delay1ms(400);
              Motor_Forward(LeftSpeed,RightSpeed);
              Clock_Delay1ms(600);
              Motor_Right(LeftSpeed+500,RightSpeed+500);
              Time_For_Forward = 15;break; //Right
          default: Motor_Forward(LeftSpeed,RightSpeed); Time_For_Forward = 13; break; //Right
      }
  }
  else if(Time==Time_For_Forward){
      Motor_Forward(LeftSpeed,RightSpeed);
      }
}
#endif

int main(void){
  DisableInterrupts();
  Clock_Init48MHz();   // 48 MHz clock; 12 MHz Timer A clock
  CollisionFlag = 0;
  LaunchPad_Init();
  // initialize P1.0 and make it output (P1.0 built-in LED1)
  P1->SEL0 &= ~0x01;
  P1->SEL1 &= ~0x01;                 // configure built-in LED1 as GPIO
  P1->DIR |= 0x01;                   // make built-in LED1 out
  P1->OUT &= ~0x01;                  // LED1 = off
  // initialize P2.2-P2.0 and make them outputs (P2.2-P2.0 built-in RGB LEDs)
  P2->SEL0 &= ~0x07;
  P2->SEL1 &= ~0x07;                 // configure built-in RGB LEDs as GPIO
  P2->DS |= 0x07;                    // make built-in RGB LEDs high drive strength
  P2->DIR |= 0x07;                   // make built-in RGB LEDs out
  P2->OUT &= ~0x07;


  UART0_Initprintf();
  Motor_Init();        // activate Lab 13 software
  TimerA1_Init(&TimerA1_Task,50000);  // 10 Hz
  Motor_Forward(5300,5000); // 50%
  BumpInt_Init(&HandleCollision);

// write this as part of Lab 14, section 14.4.4 Integrated Robotic System
  EnableInterrupts();
  while(1){

    WaitForInterrupt();
  }
}


int main1(void){  // test of interrupt-driven bump interface
  Clock_Init48MHz();   // 48 MHz clock; 12 MHz Timer A clock
  CollisionFlag = 0;
  LaunchPad_Init();
  Motor_Init();        // activate Lab 13 software
  Motor_Forward(7500,7500); // 50%
  BumpInt_Init(&HandleCollision);
  //TExaS_Init(LOGICANALYZER_P4_765320);
  EnableInterrupts();
  while(1){
    WaitForInterrupt();
  }
}

