#include <stdint.h>
#include <stdio.h>
#include "msp.h"
#include "../inc/UART0.h"
#include "../inc/UART3.h"
#include "../inc/Clock.h"


/**
 * main.c
 */
void main(void)
{
    char recData;
    char sendData[14]={0xFD, 0x00, 0x0B, 0x01, 0x01, 0xBD, 0xB3, 0xC5, 0xA3, 0xC9, 0xE7, 0xC7, 0xF8, 0x8F}; //GBK �����ʽ

    Clock_Init48MHz();  // makes SMCLK=12 MHz
    UART0_Initprintf(); // initialize UART and printf
    UART3_Init();
    while(1){
       //MSP432����������ģ�鷢�͡���ţ������
       UART3_Out_arr(sendData,14);

       recData = UART3_InChar();
       //����0x4E ������������ģ�����ںϳɲ����У�  ����0x4F �����ı�����ȷ�����Һϳɲ�����ϣ���������ģ�鴦�ڿ���״̬�� 0x41 �������ճɹ���
       printf("== %02x ==\n ",recData);
       Clock_Delay1ms(2000);
    }
}
