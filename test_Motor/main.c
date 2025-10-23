#include "msp.h"
#include "../RSLK_base/inc/Clock.h"
#include "../RSLK_base/inc/Motor.h"
#include "../RSLK_base/inc/PWM.h"
#include "../RSLK_base/inc/LaunchPad.h"
#include <stdio.h>
uint8_t dir,vel;
void Port1_Motor_Init(void)   //assign2_interrupt
{
    P1->SEL0 &= ~0xD2;
    P1->SEL1 &= ~0xD2;    // 1) configure P1.4 and P1.1 P1.6 & P1.7 as GPIO
    P1->DIR |= 0xC0;    // make P1.6 & P1.7 output
    P1->OUT &= ~0xC0;   //make P1.6 & P1.7 output is low
    P1->DIR &= ~0x12;     // 2) make P1.4 and P1.1 input
    P1->REN |= 0x12;   //上拉使能
    P1->OUT |= 0x12;   //上拉
    P1->IFG &= ~0x12;  //清除中断标志位
    P1->IE |= 0x12;   //中断使能
    P1->IES |= 0x12;   //下降沿触发
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;  // configure as GPIO
    P3->DIR |= 0xC0;    // make P3.6 & P3.7 out
    P3->OUT &= ~0xC0;   // low current sleep mode
}
void PORT1(void)
{    uint8_t  InFlag;
     InFlag=P1->IFG;
     if(InFlag & 0x02 )
    {
         if(dir>0x04)dir=0x01;
         else
             dir++;
         P1->IFG &=~0x02;
    }
    if(InFlag & 0x10 )
    {
        if(vel>0x03)vel=0x01;
        else
            vel++;
        P1->IFG &=~0x10;
    }
    P1->IFG=0x00;
}
uint8_t Port1_Input(void){
    return (P1->IN&0x12);  //read P1.4,P1.1 inputs
}
void main (void){
    Clock_Init48MHz();
    Port1_Motor_Init();
    int right=3500;
    int left=3500;
    dir=0x01;
    vel=0x01;//速度
    while(1){
        PORT1();
        switch(dir){
        case 0x02:
            Motor_Forward(left*vel,right*vel);
            break;
        case 0x03:
            Motor_Backward(left*vel,right*vel);
            break;
        case 0x04:
            Motor_Left(left*vel,right*vel);
            break;
        case 0x05:
            Motor_Right(left*vel,right*vel);
            break;
        case 0x01:
            Motor_Stop();
            break;
        default: break;
        }
    }
}
