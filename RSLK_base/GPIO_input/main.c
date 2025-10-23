#include "msp.h"


void Port5_Init(void){
  P5->SEL0 = 0x00;
  P5->SEL1 = 0x00;//configure P5.0-P5.1 as GPIO
  P5->DIR = 0x02; //make P5.0 in£¬P5.1 out
  P5->REN = 0x01; //enable pull resistors on P5.0
  P5->OUT = 0x01; //P5.0 are pull-up
}

uint8_t Port5_Input(void){
  return (P5->IN&0x01); //read P5.0 inputs
}

void Port5_Output(uint8_t data){//write P5.1 outputs
  P5->OUT = (P5->OUT&0xFD)|data;
}


/**
 * main.c
 */
void main(void)
{
    uint8_t status;
    Port5_Init(); //initialize P5.0-P5.1 and make P5.0 input and P5.1 outputs
    while(1){
        status = Port5_Input(); //get P5.0 input status
        switch(status){
        case 0x01:
            Port5_Output(0x02); //control the buzzer module to make a sound
            break;
        case 0x00:
            Port5_Output(0x00);
            break;
        default:
            Port5_Output(0x00);
            break;
        }
    }
}
