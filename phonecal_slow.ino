#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

/****************** Colors */
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ILI9341_LIGHTGREY   0xC618      
#define ILI9341_DARKGREY    0x7BEF 

/******************* UI details */
#define BUTTON_X 50
#define BUTTON_Y 100
#define BUTTON_W 85
#define BUTTON_H 50
#define BUTTON_SPACING_X 20
#define BUTTON_SPACING_Y 20
#define BUTTON_TEXTSIZE 2

#define TEXT_X 10
#define TEXT_Y 10
#define TEXT_W 300
#define TEXT_H 50
#define TEXT_TSIZE 3
#define TEXT_TCOLOR WHITE 
#define TEXT_LEN 12
char textfield[TEXT_LEN+1] = "";
uint8_t textfield_i=0;

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define P1_FUELSTAND A12
#define P2_FUELSTAND A13

//Touch For New ILI9341 TP
#define TS_MINX 100
#define TS_MAXX 920

#define TS_MINY 120
#define TS_MAXY 900

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Elegoo_GFX_Button buttons[15];

char buttonlabels[6][10] = {"Solo", "Duo", "Tunier", "Liter", "Stats", "Reset"};
uint16_t buttoncolors[6] = { ILI9341_LIGHTGREY,ILI9341_LIGHTGREY,ILI9341_LIGHTGREY,ILI9341_LIGHTGREY,ILI9341_LIGHTGREY,ILI9341_LIGHTGREY };
enum state {
  solo,
  duo,
  turnier,
  liter,
  stats,
  reset,
  main
};
state currentState = main;
                        
void setup(void) {
  Serial.begin(9600);
  Serial.println(F("TFT LCD test"));

#ifdef USE_Elegoo_SHIELD_PINOUT
  Serial.println(F("Using Elegoo 2.8\" TFT Arduino Shield Pinout"));
#else
  Serial.println(F("Using Elegoo 2.8\" TFT Breakout Board Pinout"));
#endif

  Serial.print("TFT size is "); Serial.print(tft.width()); Serial.print("x"); Serial.println(tft.height());



  tft.reset();

  uint16_t identifier = tft.readID();
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  }else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier==0x0101)
  {     
      identifier=0x9341;
       Serial.println(F("Found 0x9341 LCD driver"));
  }else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Elegoo 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_Elegoo_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Elegoo_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    identifier=0x9341;
   
  }

  tft.begin(identifier);
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  
  // create buttons
  for (uint8_t row=0; row<2; row++) {
    for (uint8_t col=0; col<3; col++) {
      buttons[col + row*3].initButton(&tft, BUTTON_X+col*(BUTTON_W+BUTTON_SPACING_X), 
                 BUTTON_Y+row*(BUTTON_H+BUTTON_SPACING_Y),    // x, y, w, h, outline, fill, text
                  BUTTON_W, BUTTON_H, WHITE, buttoncolors[col+row*3], WHITE,
                  buttonlabels[col + row*3], BUTTON_TEXTSIZE); 
      buttons[col + row*3].drawButton();
    }
  }
  
  // create 'text field'
  tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, WHITE);

}


#define MINPRESSURE 10
#define MAXPRESSURE 1000

//--------------------------------------------------------------------------------------------------------------------------------------------------------

void tft_loop() {
    digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);
  
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

   // Locates the pressurepoint on the ts
   if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    // scale from 0->1023 to tft.width
    p.x = (tft.height()- map(p.x, TS_MINY, TS_MAXY, tft.height(), 0));
    p.y = (tft.width()- map(p.y, TS_MINX, TS_MAXX, tft.width(), 0));
   }

  // gets hold of witch Button was pressed
  for (uint8_t b=0; b<6; b++) {
    if (buttons[b].contains(p.y, p.x)) {
      //Serial.print("Pressing: "); Serial.println(b);
      buttons[b].press(true);  // tell the button it is pressed
    } else {
      buttons[b].press(false);  // tell the button it is NOT pressed
    }
  }

  //redraws Button
  for (uint8_t b=0; b<6; b++) {
    if (buttons[b].justReleased()) {
      buttons[b].drawButton(); 
    }
    
    if (buttons[b].justPressed()) {
        buttons[b].drawButton(true);  

        writeState(b);

        
        
      //delay(10); // UI debouncing
    }
  }
}

void writeState(int b) {
  switch(b)
        {
          case(solo):
          {
            soloSetup();
          break;
          }
          case(duo):
          {
            duoSetup();
          break;
          }
          /*
          case(turnier):
          {
          break;
          }
          case(liter):
          {
            break;
          }
          case(stats):
          {
          break;
          }
          case(reset):
          {
          break;
          }
          */
          default:
          {
            currentState = main;
          }
        }
}

unsigned long p1Start;
enum {
  start,
  drinking,
  done
} soloState;

void soloSetup() {
  p1Start = millis();
  currentState = solo;
  soloState = drinking;
}

void duoSetup() {
  currentState = duo;  
}

double currentSolo;
void soloUpdate() {
  if (soloState == drinking) {
    double fuel = analogRead(P1_FUELSTAND);
    if (fuel > 40) {
      Serial.println(fuel);
      Serial.println("WINNER");
      soloState = done;
    } else {
      Serial.println(fuel);
      tft.setCursor(TEXT_X + 2, TEXT_Y+10);
      tft.setTextColor(TEXT_TCOLOR, BLACK);
      tft.setTextSize(TEXT_TSIZE);
      currentSolo = double((millis() - p1Start))/1000;
      tft.print(currentSolo);
    }
    
  }
  
}

void duoUpdate() {
  String fuel1 = String(analogRead(P1_FUELSTAND));
  String fuel2 = String(analogRead(P2_FUELSTAND));
  tft.setCursor(TEXT_X + 2, TEXT_Y+10);
  tft.setTextColor(TEXT_TCOLOR, BLACK);
  tft.setTextSize(TEXT_TSIZE);
  tft.setCursor(TEXT_X + 2, TEXT_Y+10);
  tft.print(fuel1 + " " + fuel2);
  
}

void mainUpdate() {
  tft.setCursor(TEXT_X + 2, TEXT_Y+10);
  tft.setTextColor(TEXT_TCOLOR, BLACK);
  tft.setTextSize(TEXT_TSIZE);
  tft.print("HI");
}

void loop(void) {

  tft_loop();
  switch (currentState) {
    case solo:
      soloUpdate();
      break;
    case duo:
      duoUpdate();
      break;
    default:
      mainUpdate();  
  }
  
}
