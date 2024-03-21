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


// Defines
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define FPS 30 //Recommended to be divisible by 5 and equal or less than 60

// Init values
/*
  int buttonNext = 2;   // кнопка следующий трек
  int buttonPause = 3;  // кнопка пауза/ пуск
  int buttonPrevious = 4; // кнопка предыдущий трек
  int buttonVolumeUp = 5; // кнопка увеличение громкости
  int buttonVolumeDown = 6; // кнопка уменьшение громкости
  boolean isPlaying = false; // статус воспроизведения/пауза
*/
int16_t globalOffset[2] = {0, 0};
uint8_t volume = 3;
uint8_t batt = 4;
uint8_t timer = 0; //test

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// Declaration for SoftwareSerial (for communication with player) and DFPLayerMini chip
//SoftwareSerial mySoftwareSerial(10, 11); // RX, TX для плеера DFPlayer Mini
//DFRobotDFPlayerMini myDFPlayer;

// Structs
struct AnimSeq { // A struct for intercepting every animation at the same time synced with 60hz timer
  void (*renderFunc)(struct AnimSeq *self);
  uint8_t step;
  uint8_t duration; // duration in 60 frames per second (interpolated, if FPS is set to less than that)
  int16_t offset[2];
};

struct MusicState { // A struct defining current state of playing track 
  char *trackName; //if not working try []
  byte PlayState; // That icon close to the timer
  uint16_t timer; // I need more digits!!!
  uint8_t volume;
  uint8_t battery;
} MusicState;


struct AnimSeq *AnimQueue[4];

void setup() {
  Serial.begin(9600); //debug Serial with PC

  for (uint8_t i = 0; i < 4; i++) {
    AnimQueue[i] = (struct AnimSeq*)malloc(sizeof(struct AnimSeq));
    AnimQueue[i]->renderFunc = NULL;
  }

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

  Serial.println(F("testing serial"));

  
  Serial.println(isEmpty());
  Serial.println(getFirstEmptyAnim());
  /*
  // Menu testing snippet
  
  // render corner with name
  display.fillTriangle(0,0, 0,32,16,32, BLACK);
  display.setRotation(3); //rotates text on OLED 1=90 degrees, 2=180 degrees
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(F("MAIN"));
  display.setRotation(0);

  uint8_t winX = 16;
  uint8_t winY = 10;
  uint8_t winWidth = 96;
  uint8_t winHeight = 11;

  // center option
  display.fillRect(winX, winY, winWidth, winHeight, BLACK);
  display.setCursor(winX + 2, winY + 2);
  display.print(F("First option"));

  // side options
  display.setTextColor(BLACK);

  display.setCursor(winX, winY - 8);
  display.print((char)0x1E);
  display.print(" ");
  display.print("Second option");

  display.setCursor(winX,winY + 12);
  display.print((char)0x1F);
  display.print(" ");
  display.print("Third option");
  */

  // Scrolling test
  //display.startscrolldiagright(0x0F, 0x0F);
  //display.startscrollright(0x00, 0x64);


  //display.display(); // Wake up, boy

  //Serial.println("starting");
  drawMediaBar(globalOffset);

  display.display();

  // 60 hz timer
  cli();
  TCCR1A = 0x00; // Normal mode, => Disconnect Pin OC1 PWM Operation disabled
  TCCR1B = 1 << CS11; // 16MHz clock with prescaler, TCNT1 increments every .5 uS (cs11 bit set)

  OCR1A = (unsigned int)(33333 * FPS); // = 16666 microseconds (each count is .5 us)
  TIMSK1 |= (1 << OCIE1A); // Enable interrupts
  sei();
  

  
  //Serial.println(freeMemory());
  //Serial.println(F("Updating"));
  delay(1000);
  Anim_SwitchSongs();
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
  }

}

void loop() {
  display.display();
}

bool isEmpty(){
  for (uint8_t i = 0; i < 4; i++) {
    if (AnimQueue[i]->renderFunc != NULL) {
      return false;
    }
  }
  return true;
}

uint8_t getFirstEmptyAnim() {
  // 4 means AnimQueue is full
  for (uint8_t i = 0; i < 4; i++) {
    if (AnimQueue[i]->renderFunc == NULL) {
      return i;
    }
  }
  return 4;
}

void PopAnim() {
  for (uint8_t i = 0; i < 4; i++) {
    if (AnimQueue[i]->duration < AnimQueue[i]->step) {
      AnimQueue[i]->renderFunc = NULL;
    }
  }
}

// animation internal functions
void renderAnim_SwipeOutSong(struct AnimSeq *self) {
  uint8_t travel = 32; //in pixels

  if (self->step > self->duration) {
    PopAnim();
    return;
  }

  self->offset[1] = map(self->step, 0, self->duration, 0, travel);
  drawMediaBar(self->offset);
  //Serial.print("OUT");
  //Serial.println(self->offset[1]);
  self->step++;
}

void renderAnim_SwipeInSong(struct AnimSeq *self) {
  uint8_t travel = 32; //in pixels

  if (self->step > self->duration) {
    PopAnim();
    return;
  }

  self->offset[1] = -map(self->step, 0, self->duration, travel, 0);
  drawMediaBar(self->offset);
  //Serial.print("IN");
  //Serial.println(self->offset[1]);
  self->step++;
}

void funcAnim_SwitchSongs(struct AnimSeq *self) {
  self->step++;
  if (self->step > self->duration) {
    Anim_SwipeInSong();
    PopAnim();
    return;
  }
  /*
  if (self->step = 13){
    //do a change text func
  }
  */
  if (self->step == 1) {
    Anim_SwipeOutSong();
  }

}

// animation methods
void Anim_SwitchSongs() {
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 12 + 6; // durations of SwipeIn and SwipeOut + delay
  uint8_t i = getFirstEmptyAnim();
  if (i < 4) {
    AnimQueue[i]->renderFunc = &funcAnim_SwitchSongs;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}

void Anim_SwipeOutSong() {
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 12;
  uint8_t i = getFirstEmptyAnim();
  if (i < 4) {
    AnimQueue[i]->renderFunc = &renderAnim_SwipeOutSong;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}

void Anim_SwipeInSong() {
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 12;
  uint8_t i = getFirstEmptyAnim();
  if (i < 4) {
    AnimQueue[i]->renderFunc = &renderAnim_SwipeInSong;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = -32;
  }
}

// Using built-in map() instead of this unfinished func
//uint8_t LerpInt(uint8_t travel,uint8_t step, uint8_t duration){
//  uint8_t diff = travel % duration;
//  if (diff == 0) {
//    return (32 / duration * step);
//  } else if (travel < duration) {

//  } else if (travel > duration){
//    if (step == duration){
//      return (32 / duration * step) + diff
//    } else {
//      return (32 / duration * step)
//    }
//  }
//}


// draw methods, todo: redrawing if already drawn smth

// very debug, maybe not so sure
void drawMediaBar(int16_t offset[2]){
  display.fillRect(0,0, 128, 32, WHITE);
  drawTrackName((char *)"Burn My Dread", offset);
  drawPlaybackState(0, offset);

  drawTrackTime(124, offset);

  drawVolume(volume, offset);
  drawBattery(batt, offset);
}

void drawTrackName(char * text, int16_t offset[2]) {
  display.setCursor(offset[0] + 2, offset[1] + 2); // Track name display
  display.setTextSize(1);
  display.write(text);
}

void drawPlaybackState(byte state, int16_t offset[2]) {
  // lets say
  // 0 - Play +
  // 1 - Pause - (is kinda weird, i should choose another char)
  // 2 - Stop +
  char stateChar = 0x21;
  switch (state) {
    case 0x00:
      stateChar = 0x10;
      break;
    case 0x01:
      stateChar = 0xC7;
      break;
    case 0x02:
      stateChar = 0xFE;
      break;
  }
  display.setCursor(offset[0] + 2, offset[1] + 32 - 16); // Current state display
  display.setTextSize(2);
  display.write(stateChar);
}

void drawTrackTime(uint16_t time, int16_t offset[2]) {
  byte minutes = time / 60;
  byte seconds = time % 60; // or time - minutes * 60
  if (minutes > 99) {
    minutes = 99;
  }

  char * mins = timeIntToChar(minutes);
  char * secs = timeIntToChar(seconds);

  mins[2] = '\0';  // bug fix: i seriously don't know wtf is going on, there's always garbage 3rd letter

  display.setCursor(offset[0] + 14, offset[1] + 32 - 16); // Time display
  display.setTextSize(2);
  display.write(mins);
  display.write(":");
  display.write(secs);
  free(mins);
  free(secs);
}

char* timeIntToChar(uint16_t time) {
  char * res = (char *) malloc (sizeof(char) * 2);
  char buf[3] = {(char)(time / 10 + '0'), (char)(time % 10 + '0'), '\0'};
  strcpy(res, buf);
  return res;
}

void drawVolume(uint8_t level, int16_t offset[2]) {
  // only from 0 to 5
  if (level > 5) {
    level = 5;
  }
  display.setCursor(offset[0] + 78, offset[1] + 32 - 10); // volume display
  display.setTextSize(1);
  display.write("VOL");
  uint8_t i = 0;
  for (; i < level; i++) {
    display.write(0xDB);
  }
}

void drawBattery(uint8_t level, int16_t offset[2]) {
  // only from 0 to 4
  if (level > 4) {
    level = 4;
  }
  uint8_t startPos = 128 - 21;
  display.setTextSize(1);
  uint8_t i = 0;
  for (; i < level; i++) {
    display.setCursor(offset[0] + startPos + i * 4, offset[1] + 2);
    display.write(0xDD);
  }
  display.setCursor(offset[0] + startPos + 16 - 2, offset[1] +  2);
  display.write(0xF9);
}


// Please dont look down here
/*
  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonNext, INPUT_PULLUP);
  pinMode(buttonPrevious, INPUT_PULLUP);
  pinMode(buttonVolumeUp, INPUT_PULLUP);
  pinMode(buttonVolumeDown, INPUT_PULLUP);
  mySoftwareSerial.begin(9600);
  Serial.begin(9600);
  delay(1000);
  Serial.println();
  Serial.println("DFPlayer Mini Demo");
  Serial.println("Initializing DFPlayer...");
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println("Unable to begin:");
    Serial.println("1.Please recheck the connection!");
    Serial.println("2.Please insert the SD card!");
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.setTimeOut(300);
  //----Set volume----
  myDFPlayer.volume(15); //Set volume value (0~30).
  //----Set different EQ----
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  myDFPlayer.play(1); //Воспроизведение первого mp3
  isPlaying = true; // воспроизводим
  Serial.println("Playing..");
  //----Читать информацию----
  Serial.println(myDFPlayer.readState()); //читать состояние mp3
  Serial.println(myDFPlayer.readVolume()); //Текущая громкость
  Serial.println(myDFPlayer.readEQ()); // читаем настройку эквалайзера
  Serial.println(myDFPlayer.readFileCounts()); // читать все файлы на SD-карте
  Serial.println(myDFPlayer.readCurrentFileNumber()); // текущий номер файла воспроизведения
  }

  void loop() {
  if (digitalRead(buttonPause) == LOW) {
    if (isPlaying) { // если было воспроизведение трека
      myDFPlayer.pause(); // пауза
      isPlaying = false; // пауза
      Serial.println("Paused..");
    } else {        // иначе
      isPlaying = true; // воспроизводим
      myDFPlayer.start(); //запускаем mp3 с паузы
    }
    delay(500);
  }
  if (digitalRead(buttonNext) == LOW) {
    if (isPlaying) {
      myDFPlayer.next(); //Next Song
      Serial.println("Next Song..");
    }
    delay(500);
  }
  if (digitalRead(buttonPrevious) == LOW) {
    if (isPlaying) {
      myDFPlayer.previous(); //Previous Song
      Serial.println("Previous Song..");
    }
    delay(500);
  }
  if (digitalRead(buttonVolumeUp) == LOW) {
    if (isPlaying) {
      myDFPlayer.volumeUp(); //Volume Up
      Serial.println("Volume Up..");
    }
    delay(500);
  }
  if (digitalRead(buttonVolumeDown) == LOW) {
    if (isPlaying) {
      myDFPlayer.volumeDown(); //Volume Down
      Serial.println("Volume Down..");
    }
    delay(500);
  }
  }
*/
