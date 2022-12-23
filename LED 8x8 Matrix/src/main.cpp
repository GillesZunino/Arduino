#include <Arduino.h>


#include <avr8-stubs-utilities.h>

#include <MAX7219LEDMatrix.h>



#define SPI_DIN 8
#define SPI_CS 9
#define SPI_CLK 10

MAX7219LEDMatrix ledMatrix_8x8(SPI_DIN, SPI_CS, SPI_CLK);


void setup() {
  INIT_DEBUGGER();

  ledMatrix_8x8.initialize();
  ledMatrix_8x8.shutdown(false);
  ledMatrix_8x8.clearDisplay();

  ledMatrix_8x8.setBrightness(1);
}


// ------------------------------------------------------------------------------------------

// Time to wait in between some of the update for better visual effect
unsigned long delaytime=100;


//
// Display the characters for the word "Arduino" one after the other on the matrix. 
// (Need at least a 5x7 leds display to see the whole chars)
//
void writeArduinoOnMatrix() {
  // Character data
  uint8_t a[5]={B01111110,B10001000,B10001000,B10001000,B01111110};

  uint8_t r[5]={B00111110,B00010000,B00100000,B00100000,B00010000};
  uint8_t d[5]={B00011100,B00100010,B00100010,B00010010,B11111110};
  uint8_t u[5]={B00111100,B00000010,B00000010,B00000100,B00111110};
  uint8_t i[5]={B00000000,B00100010,B10111110,B00000010,B00000000};
  uint8_t n[5]={B00111110,B00010000,B00100000,B00100000,B00011110};
  uint8_t o[5]={B00011100,B00100010,B00100010,B00100010,B00011100};

  // A
  ledMatrix_8x8.setColumn(1,a[0]);
  ledMatrix_8x8.setColumn(2,a[1]);
  ledMatrix_8x8.setColumn(3,a[2]);
  ledMatrix_8x8.setColumn(4,a[3]);
  ledMatrix_8x8.setColumn(5,a[4]);

  delay(delaytime);

  // r
  ledMatrix_8x8.setColumn(1,r[0]);
  ledMatrix_8x8.setColumn(2,r[1]);
  ledMatrix_8x8.setColumn(3,r[2]);
  ledMatrix_8x8.setColumn(4,r[3]);
  ledMatrix_8x8.setColumn(5,r[4]);

  delay(delaytime);

  // d
  ledMatrix_8x8.setColumn(1,d[0]);
  ledMatrix_8x8.setColumn(2,d[1]);
  ledMatrix_8x8.setColumn(3,d[2]);
  ledMatrix_8x8.setColumn(4,d[3]);
  ledMatrix_8x8.setColumn(5,d[4]);

  delay(delaytime);

  // u
  ledMatrix_8x8.setColumn(1,u[0]);
  ledMatrix_8x8.setColumn(2,u[1]);
  ledMatrix_8x8.setColumn(3,u[2]);
  ledMatrix_8x8.setColumn(4,u[3]);
  ledMatrix_8x8.setColumn(5,u[4]);

  delay(delaytime);

  // i
  ledMatrix_8x8.setColumn(1,i[0]);
  ledMatrix_8x8.setColumn(2,i[1]);
  ledMatrix_8x8.setColumn(3,i[2]);
  ledMatrix_8x8.setColumn(4,i[3]);
  ledMatrix_8x8.setColumn(5,i[4]);

  delay(delaytime);

  // n
  ledMatrix_8x8.setColumn(1,n[0]);
  ledMatrix_8x8.setColumn(2,n[1]);
  ledMatrix_8x8.setColumn(3,n[2]);
  ledMatrix_8x8.setColumn(4,n[3]);
  ledMatrix_8x8.setColumn(5,n[4]);

  delay(delaytime);

  // o
  ledMatrix_8x8.setColumn(1,o[0]);
  ledMatrix_8x8.setColumn(2,o[1]);
  ledMatrix_8x8.setColumn(3,o[2]);
  ledMatrix_8x8.setColumn(4,o[3]);
  ledMatrix_8x8.setColumn(5,o[4]);

  delay(delaytime);
  ledMatrix_8x8.clearDisplay();
}

//
// Light up a some Leds in a row. The pattern will be repeated on every row.
// The pattern will blink along with the row-number: row number 4 (index==3) will blink 4 times etc..
//
void rows() {
  for(int row=0;row<8;row++) {
    delay(delaytime);
    ledMatrix_8x8.setRow(row,B10100000);
    delay(delaytime);
    ledMatrix_8x8.setRow(row,(uint8_t)0);
    for(int i=0;i<row;i++) {
      delay(delaytime);
      ledMatrix_8x8.setRow(row,B10100000);
      delay(delaytime);
      ledMatrix_8x8.setRow(row,(uint8_t)0);
    }
  }
}

//
// Light up a some Leds in a column. The pattern will be repeated on every column.
// The pattern will blink along with the column-number: column number 4 (index==3) will blink 4 times etc...
//
void columns() {
  for(int col=0;col<8;col++) {
    delay(delaytime);
    ledMatrix_8x8.setColumn(col,B10100000);
    delay(delaytime);
    ledMatrix_8x8.setColumn(col,(uint8_t)0);
    for(int i=0;i<col;i++) {
      delay(delaytime);
      ledMatrix_8x8.setColumn(col,B10100000);
      delay(delaytime);
      ledMatrix_8x8.setColumn(col,(uint8_t)0);
    }
  }
}

//
// Light up every Led on the matrix.
// The led will blink along with the row-number: row number 4 (index==3) will blink 4 times etc.
//
void single() {
  for(int row=0;row<8;row++) {
    for(int col=0;col<8;col++) {
      delay(delaytime);
      ledMatrix_8x8.set(row,col,true);
      delay(delaytime);
      for(int i=0;i<col;i++) {
        ledMatrix_8x8.set(row,col,false);
        delay(delaytime);
        ledMatrix_8x8.set(row,col,true);
        delay(delaytime);
      }
    }
  }
}

// ------------------------------------------------------------------------------------------

void loop() {
  writeArduinoOnMatrix();
  rows();
  columns();
  single();
}