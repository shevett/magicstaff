#include <Adafruit_NeoPixel.h>

//------------------------------------------------------------
// MagicStaff.ino
//------------------------------------------------------------
// Ths is an Arduino sketch that is used to control a 'magic
// staff' - basically an array of 6 strips of WS2811/WS2812 
// strips mounted to a set of tubes and carried around as part
// of a costume.
//
// Full details are on my blog...
// http://planet-geek.com/category/hacks/magicstaff/
//------------------------------------------------------------

#define COUNT 52
#define TAIL 13
#define INPUT1 13
#define INPUT2 12

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip0 = Adafruit_NeoPixel(COUNT, 4, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(COUNT, 5, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(COUNT, 6, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(COUNT, 7, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(COUNT, 8, NEO_GRB + NEO_KHZ400);
Adafruit_NeoPixel strip5 = Adafruit_NeoPixel(COUNT, 9, NEO_GRB + NEO_KHZ400);

volatile int mode;
volatile int erased=0;
int lastpos = 0;
int ringarray[5] = {0,0,0,0,0} ;
uint32_t ringcolor[5];
int ringdirection[5]= {0,0,0,0,0};

Adafruit_NeoPixel strips[6] = {strip0, strip1, strip2, strip3, strip4, strip5} ;

//------------------------------------------
// Setup - initialize ALL THE THINGS...
//------------------------------------------

void setup() {
  // Initialize the strips...
  for (int i=0; i<6; i++) {
     strips[i].begin();
  }
  randomSeed(analogRead(0));
  pinMode(INPUT1, INPUT); 
  pinMode(INPUT2, INPUT);
  digitalWrite(INPUT1, HIGH);       // turn on pullup resistors
  digitalWrite(INPUT2, HIGH);       // turn on pullup resistors
  Serial.begin(9600);
  attachInterrupt(0, userbutton, RISING);
  mode=6;
}

//------------------------------------------
// userbutton - this is an ISR - Interrupt Service Routine - it's called
// whenever someone transitions int.0 RISING (see setup() for the def).  On 
// the Uno R3, this must be pin 3.
//------------------------------------------

void userbutton() {
  // This debounce routine snarfed from http://forum.arduino.cc/index.php/topic,45000.0.html
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
      mode = mode + 1;
      erased=0;
      if ( mode > 6 ) {
        mode = 0;
      }
      
      // Conditionals based on button input... 
      // If button 1 is held down, go right back to idle mode
      // If button 2 is held down, do ${something}
      // If both buttons are held down, do the FLASH animation, then go to the sparkly.
      if (digitalRead(INPUT1) == LOW && digitalRead(INPUT2) == LOW) {
        mode=7;
      } else {
        if (digitalRead(INPUT1) == LOW && digitalRead(INPUT2) == HIGH) {
          mode = 6;
        }
      }
//      Serial.println("Reading values from INPUT1 and INPUT2....");
//      Serial.println(digitalRead(INPUT1));
//      Serial.println(digitalRead(INPUT2));
  }
  last_interrupt_time = interrupt_time;
}
  
//------------------------------------------
// loop - Main iteration loop.  This should be in the tightest
// execution possible to allow for transitions.
//------------------------------------------

void loop() {
  int wait = 0;
  switch(mode) {
    case 0 : 
      for (int i=0; i<6; i++) {
        setRandom(strips[i]);
      }
      break;
    case 1:
      mode = mode + 1;    // Skip over this.  
      // brightStrobe();
      break;
    case 2:
      rainbowCycle(0);
      break;
    case 3:
      rings();
      break;
    case 4:
      twister();
      break;
    case 5:
      matrix();
      break;
    case 6:
      idler();
      break;
    case 7:
      flash();
      break;
  }
}

//------------------------------------------

void brightStrobe() {
  int cols[4][3] = { {255,255,255},{255,255,0},{255,0,255},{0,255,255} };
  uint32_t c = 0;
  int p = 0;
  for (int s=0; s<6; s++) {
      p = random(4);
      c = strip1.Color(cols[p][0],cols[p][1],cols[p][2]);
      for (int i=0; i<COUNT; i++) {
        strips[s].setPixelColor(i,c);
      }
      strips[s].show();
      if (mode != 1) {
        return;
      }
  }
  delay(25);
}
      
//------------------------------------------

void setRandom(Adafruit_NeoPixel s) {
  int cols[3] = {random(128),random(128),random(128)} ;
  cols[random(3)] = 0;
  uint32_t on = strip1.Color(cols[0],cols[1],cols[2]);
  uint32_t off = strip2.Color(0,0,0);
  long randNumber = random(COUNT);
  s.setPixelColor(randNumber,on);
  s.setPixelColor(random(COUNT),off);
  s.setPixelColor(random(COUNT),off);
  s.show();
}

//------------------------------------------

void rings() {

  if (erased==0) {
    erase();
  }
  
  for (int r=0; r<5; r++) {
    int rpos = ringarray[r];
    if (rpos > 0) {
      for (int s=0; s<6; s++) {
        strips[s].setPixelColor(rpos,ringcolor[r]);
        strips[s].setPixelColor(rpos-1,strip1.Color(0,0,0));
        strips[s].setPixelColor(rpos+1,strip1.Color(0,0,0));
        if (mode != 3) {
          return;
        }
      }
      ringarray[r] = ringarray[r] + ringdirection[r];
      if ((ringarray[r] > COUNT) || (ringarray[r] < 1)) {
        ringarray[r]=0;
        for (int s=0; s<6; s++) {
          strips[s].setPixelColor(1,strip1.Color(0,0,0));
        }
      }
    } else {
      if (random(50) == 1) {
        ringarray[r] = 1;
        int colarray[] = { random(255), random(255), random(255) } ;
        colarray[random(3)] = 0;
        ringcolor[r] = strip1.Color(colarray[0],colarray[1],colarray[2]);
        if (random(2) == 1) {
          ringdirection[r]=-1;
          ringarray[r]=COUNT-1;
        } else {
          ringdirection[r]=1;
        }
      }
    }
  }
  for (int s=0; s<6; s++) {
    strips[s].show();
  }
}

//------------------------------------------

void erase() {
  for (int s=0; s<6; s++) {
    for (int i=1; i<=COUNT; i++) {
      strips[s].setPixelColor(i,strip1.Color(0,0,0));
    }
    strips[s].show();
  }
  erased=1;
}

//------------------------------------------

void idler() {
  if (erased==0) {
    erase();
  }
  // First, come up with the color sweep array
  uint32_t c[6];
  for (int i=1; i<=6; i++) {
    c[i-1] = strip1.Color(i*2,i*2,i*2);
  }
  
  // Now load that into the strip...
  for (int i=0; i<6; i++) {
    int position = (lastpos + i) % 6;
    strips[position].setPixelColor(0,c[i]);
    strips[position].show();

    if (mode != 6) {
      return;
    }
  }
  delay(200);
  lastpos++;

}


//------------------------------------------

void twister() {
  if (erased==0) {
    erase();
  }
  
  int s = random(5);
  int p = 0;
  int cols[3] = {random(255),random(255),random(255)} ;
  cols[random(3)] = 0;

  while (p < COUNT + 11) {
    if (mode != 4) {
      return;
    }
    p++;
    if (p <= COUNT) {
      strips[s].setPixelColor(p,cols[0],cols[1],cols[2]);
    }
    s = s + 1;
    if (s > 5) { s = 0; }
    if (p > 10) {
      strips[s].setPixelColor(p - 17,0,0,0);
    }
    strips[s].show();
    delay(10);
  }
  erase();
}
    
//------------------------------------------

void matrix() {
  
  if (erased==0) {
    erase();
  }

  int glyph[10] = {0,0,0,0,0,0,0,0,0,0};
  int glyphstrip[10] = {0,0,0,0,0,0,0,0,0,0};
  
  while (true) {
    for (int i=0; i<10; i++) {
      delay(10);
      if (mode != 5) {
        return;
      }
      if (glyph[i] == 0) {
        // Should we switch this on?
        if (random(40) == 1) {
          glyph[i] = 1;
          glyphstrip[i] = random(6);
        }
      } else {
        // Okay, it's on, turn the glyph point white, draw the trails green back from it.
        boolean trailing = true;
        int startpoint = glyph[i];
        glyph[i] = startpoint + 1;
        strips[glyphstrip[i]].setPixelColor(startpoint,255,255,255);
        int l = 0;
        int tailvalue = 64;
        while (trailing) {
          startpoint = startpoint-1;
          if (startpoint < 0) {
            trailing = false;
          } else {
            tailvalue = tailvalue - (64/ TAIL);
            strips[glyphstrip[i]].setPixelColor(startpoint,0,tailvalue,0);
            l = l + 1;
            if (l == TAIL) {
              trailing = false;
              strips[glyphstrip[i]].setPixelColor(startpoint,0,0,0);
            }
          }
        }
        if (glyph[i] > COUNT + TAIL) {
          glyph[i] = 0;
        }
        strips[glyphstrip[i]].show();
      }
    }
  }
}
            
    

//------------------------------------------
void flash() {
  // This simply sets all the LED's bright white, which will be cleared by the random cycle.
  for(int i=0; i< COUNT; i++) {
    for (int s = 0; s< 6; s++) {
      strips[s].setPixelColor(i,255,255,255);
    }
  }
   
  for (int s=0; s<6; s++) {
    strips[s].show();
  }
  mode = 0;  // Jump to randomize.
}
      
//------------------------------------------


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j, s;

  for(j=0; j<256*5; j=j+7) { // 5 cycles of all colors on wheel
    for(i=0; i< COUNT; i++) {
      for (s=0; s<6; s++) {
        strips[s].setPixelColor(i, Wheel(((i * 256 / strip1.numPixels()) + j) & 255));
        if ( mode != 2) {
          return;
        }
      }
    }
    for (s=0; s<6; s++) {
      strips[s].show();
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip1.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip1.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip1.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

