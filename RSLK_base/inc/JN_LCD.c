// JN_LCD.c
// Runs on MSP432
// Use eUSCI_A3 to send an 8-bit code to the JN_LCD 64x128
// pixel LCD to display text, images, or other information.
// jiangniushequ
// February 17, 2019
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

// JN_LCD
// -----------------------------------
// Signal        (JN_LCD) LaunchPad pin
// 3.3V          (VCC, pin 1) power
// Ground        (GND, pin 2) ground
// UCA3STE       (CS, pin 3) connected to P9.4
// Reset         (RST, pin 4) connected to P9.3
// Data/Command  (DC, pin 5) connected to P9.6
// UCA3SIMO      (DIN,  pin 6) connected to P9.7
// UCA3CLK       (CLK, pin 7) connected to P9.5
// back light    (LED, pin 8) not connected, consists of 4 3.3 V white LEDs which draw ~80mA total

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "msp.h"
#include "JN_LCD.h"

#define SCREENW     128
#define SCREENH     64
// *************************** Screen dimensions ***************************
// The regular 8-bit access for P9OUT  is 0x40004C82.
// For bit-banding of bit 6 of P9OUT, n=0x4C82 and b=6.
//    0x42000000 + 32*4C82 + 4*6 = 0x42099040+0x18 = 0x42099058
// For bit-banding of bit 3 of P9OUT, n=0x4C82 and b=3.
//    0x42000000 + 32*4C82 + 4*3 = 0x42099040+0x0C = 0x4209904C
#define DC          (*((volatile uint8_t *)0x42099058))   /* Port 9 Output, bit 6 is DC*/
#define RESET       (*((volatile uint8_t *)0x4209904C))   /* Port 9 Output, bit 3 is RESET*/
#define DC_BIT 0x40
#define RESET_BIT 0x08

//#define P9DIR                   (*((volatile uint8_t *)0x40004C84))   /* Port 9 Direction */
//#define P9SEL0                  (*((volatile uint8_t *)0x40004C8A))   /* Port 9 Select 0 */
//#define P9SEL1                  (*((volatile uint8_t *)0x40004C8C))   /* Port 9 Select 1 */
//#define UCA3CTLW0               (*((volatile uint16_t *)0x40001C00))  /* eUSCI_Ax Control Word Register 0 */
//#define UCA3BRW                 (*((volatile uint16_t *)0x40001C06))  /* eUSCI_Ax Baud Rate Control Word Register */
//#define UCA3MCTLW               (*((volatile uint16_t *)0x40001C08))  /* eUSCI_Ax Modulation Control Word Register */
//#define UCA3STATW               (*((volatile uint16_t *)0x40001C0A))  /* eUSCI_Ax Status Register */
//#define UCA3RXBUF               (*((volatile uint16_t *)0x40001C0C))  /* eUSCI_Ax Receive Buffer Register */
//#define UCA3TXBUF               (*((volatile uint16_t *)0x40001C0E))  /* eUSCI_Ax Transmit Buffer Register */
//#define UCA3IE                  (*((volatile uint16_t *)0x40001C1A))  /* eUSCI_Ax Interrupt Enable Register */
//#define UCA3IFG                 (*((volatile uint16_t *)0x40001C1C))  /* eUSCI_Ax Interrupt Flag Register */

// This table contains the hex values that represent pixels
// for a font that is 5 pixels wide and 8 pixels high
static const uint8_t ASCII[][5] = {
                                   {0x00, 0x00, 0x00, 0x00, 0x00} // 20
                                   ,{0x00, 0x00, 0x5f, 0x00, 0x00} // 21 !
                                   ,{0x00, 0x07, 0x00, 0x07, 0x00} // 22 "
                                   ,{0x14, 0x7f, 0x14, 0x7f, 0x14} // 23 #
                                   ,{0x24, 0x2a, 0x7f, 0x2a, 0x12} // 24 $
                                   ,{0x23, 0x13, 0x08, 0x64, 0x62} // 25 %
                                   ,{0x36, 0x49, 0x55, 0x22, 0x50} // 26 &
                                   ,{0x00, 0x05, 0x03, 0x00, 0x00} // 27 '
                                   ,{0x00, 0x1c, 0x22, 0x41, 0x00} // 28 (
                                   ,{0x00, 0x41, 0x22, 0x1c, 0x00} // 29 )
                                   ,{0x14, 0x08, 0x3e, 0x08, 0x14} // 2a *
                                   ,{0x08, 0x08, 0x3e, 0x08, 0x08} // 2b +
                                   ,{0x00, 0x50, 0x30, 0x00, 0x00} // 2c ,
                                   ,{0x08, 0x08, 0x08, 0x08, 0x08} // 2d -
                                   ,{0x00, 0x60, 0x60, 0x00, 0x00} // 2e .
                                   ,{0x20, 0x10, 0x08, 0x04, 0x02} // 2f /
                                   ,{0x3e, 0x51, 0x49, 0x45, 0x3e} // 30 0
                                   ,{0x00, 0x42, 0x7f, 0x40, 0x00} // 31 1
                                   ,{0x42, 0x61, 0x51, 0x49, 0x46} // 32 2
                                   ,{0x21, 0x41, 0x45, 0x4b, 0x31} // 33 3
                                   ,{0x18, 0x14, 0x12, 0x7f, 0x10} // 34 4
                                   ,{0x27, 0x45, 0x45, 0x45, 0x39} // 35 5
                                   ,{0x3c, 0x4a, 0x49, 0x49, 0x30} // 36 6
                                   ,{0x01, 0x71, 0x09, 0x05, 0x03} // 37 7
                                   ,{0x36, 0x49, 0x49, 0x49, 0x36} // 38 8
                                   ,{0x06, 0x49, 0x49, 0x29, 0x1e} // 39 9
                                   ,{0x00, 0x36, 0x36, 0x00, 0x00} // 3a :
                                   ,{0x00, 0x56, 0x36, 0x00, 0x00} // 3b ;
                                   ,{0x08, 0x14, 0x22, 0x41, 0x00} // 3c <
                                   ,{0x14, 0x14, 0x14, 0x14, 0x14} // 3d =
                                   ,{0x00, 0x41, 0x22, 0x14, 0x08} // 3e >
                                   ,{0x02, 0x01, 0x51, 0x09, 0x06} // 3f ?
                                   ,{0x32, 0x49, 0x79, 0x41, 0x3e} // 40 @
                                   ,{0x7e, 0x11, 0x11, 0x11, 0x7e} // 41 A
                                   ,{0x7f, 0x49, 0x49, 0x49, 0x36} // 42 B
                                   ,{0x3e, 0x41, 0x41, 0x41, 0x22} // 43 C
                                   ,{0x7f, 0x41, 0x41, 0x22, 0x1c} // 44 D
                                   ,{0x7f, 0x49, 0x49, 0x49, 0x41} // 45 E
                                   ,{0x7f, 0x09, 0x09, 0x09, 0x01} // 46 F
                                   ,{0x3e, 0x41, 0x49, 0x49, 0x7a} // 47 G
                                   ,{0x7f, 0x08, 0x08, 0x08, 0x7f} // 48 H
                                   ,{0x00, 0x41, 0x7f, 0x41, 0x00} // 49 I
                                   ,{0x20, 0x40, 0x41, 0x3f, 0x01} // 4a J
                                   ,{0x7f, 0x08, 0x14, 0x22, 0x41} // 4b K
                                   ,{0x7f, 0x40, 0x40, 0x40, 0x40} // 4c L
                                   ,{0x7f, 0x02, 0x0c, 0x02, 0x7f} // 4d M
                                   ,{0x7f, 0x04, 0x08, 0x10, 0x7f} // 4e N
                                   ,{0x3e, 0x41, 0x41, 0x41, 0x3e} // 4f O
                                   ,{0x7f, 0x09, 0x09, 0x09, 0x06} // 50 P
                                   ,{0x3e, 0x41, 0x51, 0x21, 0x5e} // 51 Q
                                   ,{0x7f, 0x09, 0x19, 0x29, 0x46} // 52 R
                                   ,{0x46, 0x49, 0x49, 0x49, 0x31} // 53 S
                                   ,{0x01, 0x01, 0x7f, 0x01, 0x01} // 54 T
                                   ,{0x3f, 0x40, 0x40, 0x40, 0x3f} // 55 U
                                   ,{0x1f, 0x20, 0x40, 0x20, 0x1f} // 56 V
                                   ,{0x3f, 0x40, 0x38, 0x40, 0x3f} // 57 W
                                   ,{0x63, 0x14, 0x08, 0x14, 0x63} // 58 X
                                   ,{0x07, 0x08, 0x70, 0x08, 0x07} // 59 Y
                                   ,{0x61, 0x51, 0x49, 0x45, 0x43} // 5a Z
                                   ,{0x00, 0x7f, 0x41, 0x41, 0x00} // 5b [
                                   ,{0x02, 0x04, 0x08, 0x10, 0x20} // 5c '\'
                                   ,{0x00, 0x41, 0x41, 0x7f, 0x00} // 5d ]
                                   ,{0x04, 0x02, 0x01, 0x02, 0x04} // 5e ^
                                   ,{0x40, 0x40, 0x40, 0x40, 0x40} // 5f _
                                   ,{0x00, 0x01, 0x02, 0x04, 0x00} // 60 `
                                   ,{0x20, 0x54, 0x54, 0x54, 0x78} // 61 a
                                   ,{0x7f, 0x48, 0x44, 0x44, 0x38} // 62 b
                                   ,{0x38, 0x44, 0x44, 0x44, 0x20} // 63 c
                                   ,{0x38, 0x44, 0x44, 0x48, 0x7f} // 64 d
                                   ,{0x38, 0x54, 0x54, 0x54, 0x18} // 65 e
                                   ,{0x08, 0x7e, 0x09, 0x01, 0x02} // 66 f
                                   ,{0x0c, 0x52, 0x52, 0x52, 0x3e} // 67 g
                                   ,{0x7f, 0x08, 0x04, 0x04, 0x78} // 68 h
                                   ,{0x00, 0x44, 0x7d, 0x40, 0x00} // 69 i
                                   ,{0x20, 0x40, 0x44, 0x3d, 0x00} // 6a j
                                   ,{0x7f, 0x10, 0x28, 0x44, 0x00} // 6b k
                                   ,{0x00, 0x41, 0x7f, 0x40, 0x00} // 6c l
                                   ,{0x7c, 0x04, 0x18, 0x04, 0x78} // 6d m
                                   ,{0x7c, 0x08, 0x04, 0x04, 0x78} // 6e n
                                   ,{0x38, 0x44, 0x44, 0x44, 0x38} // 6f o
                                   ,{0x7c, 0x14, 0x14, 0x14, 0x08} // 70 p
                                   ,{0x08, 0x14, 0x14, 0x18, 0x7c} // 71 q
                                   ,{0x7c, 0x08, 0x04, 0x04, 0x08} // 72 r
                                   ,{0x48, 0x54, 0x54, 0x54, 0x20} // 73 s
                                   ,{0x04, 0x3f, 0x44, 0x40, 0x20} // 74 t
                                   ,{0x3c, 0x40, 0x40, 0x20, 0x7c} // 75 u
                                   ,{0x1c, 0x20, 0x40, 0x20, 0x1c} // 76 v
                                   ,{0x3c, 0x40, 0x30, 0x40, 0x3c} // 77 w
                                   ,{0x44, 0x28, 0x10, 0x28, 0x44} // 78 x
                                   ,{0x0c, 0x50, 0x50, 0x50, 0x3c} // 79 y
                                   ,{0x44, 0x64, 0x54, 0x4c, 0x44} // 7a z
                                   ,{0x00, 0x08, 0x36, 0x41, 0x00} // 7b {
                                   ,{0x00, 0x00, 0x7f, 0x00, 0x00} // 7c |
                                   ,{0x00, 0x41, 0x36, 0x08, 0x00} // 7d }
                                   ,{0x10, 0x08, 0x08, 0x10, 0x08} // 7e ~
                                   //  ,{0x78, 0x46, 0x41, 0x46, 0x78} // 7f DEL
                                   ,{0x1f, 0x24, 0x7c, 0x24, 0x1f} // 7f UT sign
};

//jiangniu's English icon
//Modulating mode: column type, reverse
const uint8_t Str_USB_08x16[8][16]=
{
 {0x00,0x00,0x00,0x04,0xFC,0xFC,0x04,0x00,0x07,0x0F,0x08,0x08,0x0F,0x07,0x00,0x00},/*J*/
 {0x00,0x00,0x20,0xEC,0xEC,0x00,0x00,0x00,0x00,0x00,0x08,0x0F,0x0F,0x08,0x00,0x00},/*i*/
 {0x00,0xA0,0xA0,0xA0,0xE0,0xC0,0x00,0x00,0x07,0x0F,0x08,0x08,0x07,0x0F,0x08,0x00},/*a*/
 {0x20,0xE0,0xC0,0x20,0x20,0xE0,0xC0,0x00,0x00,0x0F,0x0F,0x00,0x00,0x0F,0x0F,0x00},/*n*/
 {0xC0,0xE0,0x20,0x20,0xC0,0xE0,0x20,0x00,0x27,0x6F,0x48,0x48,0x7F,0x3F,0x00,0x00},/*g*/
 {0xFC,0xFC,0x38,0x70,0xE0,0xFC,0xFC,0x00,0x0F,0x0F,0x00,0x00,0x00,0x0F,0x0F,0x00},/*N*/
 {0x00,0x00,0x20,0xEC,0xEC,0x00,0x00,0x00,0x00,0x00,0x08,0x0F,0x0F,0x08,0x00,0x00},/*i*/
 {0xE0,0xE0,0x00,0x00,0xE0,0xE0,0x00,0x00,0x07,0x0F,0x08,0x08,0x07,0x0F,0x08,0x00},/*u*/

};

//JN's bull head icon
//Width x height = 45x48
uint8_t JN_Log[] ={
                   0xE0,0xF8,0xFC,0xFE,0xFE,0x1E,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                   0x00,0x00,0x00,0x00,0x00,0x00,0x83,0x87,0xCF,0xFF,0xFE,0xFE,0xF8,0x3F,0x7F,0x7F,
                   0xFF,0xFF,0xFF,0xFE,0xFE,0xFC,0xFC,0xFC,0xFC,0xFC,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,
                   0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xF8,0xFC,0xFC,0xFC,
                   0xFE,0xFE,0xFF,0xFF,0xFF,0x7F,0x3F,0x8F,0x07,0x01,0x00,0x00,0x00,0x00,0x01,0x01,
                   0x01,0x03,0x03,0x03,0x07,0x37,0xE7,0xEF,0xEF,0xEF,0xEF,0xCF,0xCF,0xDF,0xDF,0xDF,
                   0xFF,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x9F,
                   0x3D,0x3E,0x3E,0x1F,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                   0x00,0x00,0x00,0x00,0x01,0x01,0x03,0x03,0x07,0x07,0x07,0x07,0x03,0x01,0x00,0x01,
                   0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x80,0x80,
                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0xFC,0xFC,0xFC,0xFC,0xFD,0xFF,
                   0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x3F,0x0E,0x00,0x00,
                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x03,0x03,0x03,0x03,0x03,0x03,
                   0x03,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

//Variable for defining position
uint16_t X1=0,Y1=0;

//********lcdcommandwrite*****************
// This is a helper function that sends 8-bit commands to the LCD.
// Inputs: command  8-bit function code to transmit
// Outputs: none
// Assumes: UCA3 and Port 9 have already been initialized and enabled
// The Data/Command pin must be valid when the eighth bit is
// sent.  The eUSCI module has no hardware FIFOs.
// 1) Wait for SPI to be idle (let previous frame finish)
// 2) Set DC for command (0)
// 3) Write command to TXBUF, starts SPI
// 4) Wait for SPI to be idle (after transmission complete)
void static lcdcommandwrite(uint8_t command){
    // write this code as part of Lab 11
    while(EUSCI_A3->STATW&0x01);
    DC=0;
    EUSCI_A3->TXBUF=command;
    while(EUSCI_A3->STATW&0x01);
}

//********lcddatawrite*****************
// This is a helper function that sends 8-bit data to the LCD.
// Inputs: data  8-bit data to transmit
// Outputs: none
// Assumes: UCA3 and Port 9 have already been initialized and enabled
// The Data/Command pin must be valid when the eighth bit is
// sent.  The eUSCI module has no hardware FIFOs.
// 1) Wait for transmitter to be empty (let previous frame finish)
// 2) Set DC for data (1)
// 3) Write data to TXBUF, starts SPI
void static lcddatawrite(uint8_t data){
    // write this code as part of Lab 11
    while(EUSCI_A3->STATW&0x01);
    DC=1;
    EUSCI_A3->TXBUF=data;
    //while(EUSCI_A3->STATW&0x01);
}

//********JN_LCD_Clear_0*****************
// Clear the LCD by writing 0x00 to the entire screen
// Inputs: none
// Outputs: none
void JN_LCD_Clear_0(void){
    int i,j;
    for(i=0; i<8; i++){
        lcdcommandwrite(0xb0+i);
        lcdcommandwrite(0x02);
        lcdcommandwrite(0x10);
        for(j=0;j<128;j++){
            lcddatawrite(0x00);
        }
    }
}

//********JN_LCD_Clear_1*****************
// Clear the LCD by writing 0xff to the entire screen
// Inputs: none
// Outputs: none
void JN_LCD_Clear_1(void){
    int i,j;
    for(i=0; i<8; i++){
        lcdcommandwrite(0xb0+i);
        lcdcommandwrite(0x02);
        lcdcommandwrite(0x10);
        for(j=0;j<128;j++){

            lcddatawrite(0xff);}
    }
}

//********JN_LCD_Init*****************
// Initialize JN_LCD 64x128 LCD by sending the proper
// commands to the PCD8544 driver.  One feature of the
// MSP432 is that its SSIs can get their baud clock from
// either the auxiliary clock (ACLK = REFOCLK/1 = 32,768 Hz
// see ClockSystem.c) or from the low-speed subsystem
// master clock (SMCLK <= 12 MHz see ClockSystem.c).  The
// SSI can further divide this clock signal by using the
// 16-bit Bit Rate Control prescaler Register, UCAxBRW.
// Inputs: none
// Outputs: none
// Assumes: low-speed subsystem master clock 12 MHz
void JN_LCD_Init(void){
    volatile uint32_t delay;
    EUSCI_A3->CTLW0 = 0x0001;             // hold the eUSCI module in reset mode
    // configure UCA3CTLW0 for:
    // bit15      UCCKPH = 1; data shifts in on first edge, out on following edge
    // bit14      UCCKPL = 0; clock is low when inactive
    // bit13      UCMSB = 1; MSB first
    // bit12      UC7BIT = 0; 8-bit data
    // bit11      UCMST = 1; master mode
    // bits10-9   UCMODEx = 2; UCSTE active low
    // bit8       UCSYNC = 1; synchronous mode
    // bits7-6    UCSSELx = 2; eUSCI clock SMCLK
    // bits5-2    reserved
    // bit1       UCSTEM = 1; UCSTE pin enables slave
    // bit0       UCSWRST = 1; reset enabled
    EUSCI_A3->CTLW0 = 0xAD83;
    // set the baud rate for the eUSCI which gets its clock from SMCLK
    // Clock_Init48MHz() from ClockSystem.c sets SMCLK = HFXTCLK/4 = 12 MHz
    // if the SMCLK is set to 12 MHz, divide by 3 for 4 MHz baud clock
    EUSCI_A3->BRW = 3;
    // modulation is not used in SPI mode, so clear UCA3MCTLW
    EUSCI_A3->MCTLW = 0;
    P9->SEL0 |= 0xB0;
    P9->SEL1 &= ~0xB0;// configure P9.7, P9.5, and P9.4 as primary module function
    P9->SEL0 &= ~(DC_BIT|RESET_BIT);
    P9->SEL1 &= ~(DC_BIT|RESET_BIT);// configure P9.3 and P9.6 as GPIO (Reset and D/C pins)
    P9->DIR |= (DC_BIT|RESET_BIT); // make P9.3 and P9.6 out (Reset and D/C pins)
    EUSCI_A3->CTLW0 &= ~0x0001;           // enable eUSCI module
    EUSCI_A3->IE &= ~0x0003;              // disable interrupts
    RESET = 0;// reset the LCD to a known state, RESET low
    for(delay=0; delay<100; delay=delay+1);// delay minimum 1000 ns
    RESET = 1; // hold RESET high
    lcdcommandwrite(0xAE);//--turn off oled panel
    lcdcommandwrite(0xA1);//--Set SEG/Column Mapping
    lcdcommandwrite(0xC8);//Set COM/Row Scan Direction
    lcdcommandwrite(0xaf);//--turn on oled panel
    JN_LCD_Clear_0();
}

//********JN_LCD_OutChar*****************
// Print a character to the JN_LCD  64x128 LCD.  The
// character will be printed at the current cursor position,
// the cursor will automatically be updated, and it will
// wrap to the next row or back to the top if necessary.
// of the character for readability.  Since characters are 8
// pixels tall and 5 pixels wide, 18 characters fit per row,
// and there are 8 rows.
// Inputs: data  character to print
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void JN_LCD_OutChar(char data){int i;

if((X1>121)||(data == 0x0D)){
    Y1 = Y1+1;
    X1 = 0;
    if(Y1>7){
        Y1 = 0;
    }
}
JN_LCD_Set_Pos(X1,Y1);
lcddatawrite(0x00);// blank vertical line padding
for(i=0; i<5; i=i+1){
    lcddatawrite(ASCII[data - 0x20][i]);
}
lcddatawrite(0x00);        // blank vertical line padding
X1 = X1+7;
}

//********JN_LCD_OutString*****************
// Print a character to the JN_LCD  64x128 LCD.
// The string will automatically wrap, so padding spaces may
// be needed to make the output look optimal.
// Inputs: ptr  pointer to NULL-terminated ASCII string
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void JN_LCD_OutString(char *ptr){
    while(*ptr){
        JN_LCD_OutChar((unsigned char)*ptr);
        ptr = ptr + 1;
    }
}

//********JN_LCD_Set_Pos*****************
// Move the cursor to the desired X- and Y-position.  The
// next character will be printed here.  X=1 is the leftmost
// column.  Y=0 is the top row.
// Inputs: newX  new X-position of the cursor (0<=newX<=126)
//         newY  new Y-position of the cursor (0<=newY<=7)
// Outputs: none
void JN_LCD_Set_Pos(unsigned char x, unsigned char y)
{
    X1=x;
    Y1=y;
    lcdcommandwrite(0xb0+(y&0x07));/* set page start address */
    lcdcommandwrite(((x+1)&0x0f)|0x00);/* set lower nibble of the column address */
    lcdcommandwrite((((x+1)&0xf0)>>4)|0x10); /* set higher nibble of the column address */
}

//********JN_LCD_OutUDec*****************
// Output a 16-bit number in unsigned decimal format with a
// fixed size of five right-justified digits of output.
// Inputs: n  16-bit unsigned number
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void JN_LCD_OutUDec(uint16_t n){
    if(n < 10){
        JN_LCD_OutString(" ");
        JN_LCD_OutChar(n+'0'); /* n is between 0 and 9 */
    } else if(n<100){
        JN_LCD_OutString("   ");
        JN_LCD_OutChar(n/10+'0'); /* tens digit */
        JN_LCD_OutChar(n%10+'0'); /* ones digit */
    } else if(n<1000){
        JN_LCD_OutString("  ");
        JN_LCD_OutChar(n/100+'0'); /* hundreds digit */
        n = n%100;
        JN_LCD_OutChar(n/10+'0'); /* tens digit */
        JN_LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else if(n<10000){
        JN_LCD_OutChar(' ');
        JN_LCD_OutChar(n/1000+'0'); /* thousands digit */
        n = n%1000;
        JN_LCD_OutChar(n/100+'0'); /* hundreds digit */
        n = n%100;
        JN_LCD_OutChar(n/10+'0'); /* tens digit */
        JN_LCD_OutChar(n%10+'0'); /* ones digit */
    }
    else {
        JN_LCD_OutChar(n/10000+'0'); /* ten-thousands digit */
        n = n%10000;
        JN_LCD_OutChar(n/1000+'0'); /* thousands digit */
        n = n%1000;
        JN_LCD_OutChar(n/100+'0'); /* hundreds digit */
        n = n%100;
        JN_LCD_OutChar(n/10+'0'); /* tens digit */
        JN_LCD_OutChar(n%10+'0'); /* ones digit */
    }
}

//********JN_LCD_OutSDec*****************
// Output a 16-bit number in signed decimal format with a
// fixed size of six right-justified digits of output.
// Inputs: n  16-bit signed number
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void JN_LCD_OutSDec(int16_t n){char sign=' ';
if(n<0){
    sign = '-';
    n =  -n; // positive now
}
if(n < 10){
    JN_LCD_OutString("    ");
    JN_LCD_OutChar(sign);
    JN_LCD_OutChar(n+'0'); /* n is between 0 and 9 */
} else if(n<100){
    JN_LCD_OutString("   ");
    JN_LCD_OutChar(sign);
    JN_LCD_OutChar(n/10+'0'); /* tens digit */
    JN_LCD_OutChar(n%10+'0'); /* ones digit */
} else if(n<1000){
    JN_LCD_OutString("  ");
    JN_LCD_OutChar(sign);
    JN_LCD_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    JN_LCD_OutChar(n/10+'0'); /* tens digit */
    JN_LCD_OutChar(n%10+'0'); /* ones digit */
}
else if(n<10000){
    JN_LCD_OutChar(' ');
    JN_LCD_OutChar(sign);
    JN_LCD_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    JN_LCD_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    JN_LCD_OutChar(n/10+'0'); /* tens digit */
    JN_LCD_OutChar(n%10+'0'); /* ones digit */
}
else {
    JN_LCD_OutChar(sign);
    JN_LCD_OutChar(n/10000+'0'); /* ten-thousands digit */
    n = n%10000;
    JN_LCD_OutChar(n/1000+'0'); /* thousands digit */
    n = n%1000;
    JN_LCD_OutChar(n/100+'0'); /* hundreds digit */
    n = n%100;
    JN_LCD_OutChar(n/10+'0'); /* tens digit */
    JN_LCD_OutChar(n%10+'0'); /* ones digit */
}
}

//********JN_LCD_OutUFix1*****************
// Output a 16-bit number in unsigned 3-digit fixed point, 0.1 resolution
// numbers 0 to 999 printed as " 0.0" to "99.9"
// Inputs: n  16-bit unsigned number
// Outputs: none
void JN_LCD_OutUFix1(uint16_t n){
    char message[5];
    if(n>999)n=999;
    if(n>=100){  // 100 to 999
        message[0] = (n/100+'0'); /* tens digit */
        n = n%100; //the rest
    }else { // 0 to 99
        message[0] = ' '; /* n is between 0.0 and 9.9 */
    }
    message[1] = (n/10+'0'); /* ones digit */
    n = n%10; //the rest
    message[2] = '.';
    message[3] = (n+'0'); /* tenths digit */
    message[4] = 0;
    JN_LCD_OutString(message);
}

//********JN_LCD_DrawFullImage*****************
// Fill the whole screen by drawing a 64x128 bitmap image.
// Inputs: ptr  pointer to 1024 byte bitmap
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void JN_LCD_DrawFullImage(const uint8_t *ptr){
    uint8_t row = 0,col = 0;
    JN_LCD_Set_Pos(0,0);
    for(row = 0xb2; row < 0xba; row++)
    {
        lcdcommandwrite(row);
        lcdcommandwrite(0x02);
        lcdcommandwrite(0x10);
        for(col = 0; col < 128; col++)
            lcddatawrite(*ptr++);
    }
}
uint8_t Screen[SCREENW*SCREENH/8]; // buffer stores the next image to be printed on the screen

//********JN_LCD_PrintBMP*****************
// Bitmaps defined above were created for the LM3S1968 or
// LM3S8962's 4-bit grayscale OLED display.  They also
// still contain their header data and may contain padding
// to preserve 4-byte alignment.  This function takes a
// bitmap in the previously described format and puts its
// image data in the proper location in the buffer so the
// image will appear on the screen after the next call to
//  JN_LCD_DisplayBuffer();
// The interface and operation of this process is modeled
// after RIT128x96x4_BMP(x, y, image);
// Inputs: xpos      horizontal position of bottom left corner of image, columns from the left edge
//                     must be less than 128
//                     0 is on the left; 126 is near the right
//         ypos      vertical position of bottom left corner of image, rows from the top edge
//                     must be less than 48
//                     2 is near the top; 63 is at the bottom
//         ptr       pointer to a 16 color BMP image
//         threshold grayscale colors above this number make corresponding pixel 'on'
//                     0 to 14
//                     0 is fine for ships, explosions, projectiles, and bunkers
// Outputs: none
void JN_LCD_PrintBMP(uint8_t xpos, uint8_t ypos, const uint8_t *ptr, uint8_t threshold){
    int32_t width = ptr[18], height = ptr[22], i, j;
    uint16_t screenx, screeny;
    uint8_t mask;
    // check for clipping
    if((height <= 0) ||              // bitmap is unexpectedly encoded in top-to-bottom pixel order
            ((width%2) != 0) ||           // must be even number of columns
            ((xpos + width) > SCREENW) || // right side cut off
            (ypos < (height - 1)) ||      // top cut off
            (ypos > SCREENH))           { // bottom cut off
        return;
    }
    if(threshold > 14){
        threshold = 14;             // only full 'on' turns pixel on
    }
    // bitmaps are encoded backwards, so start at the bottom left corner of the image
    screeny = ypos/8;
    screenx = xpos + SCREENW*screeny;
    mask = ypos%8;                // row 0 to 7
    mask = 0x01<<mask;            // now stores a mask 0x01 to 0x80
    j = ptr[10];                  // byte 10 contains the offset where image data can be found
    for(i=1; i<=(width*height/2); i=i+1){
        // the left pixel is in the upper 4 bits
        if(((ptr[j]>>4)&0xF) > threshold){
            Screen[screenx] |= mask;
        } else{
            Screen[screenx] &= ~mask;
        }
        screenx = screenx + 1;
        // the right pixel is in the lower 4 bits
        if((ptr[j]&0xF) > threshold){
            Screen[screenx] |= mask;
        } else{
            Screen[screenx] &= ~mask;
        }
        screenx = screenx + 1;
        j = j + 1;
        if((i%(width/2)) == 0){     // at the end of a row
            if(mask > 0x01){
                mask = mask>>1;
            } else{
                mask = 0x80;
                screeny = screeny - 1;
            }
            screenx = xpos + SCREENW*screeny;
            // bitmaps are 32-bit word aligned
            switch((width/2)%4){      // skip any padding
            case 0: j = j + 0; break;
            case 1: j = j + 3; break;
            case 2: j = j + 2; break;
            case 3: j = j + 1; break;
            }
        }
    }
}

//********JN_LCD_ClearBuffer*****************
// There is a buffer in RAM that holds one screen.
// This routine clears that buffer.
// Inputs: none
// Outputs: none
void JN_LCD_ClearBuffer(void){int i;
for(i=0; i<SCREENW*SCREENH/8; i=i+1){
    Screen[i] = 0;              // clear buffer
}
}

//********JN_LCD_DisplayBuffer*****************
// Fill the whole screen by drawing a 48x84 screen image
// from the RAM buffer.
// Inputs: none
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
void JN_LCD_DisplayBuffer(void){
    JN_LCD_DrawFullImage(Screen);
}

const unsigned char Masks[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
//------------JN_LCD_ClrPxl------------
// Clear the internal screen buffer pixel at (i, j),
// turning it off.
// Input: i  the row index  (0 to 63 in this case),    y-coordinate
//        j  the column index  (0 to 127 in this case), x-coordinate
// Output: none
void JN_LCD_ClrPxl(uint32_t i, uint32_t j){
    Screen[128*(i>>3) + j] &= ~Masks[i&0x07];
}

//------------JN_LCD_SetPxl------------
// Set the internal screen buffer pixel at (i, j),
// turning it on.
// Input: i  the row index  (0 to 63 in this case),    y-coordinate
//        j  the column index  (0 to 127 in this case), x-coordinate
// Output: none
void JN_LCD_SetPxl(uint32_t i, uint32_t j){
    Screen[128*(i>>3) + j] |= Masks[i&0x07];
}

//********display_char1106*****************
//Inputs:pArray pointer to bitmap
//Show JiangNiu in English, this function will only show a single letter

void display_char1106(uint8_t x,uint8_t y,uint8_t w,const uint8_t pArray[][16])
{
    uint8_t j;
    uint8_t buf_temp[8];
    memset(buf_temp,0x00,8);

    JN_LCD_Set_Pos(x+32,y+4);
    for(j=0;j<8;j++){
        buf_temp[j] = pArray[w][j];
    }
    while(EUSCI_A3->STATW&0x01);
    DC=1;
    int i;
    for(i=0;i<=7;i++){
        EUSCI_A3->TXBUF=buf_temp[i];
        while(EUSCI_A3->STATW&0x01);
    }
    memset(buf_temp,0x00,8);

    JN_LCD_Set_Pos(x+32,y+5);
    for(j=8;j<16;j++){
        buf_temp[j-8] = pArray[w][j];
    }
    while(EUSCI_A3->STATW&0x01);
    DC=1;
    //  int i;
    for(i=0;i<=7;i++){
        EUSCI_A3->TXBUF=buf_temp[i];
        while(EUSCI_A3->STATW&0x01);
    }
    memset(buf_temp,0x00,8);
}

/********displaychar*****************
// Fill the whole screen by drawing a 64x128 bitmap image.
// Inputs: p pointer to bitmap
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
 */
static void displaychar(uint8_t *p){
    uint8_t row = 0,col = 0;
    for(row = 0xb2; row < 0xb8; row++)
    {
        lcdcommandwrite(row);
        lcdcommandwrite(0x02);
        lcdcommandwrite(0x10);
        for(col = 0; col <45; col++)
            lcddatawrite(*p++);
    }
}

//********Display_char_JN*****************
//Show JiangNiu in English, this function shows all content
void Display_char_JN(void)
{
    uint8_t i;
    //JN_LCD_Clear_0();
    for(i=0;i<8;i++){
        display_char1106(12+i*8,1,i,Str_USB_08x16);
    }
}

//********JN_draw_log*****************
// Fill the whole screen by drawing a 64x128 bitmap image.
// Outputs: none
// Assumes: LCD is in default horizontal addressing mode (V = 0)
//Call this function to display the JN LOG
int JN_draw_log(void)
{
    JN_LCD_Clear_0();
    /*Display the  LOG*/
    displaychar(JN_Log);
    Display_char_JN();
    return 0;
}






