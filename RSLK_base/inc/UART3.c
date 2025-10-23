// UART3.c
// Runs on MSP432
// Busy-wait device driver for the UART UCA3.
// Daniel Valvano
// September 23, 2017
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan && Mingjie Qiu

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M Microcontrollers",
       ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/

// UCA3RXD (VCP receive) connected to P9.6
// UCA3TXD (VCP transmit) connected to P9.7

#include <stdint.h>
#include <stdio.h>
#include "UART3.h"
#include "msp.h"

//------------UART3_Init------------
// Initialize the UART for 9600 baud rate (assuming 12 MHz SMCLK clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void UART3_Init(void){
  EUSCI_A3->CTLW0 = 0x0001;                   // hold the USCI module in reset mode
  // bit15=0,      no parity bits
  // bit14=x,      not used when parity is disabled
  // bit13=0,      LSB first
  // bit12=0,      8-bit data length
  // bit11=0,      1 stop bit
  // bits10-8=000, asynchronous UART mode
  // bits7-6=11,   clock source to SMCLK
  // bit5=0,       reject erroneous characters and do not set flag
  // bit4=0,       do not set flag for break characters
  // bit3=0,       not dormant
  // bit2=0,       transmit data, not address (not used here)
  // bit1=0,       do not transmit break (not used here)
  // bit0=1,       hold logic in reset state while configuring
  EUSCI_A3->CTLW0 = 0x00C1;
                                 // set the baud rate
                                 // N = clock/baud rate = 12,000,000/115,200 = 104.1667
  EUSCI_A3->BRW =  1250; //1250 -- 9600 //115200 ---104;    // UCBR = baud rate = int(N) = 1250
  EUSCI_A3->MCTLW &= ~0xFFF1;    // clear first and second modulation stage bit fields
  P9->SEL0 |= 0xC0;
  P9->SEL1 &= ~0xC0;             // configure P9.7 and P9.6 as primary module function

  EUSCI_A3->CTLW0 &= ~0x0001;    // enable the USCI module
  EUSCI_A3->IE &= ~0x000F;       // disable interrupts (transmit ready, start received, transmit empty, receive full)
}

//------------UART3_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
char UART3_InChar(void){
  while((EUSCI_A3->IFG&0x01) == 0);
  return((char)(EUSCI_A3->RXBUF));
}

//------------UART3_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
void UART3_OutChar(char letter){
  while((EUSCI_A3->IFG&0x02) == 0);
  EUSCI_A3->TXBUF = letter;
}
//------------UART3_Out Data array------------
// Output Data array
// Input: pointer to a Data array to be transferred ; Data array size;
// Output: none
void UART3_Out_arr(char *pt,int size){
    int i = 0;
    for(i=0;i<size;i++){
    UART3_OutChar(pt[i]);
 }
}
//------------UART3_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void UART3_OutString(char *pt){
  while(*pt){
    UART3_OutChar(*pt);
    pt++;
  }
}

//------------UART3_InUDec------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 32-bit unsigned number
//     valid range is 0 to 4294967295 (2^32-1)
// Input: none
// Output: 32-bit unsigned number
// If you enter a number above 4294967295, it will return an incorrect value
// Backspace will remove last digit typed
uint32_t UART3_InUDec(void){
uint32_t number=0, length=0;
char character;
  character = UART3_InChar();
  while(character != CR){ // accepts until <enter> is typed
// The next line checks that the input is a digit, 0-9.
// If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 4294967295
      length++;
      UART3_OutChar(character);
    }
// If the input is a backspace, then the return number is
// changed and a backspace is output to the screen
    else if((character==BS) && length){
      number /= 10;
      length--;
      UART3_OutChar(character);
    }
    character = UART3_InChar();
  }
  return number;
}

//-----------------------UART3_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void UART3_OutUDec(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    UART3_OutUDec(n/10);
    n = n%10;
  }
  UART3_OutChar(n+'0'); /* n is between 0 and 9 */
}
uint32_t Messageindexb;
char Messageb[8];
void static fillmessageb(uint32_t n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    fillmessageb(n/10);
    n = n%10;
  }
  Messageb[Messageindexb] = (n+'0'); /* n is between 0 and 9 */
  if(Messageindexb<7)Messageindexb++;
}
void static fillmessage4b(uint32_t n){
  if(n>=1000){  // 1000 to 9999
    Messageindexb = 0;
  } else if(n>=100){  // 100 to 999
    Messageb[0] = ' ';
    Messageindexb = 1;
  }else if(n>=10){ //
    Messageb[0] = ' '; /* n is between 10 and 99 */
    Messageb[1] = ' ';
    Messageindexb = 2;
  }else{
    Messageb[0] = ' '; /* n is between 0 and 9 */
    Messageb[1] = ' ';
    Messageb[2] = ' ';
    Messageindexb = 3;
  }
  fillmessageb(n);
}
void static fillmessage5b(uint32_t n){
  if(n>99999)n=99999;
  if(n>=10000){  // 10000 to 99999
    Messageindexb = 0;
  } else if(n>=1000){  // 1000 to 9999
    Messageb[0] = ' ';
    Messageindexb = 1;
  }else if(n>=100){  // 100 to 999
    Messageb[0] = ' ';
    Messageb[1] = ' ';
    Messageindexb = 2;
  }else if(n>=10){ //
    Messageb[0] = ' '; /* n is between 10 and 99 */
    Messageb[1] = ' ';
    Messageb[2] = ' ';
    Messageindexb = 3;
  }else{
    Messageb[0] = ' '; /* n is between 0 and 9 */
    Messageb[1] = ' ';
    Messageb[2] = ' ';
    Messageb[3] = ' ';
    Messageindexb = 4;
  }
  fillmessageb(n);
}
//-----------------------UART3_OutUDec4-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Fixed format 4 digits with no space before or after
void UART3_OutUDec4(uint32_t n){
  if(n>9999){
    UART3_OutString("****");
  }else{
    Messageindexb = 0;
    fillmessage4b(n);
    Messageb[Messageindexb] = 0; // terminate
    UART3_OutString(Messageb);
  }
}
//-----------------------UART3_OutUDec5-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Fixed format 5 digits with no space before or after
void UART3_OutUDec5(uint32_t n){
  if(n>99999){
    UART3_OutString("*****");
  }else{
    Messageindexb = 0;
    fillmessage5b(n);
    Messageb[Messageindexb] = 0; // terminate
    UART3_OutString(Messageb);
  }
}
//-----------------------UART3_OutUFix1-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// fixed format <digit>.<digit> with no space before or after
void UART3_OutUFix1(uint32_t n){
  UART3_OutUDec(n/10);
  UART3_OutChar('.');
  UART3_OutUDec(n%10);
}
//-----------------------UART3_OutUFix2-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// fixed format <digit>.<digit><digit> with no space before or after
void UART3_OutUFix2(uint32_t n){
  UART3_OutUDec(n/100);
  UART3_OutChar('.');
  n = n%100;
  UART3_OutUDec(n/10);
  UART3_OutUDec(n%10);
}
//---------------------UART3_InUHex----------------------------------------
// Accepts ASCII input in unsigned hexadecimal (base 16) format
// Input: none
// Output: 32-bit unsigned number
// No '$' or '0x' need be entered, just the 1 to 8 hex digits
// It will convert lower case a-f to upper-case A-F
//     and converts to a 16 bit unsigned number
//     value range is 0 to FFFFFFFF
// If you enter a number above FFFFFFFF, it will return an incorrect value
// Backspace will remove last digit typed
uint32_t UART3_InUHex(void){
uint32_t number=0, digit, length=0;
char character;
  character = UART3_InChar();
  while(character != CR){
    digit = 0x10; // assume bad
    if((character>='0') && (character<='9')){
      digit = character-'0';
    }
    else if((character>='A') && (character<='F')){
      digit = (character-'A')+0xA;
    }
    else if((character>='a') && (character<='f')){
      digit = (character-'a')+0xA;
    }
// If the character is not 0-9 or A-F, it is ignored and not echoed
    if(digit <= 0xF){
      number = number*0x10+digit;
      length++;
      UART3_OutChar(character);
    }
// Backspace outputted and return value changed if a backspace is inputted
    else if((character==BS) && length){
      number /= 0x10;
      length--;
      UART3_OutChar(character);
    }
    character = UART3_InChar();
  }
  return number;
}

//--------------------------UART3_OutUHex----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void UART3_OutUHex(uint32_t number){
// This function uses recursion to convert the number of
//   unspecified length as an ASCII string
  if(number >= 0x10){
    UART3_OutUHex(number/0x10);
    UART3_OutUHex(number%0x10);
  }
  else{
    if(number < 0xA){
      UART3_OutChar(number+'0');
     }
    else{
      UART3_OutChar((number-0x0A)+'A');
    }
  }
}
//--------------------------UART3_OutUHex2----------------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Fixed format 2 digits with no space before or after
void outnibble3(uint32_t n){
    if(n < 0xA){
   UART3_OutChar(n+'0');
  }
  else{
    UART3_OutChar((n-0x0A)+'A');
  }
}
void UART3_OutUHex2(uint32_t number){
  outnibble3(number/0x10); // ms digit
  outnibble3(number%0x10); // ls digit
}
//------------UART3_InString------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until <enter> is typed
//    or until max length of the string is reached.
// It echoes each character as it is input.
// If a backspace is input, the string is modified
//    and the backspace is echoed
// terminates the string with a null character
// uses busy-waiting synchronization on RDRF
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void UART3_InString(char *bufPt, uint16_t max) {
int length=0;
char character;
  character = UART3_InChar();
  while(character != CR){
    if(character == BS){
      if(length){
        bufPt--;
        length--;
        UART3_OutChar(BS);
      }
    }
    else if(length < max){
      *bufPt = character;
      bufPt++;
      length++;
      UART3_OutChar(character);
    }
    character = UART3_InChar();
  }
  *bufPt = 0;
}

// Get input from UART, echo
int fgetc3 (FILE *f){
  char ch = UART3_InChar();  // receive from keyboard
  UART3_OutChar(ch);            // echo
  return ch;
}
// Function called when file error occurs.
int ferror3(FILE *f){
  /* Your implementation of ferror */
  return EOF;
}

// Abstraction of general output device
// Volume 2 section 3.4.5

//-----CCS implementation
#include "file.h"
int uart_open3(const char *path, unsigned flags, int llv_fd){
  UART3_Init();
  return 0;
}
int uart_close3( int dev_fd){
    return 0;
}
int uart_read3(int dev_fd, char *buf, unsigned count){char ch;
  ch = UART3_InChar();    // receive from keyboard
  ch = *buf;         // return by reference
  UART3_OutChar(ch);  // echo
  return 1;
}
int uart_write3(int dev_fd, const char *buf, unsigned count){ unsigned int num=count;
    while(num){
        if(*buf == 10){
           UART3_OutChar(13);
        }
        UART3_OutChar(*buf);
        buf++;
        num--;
    }
    return count;
}
off_t uart_lseek3(int dev_fd, off_t ioffset, int origin){
    return 0;
}
int uart_unlink3(const char * path){
    return 0;
}
int uart_rename3(const char *old_name, const char *new_name){
    return 0;
}

//------------UART3_Initprintf------------
// Initialize the UART for 115,200 baud rate (assuming 48 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void UART3_Initprintf(void){int ret_val; FILE *fptr;
  UART3_Init();
  ret_val = add_device("uart", _SSA, uart_open3, uart_close3, uart_read3, uart_write3, uart_lseek3, uart_unlink3, uart_rename3);
  if(ret_val) return; // error
  fptr = fopen("uart","w");
  if(fptr == 0) return; // error
  freopen("uart:", "w", stdout); // redirect stdout to uart
  setvbuf(stdout, NULL, _IONBF, 0); // turn off buffering for stdout
}
/*
// Keil uVision Code
// Print a character to UART.
//------------UART3_Initprintf------------
// Initialize the UART3_Initprintf for 115,200 baud rate (assuming 3 MHz bus clock),
// 8 bit word length, no parity bits, one stop bit
// Input: none
// Output: none
void UART3_Initprintf(void){
  UART3_Init();
}
int fputc(int ch, FILE *f){
  if((ch == 10) || (ch == 13) || (ch == 27)){
    UART3_OutChar(13);
    UART3_OutChar(10);
    return 1;
  }
  UART3_OutChar(ch);
  return 1;
}
*/

