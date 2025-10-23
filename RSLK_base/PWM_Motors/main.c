#include "msp.h"
#include "../inc/Clock.h"
#include "../inc/Motor.h"

/**
 * main.c
 */
void main(void)
{
    Clock_Init48MHz();
    Motor_Init();
    int right=8000;
    int left=8000;

    Motor_Forward(left,right);
    Clock_Delay1ms(1000);
    Motor_Backward(left,right);
    Clock_Delay1ms(1000);
    Motor_Stop();
    Clock_Delay1ms(1000);
}
