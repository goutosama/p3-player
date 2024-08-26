#include <SPI.h>
#include <Wire.h>

#include "animation.h"
#include "defines.h"
#include "structs.h"

void setup() {
  Serial.begin(9600); //debug Serial with PC
  for (uint8_t i = 0; i < 4; i++) {
    AnimQueue[i] = (struct AnimSeq*)malloc(sizeof(struct AnimSeq));
    AnimQueue[i]->renderFunc = NULL;
  }
  stat->trackName = "Burn My Dread";
  stat->PlayState = 3;
  stat->volume = 3;
  stat->battery = 4;
  stat->timer = 127; // test

  int16_t defaultOffset[2] = {0};
  // Default Adafruit code to check connection with display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  display.fillRect(0, 0, 128, 32, WHITE); // Draw white rectangle 128x32
  display.setTextColor(BLACK); 
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Menu testing snippet
  drawMenuCorner("MAIN");
  drawUpperOption("second", defaultOffset);
  drawCenterOption("center", defaultOffset, true);
  drawLowerOption("lower", defaultOffset);
  
  // Scrolling test
  //display.startscrolldiagright(0x0F, 0x0F);
  //display.startscrollright(0x00, 0x64);

  display.display(); // Wake up, boy


  // 60 hz timer
  cli();
  TCCR1A = 0x00; // Normal mode, => Disconnect Pin OC1 PWM Operation disabled
  TCCR1B = 0x02; // 16MHz clock with prescaler, TCNT1 increments every .5 uS (cs11 bit set)

  OCR1A = (unsigned int)(33333 * FPS); // = 16666 microseconds (each count is .5 us)
  TIMSK1 |= (1 << OCIE1A); // Enable interrupts
  sei();

  Anim_MenuCornerSlide();
}

void check(bool val){
  if (val){
    Serial.println('y');
  } else{
    Serial.println('n');
  }
}

ISR(TIMER1_COMPA_vect) {
  // Handle animation
  if (!isEmpty()) {
    for (uint8_t i = 0; i < 4; i++) {
      if (AnimQueue[i]->renderFunc != NULL) {
        AnimQueue[i]->renderFunc(AnimQueue[i]);
      }
    }
    sei();
    display.display();
  }

}

void loop() {
}