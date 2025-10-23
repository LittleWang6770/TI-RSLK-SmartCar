#include "stdint.h"
#include "Clock.h"
#include "xunji.h"
#include "Motor.h"
#include <stdio.h>
#include <string.h>
#include "Reflectance.h"
void xunji(uint8_t data)
{
   int L = 14700,R = 14700;//L��ʾ���ֵ�ռ�ձȣ�R��ʾ���ֵ�ռ�ձȣ�ȡֵ��Χ��0~15000
   //����ԭ�忨��ȡU1��U5��U7�������⣻����ѭ��ԭ����ҪС�����������ҶԳƵĺ��⴫�������жϱȽ�
   //�ʽ�ͨ��U3��U6����ѭ�����ԣ�
   printf("%d\n",data);
   Motor_Forward(L,R);
   switch(data){
                            case 0x18://ֱ��:ֱ��
                            Motor_Forward(L,R);
                            break;
                            case 0x10:
                            Motor_Right(L,R-14800);
                            break;
                            case 0x30:
                            Motor_Right(L,R-14500);
                            break;
                            case 0x60:
                            Motor_Right(L,R-14000);
                            break;
                            case 0xc0:
                            Motor_Right(L,R-14000);
                            break;
                            case 0x80:
                            Motor_Right(L,R-13000);
                            break;
                            case 0x38:
                            Motor_Right(L,R-14300);
                            break;
                            case 0x70:
                            Motor_Right(L,R-13500);
                            break;
                            case 0x08:
                            Motor_Left(L-14800,R);
                            break;
                            case 0x0e://0e
                            Motor_Left(L-14500,R);
                            break;
                            case 0x1c:
                            Motor_Left(L-13000,R);
                            break;
                            case 0x0c://1c
                            Motor_Left(L-13500,R);
                            break;
                            case 0x06://06
                            Motor_Left(L-14500,R);
                            break;
                            case 0x03://03
                            Motor_Left(L-13500,R);
                            case 0x01:
                            Motor_Left(L-13000,R);
                            break;
                            case 0x19:
                            case 0x1b:
                            case 0x98:
                            case 0xd8:
                            Motor_Forward(L,R);
                            Clock_Delay1ms(100);
                            default:
                            printf("JangNiushequ");
                          }
}






