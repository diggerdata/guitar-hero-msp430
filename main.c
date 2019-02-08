#include <msp430.h>
#include "peripherals.h"

//Function Prototypes
void swDelay(char numLoops);
void configButton(void);
unsigned char checkButton(void);
void configUserLED(char inbits);
void runtimerA2(void);
void playLED(int notes);
void a2delay(float numLoops);
int noteConverter(int notes);
//void songPlayer(void);


//Declare globals here
const unsigned int D4 = 294;
const unsigned int E4 = 311;
const unsigned int F4s = 370;
const unsigned int G4 = 392;
const unsigned int A4 = 440;
const unsigned int B4 = 494;
const unsigned int C5 = 523;
const unsigned int D5 = 587;
const unsigned int E5 = 659;
const unsigned int F5 = 698;
const unsigned int F5s = 740;
const unsigned int G5 = 784;
const unsigned int B5 = 988;
const unsigned int rest = 0;

unsigned volatile int i = 0;
int button; //
int currentNote;
int correctButton;
unsigned char state = 0;
long int timer_cnt = 0;
char tdir = 1;
unsigned int nth = 0; //nth notes
float sumDuration;
unsigned int combo = 0;
char isButtonPressed = 0;//a flag to separate notes, so that one note will only have one responding notes
int player_array[177];// in total, there are 177 notes, and we leave an empty element in the end






/* in binary:
 * 1 = 0001
 * 2 = 0010
 * 3 = 0011
 * 4 = 0100
 * 5 = 0101
 * 6 = 0110
 * 7 = 0111
 * 8 = 1000
 * 9 = 1001
 * 10 = 1010
 * 11 = 1011
 * 12 = 1100
 * 13 = 1101
 * 14 = 1110
 * 15 = 1111
 */


//https://musescore.com/user/180822/scores/172191
//https://pages.mtu.edu/~suits/notefreqs.html

unsigned int song_notes[] = {G5,F5s,B5, E5,D5,G5, C5,B4,E5, A4,D5,D5, G5,D5,G5,F5s,D5,F5s, G5,D5,F5s,G5,F5s,E5,D5,
                    E5,C5,F5s,G5,F5s,E5,D5,C5,D5,E5,F5s,G5,D5,G5,F5s,D5,F5s,G5,D5,F5s,G5,F5s,E5,D5,E5,C5,F5s,G5,F5s,E5,D5,
                    C5,D5,E5,F5s,G5,D5,G5,F5s,D5,F5s,G5,D5,F5s,G5,F5s,E5,D5,E5,C5,F5s,G5,F5s,E5,D5,C5,D5,E5,F5s,
                    G5,D5,G5,F5s,D5,F5s,G5,D5,F5s,G5,F5s,E5,D5,E5,C5,F5s,G5,F5s,E5,D5,C5,D5,E4,D4,E4,F4s,E4,F4s,G4,E5,
                    D5,B4,C5,E5,D5,B4,C5,B4,A4,G4,D5,A4,A4,G4,A4,A4,G4,A4,A4,G4,A4,B4,A4,G4,F4s,G4,E4,A4,B4,
                    A4,G4,F4s,D4,rest,A4,A4,G4,A4,A4,G4,A4,A4,G4,A4,B4,A4,G4,F4s,G4,E5,A4,B4,A4,G4,F4s,D4
}; //The notes

float song_duration[] = {2,1,1, 2,1,1, 2,1,1, 2,1.5,0.5, 0.75,0.75,0.5,0.75,0.75,0.5, 0.75,0.75,0.5,0.5,0.5,0.5,0.5,
                       0.25,1,0.5,0.75,0.5,0.5,0.5, 2,1.5,0.25,0.25, 0.75,0.75,0.5,0.75,0.75,0.5, 0.75,0.75,0.5,0.5,0.5,0.5,0.5, 0.25,1,0.5,0.75,0.5,0.5,0.5,
                       2,1.5,0.25,0.25, 0.75,0.75,0.5,0.75,0.75,0.5, 0.75,0.75,0.5,0.5,0.5,0.5,0.5, 0.25,1,0.5,0.75,0.5,0.5,0.5, 2,1.5,0.25,0.25,
                       0.75,0.75,0.5,0.75,0.75,0.5, 0.75,0.75,0.5,0.5,0.5,0.5,0.5, 0.25,1,0.5,0.75,0.5,0.5,0.5, 2,2, 1,0.25,0.25,1,0.25,0.25,0.5,0.5,
                       0.5,0.5,1.5,0.5,1, 0.5,1.5,0.5,1,0.5, 1.5,0.25,0.75,1,0.25,0.75, 1,0.25,0.75,1,0.5, 0.5,0.5,0.5,0.25,0.5,1,0.5,0.5,
                       0.5,0.5,0.5,1.5,0.25,0.25,0.75, 1,0.25,0.75,1,0.25,0.75, 1,0.5,0.5,0.5,0.5,0.25,0.5, 1,0.5,0.5,0.5,0.5,0.5,3.25
};//linked notes are written together, and we prolong the first nodes and ignore the second notes, 123 notes, lasting for 1:24. Remember to multiple factor 0.68.

/*
'1' is the corresponding button or LED.
D4 = 1000 = 8
E4 = 0100 = 4
F4s = 0010 = 2
G4 = 0001 =   1
A4 = 1000 = 8
B4 = 0100 = 4
C5 = 0010 = 2
D5 = 0001 = 1
E5 = 1000 =  8
F5s = 0100 = 4
G5 = 0010 = 2
B5 = 0001 = 1
rest = 0000 = 0
 */


//playnotes and notes would be corresponding.



//Main

void main(void)
{
    //Define some local variables
    unsigned char currKey = 1;

    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
    // You can then configure it properly, if desired

    // Useful code starts here
    initLeds();

    configDisplay();
    configKeypad();
    configButton();
    checkButton();

    _BIS_SR(GIE); // global interrupt enable

    unsigned int score = 0;


    while (1){
        switch (state){

            case 0://Welcome

                runtimerA2();

                Graphics_drawStringCentered(&g_sContext, "Guitar Hero", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "Press * to start", AUTO_STRING_LENGTH, 48, 30, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);//flush the buffer, so that the LCD will update the change
                currKey = getKey();
                if (currKey == '*'){
                    state = 1;
                    // switch to play sequence
                }

                break;

            case 1://3-2-1 countdown

                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 48, 30, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext); // update display
                setLeds(2);//LED 3 on

                // swDelay(10);//The number will appear for .5 second

                a2delay(200);//the resolution is the resolution is 0.01s/bit.
                setLeds(0);

                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 48, 30, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext); // update display
                setLeds(4);//LED2
                //swDelay(10);
                a2delay(200);
                setLeds(0);

                Graphics_clearDisplay(&g_sContext); // Clear the display
                Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 48, 30, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext); // update display
                setLeds(8);//LED 1 on
                //swDelay(10);
                a2delay(200);
                setLeds(0);
                Graphics_clearDisplay(&g_sContext); // Clear the display

                setLeds(14);//all three LEDs on
                swDelay(10);
                setLeds(0);

                state = 2;//Test

                break;


            case 2://play the notes one by one
                //Test

                //create 2 arrays to store the notes and duration
                //create a function to convert the notes to corresponding frequency
                        currentNote = song_notes[nth];
                        BuzzerOn(currentNote);
                        playLED(currentNote);
                        //swDelay(6.8*song_duration[i]);//using another function

                        if (getKey() == '#'){
                            state = 0;
                            BuzzerOff();
                            setLeds(0);
                            nth = 0;
                            break;
                        }


                    nth++;
                    timer_cnt = 0;
                    state = 3;
                break;

            case 3: //check each button

                button = checkButton();
                correctButton = noteConverter(song_notes[nth - 1]);

                if (getKey() == '#'){
                            state = 0;
                            nth = 0;
                            BuzzerOff();
                            setLeds(0);
                            break;
                        }

                else if(timer_cnt >= 100*song_duration[nth -1]){ // too slow (AKA) no press (AKA) end of note
                    isButtonPressed = 0;
                    state = 2;

                    BuzzerOff();
                    setLeds(0);

                    break;
                }

                if(button != 0 && button != correctButton && isButtonPressed != 1){ // wrong press
                    isButtonPressed = 1;
                    combo = 0;
                }

                if(button == correctButton && isButtonPressed != 1){ // correct press, stay in the state3 until the time ends
                    combo ++;
                    score += 100 + combo*10;
                    isButtonPressed = 1;
                }

                if (nth == 178 || getKey() == '0'){
                    if (score <= 10000){
                        state = 4;
                        a2delay(500);
                    }
                    else if (score > 10000){
                        a2delay(500);
                        state = 5;
                    }
                }


                break;

            case 4://loss
                BuzzerOff();
                setLeds(0);
                Graphics_clearDisplay(&g_sContext);
                Graphics_drawStringCentered(&g_sContext, "Good!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, score, AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext); // update display
                //swDelay(30);
                a2delay(300);
                nth = 0;
                state = 0;
                score = 0;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;

            case 5://Victory
                BuzzerOff();
                setLeds(0);
                Graphics_clearDisplay(&g_sContext);
                Graphics_drawStringCentered(&g_sContext, "Awesome!! You made it!!", AUTO_STRING_LENGTH, 48, 15, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, score, AUTO_STRING_LENGTH, 48, 35, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext); // update display
                //swDelay(30);
                a2delay(300);
                nth = 0;
                state = 0;
                score = 0;
                Graphics_clearDisplay(&g_sContext); // Clear the display
                break;

        }
    }

}

void configButton(void){
    /*P7SEL &= ~BIT0;
    P3SEL &= ~BIT6;
    P2SEL &= ~BIT6;
    P7SEL &= ~BIT4;
    P7IN &= ~ BIT0;
    P3IN &= ~BIT6;
    P2IN &= ~BIT6;
    P7IN &= ~BIT4;*/

    P7SEL &= ~(BIT4|BIT0);
    P3SEL &= ~(BIT6);
    P2SEL &= ~(BIT2);

    P7DIR &= (BIT4|BIT0);
    P3DIR &= (BIT6);
    P2DIR &= (BIT2);

    P7REN |= (BIT4|BIT0);
    P3REN |= (BIT6);
    P2REN |= (BIT2);
}

unsigned char checkButton(void)
{
    unsigned int ret_val = 0;
    //S1 is P7.0 3.6 2.2 7.4

    if ((P7IN & BIT0)== 0)
        ret_val = 1;
    else if ((P3IN & BIT6)==0)
        ret_val = 2;
    else if ((P2IN & BIT2)==0)
        ret_val = 4;
    else if ((P7IN & BIT4)==0)
        ret_val = 8;

    //ret_val = (val1 | val2 | val3 | val4);
    return(ret_val);
}

void configUserLED(char inbits){
    //user led =

    if (inbits == '1'){
        setLeds(1);
    }
    else if (inbits == '2'){
        setLeds(2);
    }
    else if (inbits == '3'){
        setLeds(3);
    }
    else if (inbits == '4'){
        setLeds(4); // update display
    }
}

void a2delay(float numLoops){
    timer_cnt = 0; // our timer_cnt increments ~100 times per second
    while(timer_cnt < numLoops){
        //weee! we're waiting around and having fun :)
    }
}

void swDelay(char numLoops)
{
    // This function is a software delay. It performs
    // useless loops to waste a bit of time
    //
    // Input: numLoops = number of delay loops to execute
    // Output: none
    //
    // smj, ECE2049, 25 Aug 2013

    volatile unsigned int i,j;  // volatile to prevent removal in optimization
    // by compiler. Functionally this is useless code

    for (j=0; j<numLoops; j++)
    {
        i = 10000 ;                 // SW Delay (trying for ~.1 second delay per value of input
        while (i > 0)               // could also have used while (i)
            i--;
    }
}

/*
void led_on(inputVal){//display the number player presses on the screen.
    if (inputVal == 1){
        swDelay(1);
        setLeds(8); // update display
        }
    else if (inputVal == 2){
            swDelay(1);
            setLeds(4); // update display
        }
    else if (inputVal == 3){
            swDelay(1);
            setLeds(2); // update display
            }
    else if (inputVal == 4){
            swDelay(1);
            setLeds(1); // update display
            }
    else if (inputVal == 12){

    return 0;
}
*/


/*
int buttonCorrect?(int


     code here




   */

/*
//create 2 arrays to store the notes and duration
//create a function to convert the notes to corresponding frequency
void songPlayer(void){//play one note one time
    int i = 0;
    for (i = 0;i<177;i++){
        unsigned int currentNote = song_notes[i];
        BuzzerOn(currentNote);
        playLED(song_notes[i]);
        //swDelay(6.8*song_duration[i]);//using another function
        a2delay(100*song_duration[i]);
        BuzzerOff();
        setLeds(0);
        //swDelay(1);
        a2delay(5);
        if (getKey() == '#'){
            state = 0;
            i = 177;
        }
    }
    BuzzerOff();
}*/

void runtimerA2(void)
{ // From Users Guide Ch 17
  // Use ACLK (TASSEL_1), clock divider of 1 (ID_0)
  // and start timer counting in Up mode (MC_1)
  TA2CTL = TASSEL_1 + MC_1 + ID_0;    //
  TA2CCR0 = 163;     // 163 ACLK tics = ~1/200 seconds
  TA2CCTL0 = CCIE;      // TA2CCR0 interrupt enabled
}

//------------------------------------------------------------------------------
// TimerA2 Interrupt Service Routine
//------------------------------------------------------------------------------
/*void initTimerA2(void){
   /P5SEL |= (BIT5|BIT4|BIT3|BIT3|BIT2);
}*/

// Timer A2 interrupt service routine
 #pragma vector=TIMER2_A0_VECTOR
 __interrupt void TimerA2_ISR (void)
 {
 if (tdir)
   {
     timer_cnt++; // counting up
     if (timer_cnt == 60000)
      timer_cnt = 0;
     if (timer_cnt%100==0)  // blink LEDs once a second
     {
      P1OUT = P1OUT ^ BIT0;
            P4OUT ^= BIT7;
 } }
   else
     timer_cnt--;  // counting down
 }

 //play LED while playing the song
 void playLED(int notes){
     if (notes == D4 || notes == E5 || notes == A4){
         setLeds(8);
     }
     else if (notes == E4 || notes == B4|| notes == F5s)   {
         setLeds(4);
     }
     else if (notes == F4s || notes == C5 || notes == G5)   {
         setLeds(2);
     }
     else if (notes == G4 || notes == D5 || notes == B5)   {
         setLeds(1);
     }
     else {
         setLeds(0);
     }
 }

 //play LED while playing the song
 int noteConverter(int notes){
     int ret_val = 0;
     if (notes == D4 || notes == E5 || notes == A4){
         ret_val = 8;
     }
     else if (notes == E4 || notes == B4|| notes == F5s)   {
         ret_val = 4;
     }
     else if (notes == F4s || notes == C5 || notes == G5)   {
         ret_val = 2;
     }
     else if (notes == G4 || notes == D5 || notes == B5)   {
         ret_val = 1;
     }

     return ret_val;
 }
