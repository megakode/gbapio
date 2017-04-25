#include <wiringPi.h>
#include <stdio.h>
#include <stdint.h>
#include <ncurses.h>

const int CS2 = 8;
const int WR = 29;
const int RD = 9;
const int CS = 31;
const int ADRPINS[24] = {0,1,2,3,4,5,6,7,28,30,10,11,12,13,14,15,16,21,22,23,24,25,26,27};
const int DATAPINS[16] = {0,1,2,3,4,5,6,7,28,30,10,11,12,13,14,15};

#define COLOR_CURSOR 1
#define COLOR_BIT_ON 2
#define COLOR_MENU 3

void printBinaryAt(int y, int x,int value);

int switchBits(int value, int bitFrom, int bitTo);

void setAddress(int adr);

int readInt();

int main (void)
{
    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, TRUE);  /* enable keyboard mapping */
   // (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();         /* echo input - in color */
    start_color();
    init_pair(COLOR_CURSOR, COLOR_CYAN,COLOR_BLUE);
    init_pair(COLOR_BIT_ON, COLOR_GREEN,COLOR_BLACK);
    init_pair(COLOR_MENU,COLOR_YELLOW,COLOR_BLUE);
    curs_set(0);

    wiringPiSetup();

    pinMode(CS2, OUTPUT);
    pinMode(CS,OUTPUT);
    pinMode(WR,OUTPUT);
    pinMode(RD,OUTPUT);
    
    // CS = set LOW : Latch address
    // RD = strobe LOW : Read 16 bits of data (data is valid on rising edge of RD)

    digitalWrite(CS, 1);
    digitalWrite(CS2,1);
    digitalWrite(WR,1);
    digitalWrite(RD,1);

    for(int i = 0 ; i < 24 ; i++ ){
	pinMode(ADRPINS[i],OUTPUT);
    }

    setAddress(0);
    digitalWrite(CS,0);  // CS=low - latch address   

    char c = 0;
    int on = 1;
    int bit = 0;
    int adr = 0;
    for(;;)
    {

        // --- Print menu ----
        move(LINES-1,0);
        hline(' ' | COLOR_PAIR(COLOR_MENU),COLS);
        attron(COLOR_PAIR(COLOR_MENU));
        printw(" (Q)uit - [SPACE] Toggle bit - [ARROWS] Select bit - (C)lear bits" );
        attroff(COLOR_PAIR(COLOR_MENU));
        // ------------------

        mvprintw(2,2,"Address: %9.u",adr);

        move(3,2);hline('-',24);

        move(4,2);
        // Print ADR bits
        for(int i = 23 ; i >= 0 ; i--){

            bool isSelected = i==bit;
            bool isBitOn = (1<<i) & adr;

            if(isSelected) attron(COLOR_PAIR(COLOR_CURSOR)); // selection
            if(isBitOn&&!isSelected)attron(COLOR_PAIR(COLOR_BIT_ON));
            addch((isBitOn ? '1' : '0') ); 
            if(isBitOn&&!isSelected)attroff(COLOR_PAIR(COLOR_BIT_ON));
            if(isSelected) attroff(COLOR_PAIR(COLOR_CURSOR)); // selection
        }

        printw(" (a%u)",bit);

        move(5,2);hline('|',24);
        move(6,2);printw("222211111111119876543210"); // Draw ADR pin numbers
        move(7,2);printw("32109876543210"); // Draw ADR pin numbers

        refresh();

        int c = getch();     /* refresh, accept single keystroke of input */

        if(c == 'w'){
            
            for(int i = 0 ; i < 16 ; i++){
                pinMode(DATAPINS[i],OUTPUT);
            }
            setAddress(0x20); // Når jeg læser fra 0x20 får jeg data fra 0x40, det er en bit shifted up i addresse?!
            digitalWrite(CS,1);   
            digitalWrite(CS,0);  // CS=low - latch address
/*
            setAddress(switchBits(0x98,0,1));
            digitalWrite(WR,0);
            delay(1);
            digitalWrite(WR,1);
            mvprintw(0,2,"CFI Query mode started");
            */
            for(int i = 0 ; i < 3 ; i++){
                int value = readInt();
                int switchedValue = switchBits(value,0,1);
                //printBinaryAt(10+i*2,2,value);
                printBinaryAt(11+i*2,2,switchedValue);
            }
            
        }

        if(c == 'r'){

            int value = readInt();
            printBinaryAt(10,2,value);

            mvprintw(0,2,"Read performed");
            refresh();
        }

        if(c == KEY_LEFT ){ // Left
            if(bit<23)bit+=1;
        }

        if(c == KEY_RIGHT){
            if(bit>0)bit-=1;
        }

        if(c == KEY_UP){
            adr = adr<<1;
        }

        if(c == KEY_DOWN){
            adr = adr>>1;
        }

        if(c == ' '){
            adr ^= 1<<bit;
        }

        if(c == 'c'){
            adr = 0;    
        }

        if(c == 'q' ){
            curs_set(1);
            endwin();
            return 0;
        }

        // Latch Address
        if(c=='a'){
            setAddress(adr);
            digitalWrite(CS,1);   
            digitalWrite(CS,0);  // CS=low - latch address
            mvprintw(0,2,"Address latched");
        }



    }
   
/*
    for(int i = 0 ; i < 16 ; i++){
	pinMode(DATAPINS[i],INPUT);
	pullUpDnControl(DATAPINS[i],PUD_OFF);
    }

    FILE *f = fopen("./output.gba","wb+");

    for(int numBytesRead = 0 ; numBytesRead < 32 ; numBytesRead++){

	uint16_t readInt = 0;
	readInt = 0;
	
	delay(1);
	digitalWrite(RD,0);
	delay(1);
	digitalWrite(RD,1);
	delay(1);

	for(int i = 15 ; i >= 0 ; i--)
	{
	    int  readBit = digitalRead(DATAPINS[i]);
	    if(readBit) readInt = readInt | (1<<i);
	}

	printBinary(readInt);

	fwrite(&readInt,1,2,f);    

    }

    fclose(f);

    return 0 ;
*/
}


// ***************************************************
// Prints an integer value as binary and hex
// ***************************************************

void printBinaryAt(int y, int x,int value)
{
    move(y,x);
    printw("Data: ");
    for(int i = 15 ; i >= 0 ; i--)
    {	if(i==7)printw(" ");
	    printw((value & (1<<i))? "1":"0");
    }

    printw(" %02x %02x", value&0xff, (value>>8)&0xff,value);
    refresh();

};

// ***************************************************
// Set address on ADR port
// ***************************************************

/*
Switches bit number bitFrom (0..x) with bitTo (0..x) in the value given.
Returns an integer with the two bits switched.
*/
int switchBits(int value, int bitFrom, int bitTo)
{
    int bitFromValue = ( (1<<bitFrom) & value ) ? 1 : 0;
    int bitToValue = ( (1<<bitTo) & value ) ? 1 : 0;

    value &= ~( (1 << bitFrom) | (1 << bitTo) );
    value |= ( (bitToValue << bitFrom) | (bitFromValue << bitTo) ); 

    return value;
}

void setAddress(int adr)
{
    for(int i=0;i<24;i++){
	    digitalWrite(ADRPINS[i],(adr&(1<<i)) ? 1:0 );
    }
} 

int readInt(){
    
    // Set DATA pins to INPUT
    for(int i = 0 ; i < 16 ; i++){
    pinMode(DATAPINS[i],INPUT);
    pullUpDnControl(DATAPINS[i],PUD_OFF);
    }
    // Latch a read
    digitalWrite(RD,0);
    delay(1);
    digitalWrite(RD,1);

    uint16_t readBit = 0;
    int value = 0;
    
    // Get bits from GPIO pins
    for(int i = 15 ; i >= 0 ; i--)
    {
        int  readBit = digitalRead(DATAPINS[i]);
        if(readBit) value = value | (1<<i);
    }

    return value;

}