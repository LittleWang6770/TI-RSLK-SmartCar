#include <stdio.h>
#include "msp.h"
#include "../inc/Clock.h"
#include "../inc/UART0.h"
#include "../inc/Reflectance.h"
#include "../inc/PWM.h"
#include "../inc/Motor.h"
#include "../inc/JN_LCD.h"

uint8_t Data;
uint16_t L = 5800;
uint16_t R = 6000;

void Port2_Init(void){
  P2->SEL0 = 0x00;
  P2->SEL1 = 0x00;                        // configure P2.2-P2.0 as GPIO
  P2->DS = 0x07;                          // make P2.2-P2.0 high drive strength
  P2->DIR = 0x07;                         // make P2.2-P2.0 out
  P2->OUT = 0x00;                         // all LEDs off
}

void Port2_Output(uint8_t data){        // write all of P2 outputs
  P2->OUT = data;
}

void main1(void)
{
    uint8_t Data;
    UART0_Initprintf();
    Clock_Init48MHz();
    Reflectance_Init();
    while(1){
        Data = Reflectance_Read(1000);
        printf("%02x\n",Data);
        Clock_Delay1ms(10);
    }
}

void main2(void)
{
    uint8_t Data;
    UART0_Initprintf();
    Clock_Init48MHz();
    Reflectance_Init();
    Motor_Init();

    while(1){
        Data = Reflectance_Read(1000);
        switch(Data){
            case 0x18:   //直行
                Motor_Forward(L,R);
                break;
            case 0x08:   //机器人向左偏移了，需要向右转
                Motor_Right(L,R);
                break;
            case 0x10:  //机器人向右偏移了，需要向左转
                Motor_Left(L,R);
                break;
            default:
                Motor_Stop();
                break;
        }
    }
}

void main(void)
{
    uint8_t Data = 0x00;
    UART0_Initprintf();
    Clock_Init48MHz();
    Reflectance_Init();
    Motor_Init();
    Port2_Init();

    while(1){
        Data = Reflectance_Read(500);
        printf("Data == %02x\n",Data);
        switch(Data){
            case 0x18:   //直行
                Motor_Forward(L,R);
                break;
            case 0xFF:  // T 型路口   十字路口
            case 0x7F:
            case 0xFE:
            case 0x7E:
                Port2_Output(0x01);
                Motor_Forward(L,R);
                Clock_Delay1ms(150);
                Data = Reflectance_Read(500);
                if(Data != 0x00){
                    Motor_Left(L,R+1000);
                    Clock_Delay1ms(550);  //左转90度
                }
                else{
                    Motor_Left(L,R+1000);
                    Clock_Delay1ms(550);
                }
                break;
            case 0xF0: //左转路口 、直行+左转路口
            case 0xF8:
            case 0x78:
                Motor_Forward(L,R);
                Clock_Delay1ms(70);
                Data = Reflectance_Read(500);
                if(Data == 0x18 || Data == 0x08 || Data == 0x04 || Data == 0x0C  || Data == 0x10 || Data == 0x20 || Data == 0x30){   // 左T字路口
                    Motor_Left(L,R+1000);
                    Clock_Delay1ms(450);
                }
                else{
                    Motor_Left(L,R+1000);
                    Clock_Delay1ms(450);
                }
                break;
            case 0x0F: //右转路口、 直行+右转路口
            case 0x1F:
            case 0x1E:
                Motor_Forward(L,R);
                Clock_Delay1ms(50);
                Data = Reflectance_Read(500);
                if(Data == 0xFF || Data == 0x7F || Data == 0x3F|| Data == 0xFE|| Data == 0xFC|| Data == 0x7E || Data == 0x7C || Data == 0x3E){
                    Port2_Output(0x04);
                    Motor_Left(L,R);
                    Clock_Delay1ms(550);
                }
                else{
                    Motor_Forward(L,R);
                    Clock_Delay1ms(55);
                    Data = Reflectance_Read(500);
                    if(Data == 0x18 || Data == 0x08 || Data == 0x04 || Data == 0x0C  || Data == 0x10 || Data == 0x20 || Data == 0x30){ //直行+右转路口 ，  直行
                        Port2_Output(0x02);
                        Motor_Forward(L,R);
                        Clock_Delay1ms(50);
                        Port2_Output(0x00);
                    }
                    else{
                        Port2_Output(0x03);
                        Motor_Right(L+1000,R);
                        Clock_Delay1ms(450); //450
                        Port2_Output(0x00);
                    }
                }
                break;
            case 0x08:  //机器人向左偏移了，需要向右转
            case 0x0C:
                Port2_Output(0x07);
                Motor_Right(L,R);
                break;
            case 0x10:  //机器人向右偏移了，需要向左转
            case 0x30:
                Port2_Output(0x07);
                Motor_Left(L,R);
                break;
            case 0x00:  //掉头
                Motor_Left(L,R);
                Clock_Delay1ms(500); //450
                break;

            default:
                Motor_Stop();
                break;
        }
    }
}

