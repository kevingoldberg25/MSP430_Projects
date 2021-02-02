#include <msp430.h>
#include <time.h>
#include <stdlib.h>

/*
 * Initialize important variables
 */
int setSimon;                //determine currently checked of user input
int level;               //determines current level of the game + size of sequence
int count;               //current progress of game tracker
int game;
int simon[10];

void msp_init()
{
    WDTCTL = WDTPW | WDTHOLD;     // Stop watchdog timer

    P2DIR &= ~BIT0;               // Set pin P2.0 to be an input
    P2REN |=  BIT0;               // Enable internal pullup/pulldown resistor on P2.0
    P2OUT |=  BIT0;               // Pullup selected on P2.0
    P2IES |=  BIT0;               // Make P2.0 interrupt happen on the falling edge
    P2IFG &= ~BIT0;               // Clear the P2.0 interrupt flag
    P2IE  |=  BIT0;               // Enable P2.0 interrupt
    /*
     * ADDITIONAL BUTTON PORTS HERE
     */
    P2DIR &= ~BIT1;
    P2REN |=  BIT1;
    P2OUT |=  BIT1;
    P2IES |=  BIT1;
    P2IFG &= ~BIT1;
    P2IE  |=  BIT1;

    P2DIR &= ~BIT2;
    P2REN |=  BIT2;
    P2OUT |=  BIT2;
    P2IES |=  BIT2;
    P2IFG &= ~BIT2;
    P2IE  |=  BIT2;

    P2DIR &= ~BIT3;
    P2REN |=  BIT3;
    P2OUT |=  BIT3;
    P2IES |=  BIT3;
    P2IFG &= ~BIT3;
    P2IE  |=  BIT3;

    /*
     * Internal Button
     */
    P1DIR &= ~BIT1;
    P1REN |=  BIT1;
    P1OUT |=  BIT1;
    P1IES |=  BIT1;
    P1IFG &= ~BIT1;
    P1IE  |=  BIT1;

    /*
     * LED OUTPUT PORTS
     */
    P3DIR |=  (BIT0 | BIT1 | BIT2 | BIT3);

    PM5CTL0 &= ~LOCKLPM5;         // Unlock ports from power manager
    __enable_interrupt();         // Set global interrupt enable bit in SR register
}

void delay(volatile unsigned long loopTime) {
    volatile unsigned long loops = loopTime; // Start the delay counter at 25,000
    while (--loops > 0);             // Count down until the delay counter reaches 0
}

/*
 * Code for LCD HERE
 */

void clearSeg(){
    LCDM10 = 0xFC;
    LCDM6 = 0x00;
}

void trueSegOn(int segNum){
    switch(segNum){
        case 0:
            LCDM10 = 0xFC;
            break;
        case 1:
            LCDM10 = 0x60;
            break;
        case 2:
            LCDM10 = 0xDB;
            break;
        case 3:
            LCDM10 = 0xF3;
            break;
        case 4:
            LCDM10 = 0x67;
            break;
        case 5:
            LCDM10 = 0xB7;
            break;
        case 6:
            LCDM10 = 0xBF;
            break;
        case 7:
            LCDM10 = 0xE0;
            break;
        case 8:
            LCDM10 = 0xFF;
            break;
        case 9:
            LCDM10 = 0xF7;
            break;
        case 10:
            LCDM10 = 0x60;
            LCDM6 = 0xFC;
            break;
    }
}

void seg_init(){
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

/*
 * Interrupt Code HERE
 */
#pragma vector = PORT1_VECTOR
__interrupt void p1_IST()
{
    switch (P1IV)
    {
    case P1IV_NONE: break;
    case P1IV_P1IFG0: break;
    case P1IV_P1IFG1:
        game ^= 1;
        break;
    default: break;
    }
    //when mode is changed, all variables reset
    resetGame();
}

#pragma vector = PORT2_VECTOR
__interrupt void p2_ISR()
{
    switch (P2IV)
    {
        case P2IV_NONE: break;
        case P2IV_P2IFG0:
            P3OUT ^= BIT0;
            setSimon = 1;
            break;
        case P2IV_P2IFG1:
            P3OUT ^= BIT1;
            setSimon = 2;
            break;
        case P2IV_P2IFG2:
            P3OUT ^= BIT2;
            setSimon = 3;
            break;
        case P2IV_P2IFG3:
            P3OUT ^= BIT3;
            setSimon = 4;
            break;
        case P2IV_P2IFG4: break;
        case P2IV_P2IFG5: break;
        case P2IV_P2IFG6: break;
        case P2IV_P2IFG7: break;
        default:   break;
    }
    delay(50000);
    allOut();
    if(setSimon == simon[count])
        count += 1;
    else
        count = 11; //this value signifies that it is an error

}

void allOut(){
    P3OUT &= ~BIT0;
    P3OUT &= ~BIT1;
    P3OUT &= ~BIT2;
    P3OUT &= ~BIT3;
}

void allOn(){
    P3OUT ^= BIT0;
    P3OUT ^= BIT1;
    P3OUT ^= BIT2;
    P3OUT ^= BIT3;
}

void blinkSeq(int LED){
    switch(LED)
    {
        case 1:
            P3OUT ^= BIT0;
            break;
        case 2:
            P3OUT ^= BIT1;
            break;
        case 3:
            P3OUT ^= BIT2;
            break;
        case 4:
            P3OUT ^= BIT3;
            break;
        default:    break;
    }
    delay(25000);
    allOut();
}

void patternMake(int level){
     int randSimon = rand() % 4 + 1;     //random number between 0 - 4
     simon[level - 1] = randSimon;
     int j = 0;
     for(j; j < level; j++){
         delay(25000);
         blinkSeq(simon[j]);
     }
}

//Cleans up the array
void emptySet(){
    int reset = 0;
    for(reset; reset < 10; reset++){
        simon[reset] = 0;
    }
}

void resetGame(){
    setSimon = 0;
    level = 1;
    count = 0;
    //empty array and LCD
    emptySet();
    clearSeg();
}

void main(void)
{
    int mode;
    srand(time(NULL));
    msp_init();
    seg_init();

    game = 0;
    resetGame();


    while(1){
        //__bis_SR_register(LPM3_bits);  // Enter low power mode

        //MAIN GAME MODE
        mode = game;
        if(mode == 1){
            patternMake(level);
            while(count < level){
                if(game == 0)
                    break;
                else{}
            }

            //END OF SEQUENCE
            if(count == level){
                trueSegOn(level);
                level++;
                count = 0;
            }

            //LOSE CONDITION MET
            if(count == 11){
                allOn();
                delay(150000);
                allOut();
                delay(50000);
                //end game signaled
                count = 0;
                game = 0;
            }

            //WIN CONDITION MET
            if(level > 10){
                int k = 0;
                //mode change to signify end of game
                game = 0;
                //victory blinks
                for(k; k < 3; k++){
                    allOn();
                    delay(50000);
                    allOut();
                    delay(50000);
                }
            }

        }
    }
}