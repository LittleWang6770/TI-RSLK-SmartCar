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
    char sendData[14]={0xFD, 0x00, 0x0B, 0x01, 0x01, 0xBD, 0xB3, 0xC5, 0xA3, 0xC9, 0xE7, 0xC7, 0xF8, 0x8F}; //GBK 编码格式

    Clock_Init48MHz();  // makes SMCLK=12 MHz
    UART0_Initprintf(); // initialize UART and printf
    UART3_Init();
    while(1){
       //MSP432向语音播报模块发送“匠牛社区”
       UART3_Out_arr(sendData,14);

       recData = UART3_InChar();
       //返回0x4E 表明语音播报模块仍在合成播音中；  返回0x4F 表明文本被正确接收且合成播音完毕，语音播报模块处于空闲状态； 0x41 表明接收成功；
       printf("== %02x ==\n ",recData);
       Clock_Delay1ms(2000);
    }
}
