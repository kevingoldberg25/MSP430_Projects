#include <msp430.h>

void onboard_seg_display_init(void)
{
    PJSEL0 = BIT4 | BIT5;                   // For LFXT

    LCDCPCTL0 = 0xFFD0;     // Init. LCD segments 4, 6-15
    LCDCPCTL1 = 0xF83F;     // Init. LCD segments 16-21, 27-31
    LCDCPCTL2 = 0x00F8;     // Init. LCD segments 35-39

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure LFXT 32kHz crystal
    CSCTL0_H = CSKEY >> 8;                  // Unlock CS registers
    CSCTL4 &= ~LFXTOFF;                     // Enable LFXT
    do
    {
      CSCTL5 &= ~LFXTOFFG;                  // Clear LFXT fault flag
      SFRIFG1 &= ~OFIFG;
    }while (SFRIFG1 & OFIFG);               // Test oscillator fault flag
    CSCTL0_H = 0;                           // Lock CS registers

    // Initialize LCD_C
    // ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

    // VLCD generated internally,
    // V2-V4 generated internally, v5 to ground
    // Set VLCD voltage to 2.60v
    // Enable charge pump and select internal reference for it
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;

    LCDCCPCTL = LCDCPCLKSYNC;               // Clock synchronization enabled

    LCDCMEMCTL = LCDCLRM;                   // Clear LCD memory

    LCDCCTL0 |= LCDON;
}
void clearSeg(){
    LCDM10 = 0x00;
    LCDM6 = 0x00;
    LCDM4 = 0x00;
    LCDM19 = 0x00;
    LCDM15 = 0x00;
    LCDM8 = 0x00;
}

void delay(int loopTime) {
    volatile unsigned loops = loopTime; // Start the delay counter at 25,000
    while (--loops > 0);             // Count down until the delay counter reaches 0
}

void trueSegOn(int segNum){
    switch(segNum){
        case 0:
            LCDM10 = 0x80;
            break;
        case 1:
            LCDM6 = 0x80;
            break;
        case 2:
            LCDM4 = 0x80;
            break;
        case 3:
            LCDM19 = 0x80;
            break;
        case 4:
            LCDM15 = 0x80;
            break;
        case 5:
            LCDM8 = 0x80;
            break;
        case 6:
            LCDM8 = 0x40;
            break;
        case 7:
            LCDM8 = 0x20;
            break;
        case 8:
            LCDM8 = 0x10;
            break;
        case 9:
            LCDM15 = 0x10;
            break;
        case 10:
            LCDM19 = 0x10;
            break;
        case 11:
            LCDM4 = 0x10;
            break;
        case 12:
            LCDM6 = 0x10;
            break;
        case 13:
            LCDM10 = 0x10;
            break;
        case 14:
            LCDM10 = 0x08;
            break;
        case 15:
            LCDM10 = 0x04;
            break;
    }
}

void main(void) {

    int i = 0;
    int dire = 0;
    int speed = 50000;
    int segSize = 1;
    int k;

    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    onboard_seg_display_init();     // Init the LCD

    //init internal buttons
    P1REN |=  BIT1;
    P1OUT =   BIT1;

    // init. external buttons
    P2DIR &= ~(BIT0 | BIT1);
    P2REN |=  (BIT0 | BIT1);
    P2OUT |=  (BIT0 | BIT1);

    P2DIR &= ~(BIT2);
    P2REN |=  (BIT2);
    P2OUT |=  (BIT2);

    P2DIR &= ~(BIT3);
    P2REN |=  (BIT3);
    P2OUT |=  (BIT3);

    while(1)                        // loop continuously
    {
        //LCD + additional chains of snake + direction based
        if(segSize == 1)
            trueSegOn(i);
        else{
            if(dire <= 0){
                for(k = 0; k < segSize; k++){
                    if((i - k) < 0)
                        trueSegOn(16-k);
                    else
                        trueSegOn(i-k);

                }
            }

            else{
                for(k = 0; k < segSize; k++){
                    if((i + k) > 15)
                        trueSegOn(k);
                    else
                        trueSegOn(i+k);
                }
            }
        }

        delay(speed);
        // poll the inputs for speed
        if(!(P2IN & BIT0)){
            speed -= 5000;
        }
        if(!(P2IN & BIT1))
            speed += 5000;

        //poll built-in input for changing direction
        if(!(P1IN & BIT1))
            dire ^= 1;

        //poll input buttons for changing Snake length
        if(!(P2IN & BIT2)){
            segSize++;
            if(segSize > 15)
                segSize = 15;
        }

        if(!(P2IN & BIT3)){
            segSize--;
            if(segSize < 0)
                segSize = 0;
        }

        /*
         *
         * next segment
         * depending on direction
         *
         */
        if(dire == 0){            //checks if forward
            if(i & 16)          //if at end of snake, reset to start
                i = -1;
            i++;
        }
        else{            //if reverse
            if(i < 0)           //if reaches 0, it should go to segment 15
                i = 16;
            i--;
        }
        //delay(speed);
        clearSeg();
    }
}
