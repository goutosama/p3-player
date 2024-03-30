#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// библиотека для эмуляции Serial порта
#include <DFMiniMp3.h>
#include <SoftwareSerial.h>
// создаём объект mySoftwareSerial и передаём номера управляющих пинов RX и TX
// RX - цифровой вывод 10, необходимо соединить с выводом TX дисплея
// TX - цифровой вывод 11, необходимо соединить с выводом RX дисплея

#include "animation.h"
#include "defines.h"
#include "structs.h"

// Init values
/*
  int buttonNext = 2;   // кнопка следующий трек
  int buttonPause = 3;  // кнопка пауза/ пуск
  int buttonPrevious = 4; // кнопка предыдущий трек
  int buttonVolumeUp = 5; // кнопка увеличение громкости
  int buttonVolumeDown = 6; // кнопка уменьшение громкости
  boolean isPlaying = false; // статус воспроизведения/пауза
*/

// Declaration for SoftwareSerial (for communication with player) and DFPLayerMini chip
//SoftwareSerial mySoftwareSerial(10, 11); // RX, TX для плеера DFPlayer Mini
//DFRobotDFPlayerMini myDFPlayer;


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