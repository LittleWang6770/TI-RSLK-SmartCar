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
 P2->SEL1 = 0x00; // configure P2.2-P2.0 as GPIO
 P2->DS = 0x07; // make P2.2-P2.0 high drive
 P2->DIR = 0x07; // make P2.2-P2.0 out
 P2->OUT = 0x00; // all LEDs off
}
void Port2_Output(uint8_t data){ // write all of P2 outputs
 P2->OUT = data;
}
void main(void) {
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
 case 0x18: //ֱ��
 Motor_Forward(L,R);
 break;
 case 0xFF: // T ��·�� ʮ��·��
 case 0x7F:
 case 0xFE:
 case 0x7E:
 Port2_Output(0x01);
 Motor_Forward(L,R);
 Clock_Delay1ms(150);
 Data = Reflectance_Read(500);
 if(Data != 0x00){
// Motor_Left(L,R+1000);
// Clock_Delay1ms(550); //��ת90��
 Port2_Output(0x05);
 Motor_Forward(L,R);
 Clock_Delay1ms(550);
 }
 else{
 Motor_Left(L,R+1000);
 Clock_Delay1ms(550);
 }
 break;
 case 0xF0: //��ת·�� ��ֱ��+��ת·��
 case 0xF8:
 case 0x78:
 Motor_Forward(L,R);
 Clock_Delay1ms(70);
 Data = Reflectance_Read(500);
 if(Data == 0x18 || Data == 0x08 || Data == 0x04 || Data ==
0x0C || Data == 0x10 || Data == 0x20 || Data == 0x30){ // ��T��·��
 Motor_Left(L,R+1000);
 Clock_Delay1ms(450);
 }
 else{
 Motor_Left(L,R+1000);
 Clock_Delay1ms(450);
 }
 break;
 case 0x08: //����������ƫ���ˣ���Ҫ����ת
 case 0x0C:
 Port2_Output(0x07);
 Motor_Right(L,R);
 break;
 case 0x10: //����������ƫ���ˣ���Ҫ����ת
 case 0x30:
 Port2_Output(0x07);
 Motor_Left(L,R);
 break;
 case 0x00: //��ͷ
// Motor_Left(L,R);
 Motor_Right(L,R);
 Clock_Delay1ms(500); //450
 break;
 }
 } }
