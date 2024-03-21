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
  void (*renderFunc)(struct AnimSeq *self); // A function that executes every frame of animation. The whole AnimSeq object is considered empty if this pointer equals NULL. @note However, calling this function when it's NULL may cause instant crash, be careful.
  uint8_t step; // Current frame of animation (gets updated with every frame)
  uint8_t duration; // Duration in 60 frames per second (interpolated, if FPS is set to less than that)
  int16_t offset[2]; // Should be used to offset drawing current animation if whole object moves
};

struct MusicState { // A struct defining current state of playing track 
  char *trackName; // Name of the track currently playing (if not working try [])
  byte PlayState; // That icon close to the timer, shows the status of playing (Play, Pause, Stop)
  uint16_t timer; // A timer of current song. Note that it uses uint16 (I need more digits!!!)
  uint8_t volume; // Current volume of player from 0 to 5
  uint8_t battery; // Current battery level of player from 0 to 4
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

  // Menu testing snippet
  drawMenuCorner("MAIN");
  drawUpperOption("second");
  drawCenterOption("center");
  drawLowerOption("lower");

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

  //Anim_SwitchSongs();
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

/*!
    @brief  Function that returns 0 when there's no animations in the queue and 1 elsewhere
    @return Returns bool value
            0 - no animations in the queue
            1 - at least one animation is in the queue
*/
bool isEmpty() { // 
  for (uint8_t i = 0; i < 4; i++) {
    if (AnimQueue[i]->renderFunc != NULL) {
      return false;
    }
  }
  return true;
}

/*!
    @brief  Function that returns first empty animation ready to be overwritten
    @return Returns uint8 number with an index of animation between 0 and 3, and 4 if whole queue is full
*/
uint8_t getFirstEmptyAnim() {
  // 4 means AnimQueue is full
  for (uint8_t i = 0; i < 4; i++) {
    if (AnimQueue[i]->renderFunc == NULL) {
      return i;
    }
  }
  return 4;
}

/*!
    @brief  This function *deletes* every animation in the queue that is already finished
*/
void PopAnim() {
  for (uint8_t i = 0; i < 4; i++) {
    if (AnimQueue[i]->duration < AnimQueue[i]->step) {
      AnimQueue[i]->renderFunc = NULL;
    }
  }
}

// Animation internal functions

// Render function for Menu Corner sliding across the screen on menu open
void renderAnim_MenuCornerSlide(struct AnimSeq *self) {
  uint8_t travel = 112; //in pixels

  if (self->step > self->duration) {
    PopAnim();
    return;
  }

  self->offset[1] = map(self->step, 0, self->duration, 0, travel);

  self->step++;
}

// Render function for swiping out the current song
void renderAnim_SwipeOutSong(struct AnimSeq *self) {
  uint8_t travel = 32; //in pixels

  if (self->step > self->duration) {
    PopAnim();
    return;
  }

  self->offset[1] = map(self->step, 0, self->duration, 0, travel);
  drawMediaBar(self->offset);
  self->step++;
}

// Render function for swiping in the next song
void renderAnim_SwipeInSong(struct AnimSeq *self) {
  uint8_t travel = 32; //in pixels

  if (self->step > self->duration) {
    PopAnim();
    return;
  }

  self->offset[1] = -map(self->step, 0, self->duration, travel, 0);
  drawMediaBar(self->offset);
  self->step++;
}

/*!
    @brief  Function for a "functional" animation which launches animation to switch songs in and out.
    @note    There's no function to actually switch current song to another one
*/
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

// Function that initiates the animation for Menu Corner sliding across the screen on menu open
void Anim_MenuCornerSlide() {
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 12; // durations of SwipeIn and SwipeOut + delay
  uint8_t i = getFirstEmptyAnim();
  if (i < 4) {
    AnimQueue[i]->renderFunc = &renderAnim_MenuCornerSlide;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}

// Function that initiates the animation which launches animation to switch songs in and out.
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

// Function that initiates the animation for swiping out the current song
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

// Function that initiates the animation for swiping in the next song
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

// draw methods, todo: redrawing if already drawn smth

  // options
  // kinda need to move this to define or const
const uint8_t winX = 16;
const uint8_t winY = 10;
const uint8_t winWidth = 96;
const uint8_t winHeight = 11;

void drawCenterOption(char * text){
  // center option
  display.setTextColor(WHITE);
  display.fillRect(winX, winY, winWidth, winHeight, BLACK);
  display.setCursor(winX + 2, winY + 2);
  display.print(text);
}

void drawUpperOption(char * text){
  display.setTextColor(BLACK);
  display.setCursor(winX, winY - 8);
  display.print((char)0x1E);
  display.print(" ");
  display.print(text);
}

void drawLowerOption(char * text){
  display.setTextColor(BLACK);
  display.setCursor(winX,winY + 12);
  display.print((char)0x1F);
  display.print(" ");
  display.print(text);
}

void drawMenuCorner(char * text) {
  display.fillTriangle(0, 0, 0, 32, 16, 32, BLACK);
  display.setRotation(3); //rotates text on OLED 1=90 degrees, 2=180 degrees
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(text);
  display.setRotation(0);
}


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
