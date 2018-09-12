#include <msp430.h>
#include <stdlib.h>
/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"

// Function Prototypes
void startupScreen();
void countdown();
void playSequence();
void resetGame();
void watchSequence();
void swDelay(unsigned int numLoops);

// Declare globals here

// Initialize state to 0
unsigned int state = 0;
unsigned int round = 0;
// Create all the game colors
#define NUM_ROUNDS 32
unsigned int colors[32];

// *** Intro Screen ***


// Main
void main(void) {
    WDTCTL = WDTPW | WDTHOLD;
    // Useful code starts here
    unsigned char currKey=0, dispSz = 3;
    unsigned char dispThree[3];

    initLeds();
    configDisplay();
    configKeypad();
    Graphics_clearDisplay(&g_sContext); // Clear the display
    // We are now done writing to the display.  However, if we stopped here, we would not
    // see any changes on the actual LCD.  This is because we need to send our changes
    // to the LCD, which then refreshes the display.
    // Since this is a slow operation, it is best to refresh (or "flush") only after
    // we are done drawing everything we need.
//    Graphics_flushBuffer(&g_sContext);

    while (1)    // Forever loop
    {
        unsigned char currKey = getKey();
        switch(state) {
            case 0:
                startupScreen(); // Show startup screen
                break;
            case 1:
                resetGame();
                countdown(); // Show the countdown
                break;
            case 2:
                playSequence(); // Play the sequence
                break;
            case 3:
                watchSequence(); // Watch for user input
                break;
        }

        // Check if any keys have been pressed on the 3x4 keypad

        if (currKey == '*')
            state = 1; // Go to state 1 (countdown)
    }  // end while (1)
}

void startupScreen() {
//    setLeds(0x01);
//    Graphics_clearDisplay(&g_sContext); // Clear the display
    Graphics_drawStringCentered(&g_sContext, "ECE2049 SIMON", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "Press * to start", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext);
}

void countdown() {
   Graphics_clearDisplay(&g_sContext); // Clear the display
   Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
   Graphics_flushBuffer(&g_sContext);
   swDelay(1000);
   Graphics_clearDisplay(&g_sContext);
   Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
   Graphics_flushBuffer(&g_sContext);
   swDelay(1000);
   Graphics_clearDisplay(&g_sContext);
   Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
   Graphics_flushBuffer(&g_sContext);
   swDelay(1000);
   Graphics_clearDisplay(&g_sContext);
   state = 2; // Set the state to 2 (playSequence)
}

void playSequence() {
    unsigned int delay = 1000-(1000*(3*round)/124); // Delay that speeds up
//    unsigned int delay = 1000;
    unsigned int i;
    for(i = 0; i <= round; i++) {
        switch(colors[i]) {
            case 0:
                setLeds(0x08);
                BuzzerOn(75);
                swDelay(delay);
                setLeds(0);
                BuzzerOff();
                swDelay(delay);
                break;
            case 1:
                setLeds(0x04);
                BuzzerOn(125);
                swDelay(delay);
                setLeds(0);
                BuzzerOff();
                swDelay(delay);
                break;
            case 2:
                setLeds(0x02);
                BuzzerOn(175);
                swDelay(delay);
                setLeds(0);
                BuzzerOff();
                swDelay(delay);
                break;
            case 3:
                setLeds(0x01);
                BuzzerOn(225);
                swDelay(delay);
                setLeds(0);
                BuzzerOff();
                swDelay(delay);
                break;
        }
    }
    state = 3; // Go to wait for input
}

void resetGame() {
    round = 0;
    unsigned int j;
    for (j = 0; j < 32; j++) {
        colors[j] = rand() % 4;
    }
}

void watchSequence() {
    unsigned int inputNum = 0;
    unsigned char currKey = getKey();
    Graphics_clearDisplay(&g_sContext); // Clear the display
    while(1) {
        currKey = getKey();
        unsigned int intKey = (currKey - '0') - 1;
        if(colors[inputNum] == intKey && currKey != 0) {
            unsigned char disp[1] = currKey;
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, disp, AUTO_STRING_LENGTH, (intKey*15)+20, 25, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            if(round == inputNum) {
                state = 2;
                round++;
                swDelay(1000);
                break;
            }
            inputNum++;
            while (1)
            {
                currKey = getKey();
                if (currKey == 0)
                {
                    break;
                }
            }
        }
        else if(currKey != 0 && colors[inputNum] != intKey) {
            state = 0;
            Graphics_clearDisplay(&g_sContext); // Clear the display
            Graphics_drawStringCentered(&g_sContext, "YOU LOSE", AUTO_STRING_LENGTH, 48, 25, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);
            swDelay(1000);
            Graphics_clearDisplay(&g_sContext);
            break;
        }
        if(currKey == '*') {
            state = 0;
            break;
        }
    }
}


void swDelay(unsigned int numLoops)
{
	// This function is a software delay. It performs
	// useless loops to waste a bit of time
	//
	// Input: numLoops = number of delay loops to execute
	// Output: none
	//
	// smj, ECE2049, 25 Aug 2013

	volatile unsigned int i,j;	// volatile to prevent removal in optimization
			                    // by compiler. Functionally this is useless code

	for (j=0; j<numLoops; j++)
    {
    	i = 50 ;					// SW Delay
   	    while (i > 0)				// could also have used while (i)
	       i--;
    }
}
