#include "stdint.h"
#include "Clock.h"
#include "xunji.h"
#include "Motor.h"
#include <stdio.h>
#include <string.h>
#include "Reflectance.h"
void xunji(uint8_t data)
{
   int L = 14700,R = 14700;//L表示左轮的占空比；R表示右轮的占空比；取值范围：0~15000
   //鉴于原板卡读取U1，U5，U7存在问题；根据循迹原理，需要小车中心线左右对称的红外传感器来判断比较
   //故仅通过U3和U6进行循迹测试；
   printf("%d\n",data);
   Motor_Forward(L,R);
   switch(data){
                            case 0x18://直线:直行
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






