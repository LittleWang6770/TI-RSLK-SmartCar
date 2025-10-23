#include <stdint.h>
#include "msp.h"
#include "..\inc\BumpInt.h"
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
#include "..\inc\UART0.h"

//���
void TimedPause(uint32_t time)
{
    Clock_Delay1ms(time); // С��ֹͣ
    Motor_Stop();
}
int32_t position;
uint8_t Data;
uint16_t L = 10000;
uint16_t R = 10000;
void reflactance(void) //ѭ������
{
    Data = Reflectance_Read(500);  //5000ms ɨ��һ��
    position = Reflectance_Position(Data);  //����ѭ���Ľ����1 �ڣ�0 ��
    switch (Data)
    {
    case 0x06:
    case 0x03:
    case 0x07:
    case 0x1a:
    case 0x1c:
    case 0x1d:
    case 0x1e:
    case 0x0f:
    case 0x1f:
    case 0x3f:
    case 0x01:
    case 0x7f:Motor_Left(L, R);break;
    case 0x1b:  //0001 1011
    case 0x19:Motor_Left(0, R);break;  //0001 1001
    case 0x98:
    case 0x58:
    case 0xd8:
    case 0xf8:
    case 0xb8:
    case 0xf0:
    case 0xfc:
    case 0x80:
    case 0xfe:
    case 0x38:
    case 0x60:
    case 0xa0:
    case 0xe0: Motor_Right(L, R); break;
    case 0x00: Motor_Backward(L + 2000, R + 2000); Clock_Delay1ms(100); break;
    default:Motor_Forward(L, R); break;
    }
}

//��ײ
uint32_t CollisionData, CollisionFlag; // mailbox
uint32_t ch1, ch2, ch3;
int32_t z = 1195172;
int32_t e = 1058;
uint8_t data, cnt_collision = 0;

void HandleCollision(uint8_t bumpSensor)
{
    Motor_Stop();
    CollisionData = bumpSensor;
    CollisionFlag = 1;
    if (CollisionData == 0x1f || CollisionData == 0x2f || CollisionData == 0x37)
    {
        Motor_Backward(5500, 5500);
        Clock_Delay1ms(210);
        Motor_Right(5000, 5000);
        Clock_Delay1ms(270);
        Motor_Stop();
    }
    else if (CollisionData == 0x3b || CollisionData == 0x3d || CollisionData == 0x3e)
    {
        Motor_Backward(5500, 5500);
        Clock_Delay1ms(210);
        Motor_Left(5000, 5000);
        Clock_Delay1ms(270);
        Motor_Stop();
    }
}
void main(void) {
    uint8_t t;
    uint32_t distance;
    UART0_Initprintf();
    ADC0_InitSWTriggerCh17_12_16();                      //��ʼ�� AD ����
    CollisionFlag = 0;
    Clock_Init48MHz();                                   //ʱ�ӳ�ʼ��
    LaunchPad_Init();                                    //��ʼ�� GPIO �ӿ�
    Motor_Init();                                        // PWM ��ʼ��
    Motor_Forward(6000,6000);                            //���� 10ms�����ָߵ�ƽ�����ָߵ�ƽ
    PWM_Init34(10000, 5000, 5000);
    Reflectance_Init();
    while (LaunchPad_Input() == 0);                      // wait for touch ��ȡ����
    t = LaunchPad_Input();
    switch (t) {
    case 0x01:                                           //���� 1�����б���ѭ��
        while (1) {
            LaunchPad_Output(4);                         //��������
            ADC_In17_12_16(&ch1, &ch2, &ch3);
            distance = z / (ch2 - e);
            if (distance != 0 && distance < 150)
            {
                Motor_Right(5000, 5000);
                Clock_Delay1ms(1000);
                Motor_Stop();
            }
            else
            {
                reflactance();                           //ѭ��
            }
        }
        break;
    case 0x02:                                           //���� 2��������ײѭ��
        while (1)
        {
            BumpInt_Init(&HandleCollision);
            LaunchPad_Output(2);                         //Ĭ���̵�
            reflactance();                               //ѭ��
        }
        break;
    }
}
