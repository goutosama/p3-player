#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "structs.h"
#include "defines.h"
#include "bitmap.h"

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Defenitions

int16_t globalOffset[2] = {0, 0};
struct AnimSeq *AnimQueue[4];

/*!
    @brief  Function that returns 0 when there's no animations in the queue and 1 elsewhere
    @return Returns bool value
            0 - no animations in the queue
            1 - at least one animation is in the queue
*/
bool isEmpty()
{
  for (uint8_t i = 0; i < 4; i++)
  {
    if (AnimQueue[i]->renderFunc != NULL)
    {
      return false;
    }
  }
  return true;
}

/*!
    @brief  Function that returns first empty animation ready to be overwritten
    @return Returns uint8 number with an index of animation between 0 and 3, and 4 if whole queue is full
*/
uint8_t getFirstEmptyAnim()
{
  // 4 means AnimQueue is full
  for (uint8_t i = 0; i < 4; i++)
  {
    if (AnimQueue[i]->renderFunc == NULL)
    {
      return i;
    }
  }
  return 4;
}

/*!
    @brief  This function *deletes* every animation in the queue that is already finished
*/
void PopAnim()
{
  for (uint8_t i = 0; i < 4; i++)
  {
    if (AnimQueue[i]->duration < AnimQueue[i]->step)
    {
      AnimQueue[i]->renderFunc = NULL;
    }
  }
}

void initAnimations()
{
  // Creates Animation Queue. Length is defined in defines.h
  for (uint8_t i = 0; i < ANIM_QUEUE_LENGTH; i++)
  {
    AnimQueue[i] = (struct AnimSeq *)malloc(sizeof(struct AnimSeq));
    AnimQueue[i]->renderFunc = NULL;
  }

  // Default Adafruit code to check connection with display
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {                                                 // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed")); //                            What if there's no Serial?
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  display.fillRect(0, 0, 128, 32, WHITE); // Draw white rectangle 128x32
  display.setTextColor(BLACK);            // Default color for text
  display.cp437(true);                    // Use full 256 char 'Code Page 437' font

  display.display(); // Wake up, boy
}
// draw methods, todo: redrawing if already drawn smth

// options
// kinda need to move this to define or const
const uint8_t winX = 16;
const uint8_t winY = 10;
const uint8_t winWidth = 96;
const uint8_t winHeight = 11;

// WHY tf we need this bool var what does it change
void drawCenterOption(char * text, int16_t offset[2], bool isFullOffset) 
{
  // center option
  int8_t offX = winX;
  int8_t offY = winY;
  display.setTextColor(WHITE);
  if (isFullOffset)
  {
    offX += offset[0];
    offY += offset[1];
  }
  display.fillRect(offX, offY, winWidth, winHeight, BLACK);
  display.setCursor(winX + 2 + offset[0], winY + 2 + offset[1]);
  display.print(text);
}

void drawUpperOption(char *text, int16_t offset[2])
{
  display.setTextColor(BLACK);
  display.setCursor(winX, winY - 8);
  display.print((char)0x1E);
  display.print(" ");
  display.setCursor(winX + offset[0] + 2 * 6, winY - 8 + offset[1]);
  display.print(text);
}

void drawLowerOption(char *text, int16_t offset[2])
{
  display.setTextColor(BLACK);
  display.setCursor(winX, winY + 12);
  display.print((char)0x1F);
  display.print(" ");
  display.setCursor(winX + offset[0] + 2 * 6, winY + 12 + offset[1]);
  display.print(text);
}

void drawMenuCorner(char *text)
{
  display.fillTriangle(0, 0, 0, 32, 16, 32, BLACK);
  display.setRotation(3); // rotates text on OLED 1=90 degrees, 2=180 degrees
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(text);
  display.setRotation(0);
}

void drawTrackName(char *text, int16_t offset[2])
{
  display.setCursor(offset[0] + 2, offset[1] + 2); // Track name display
  display.setTextSize(1);
  display.write(text);
}

void drawPlaybackState(uint8_t state, int16_t offset[2])
{
  // lets say
  // 3 - Play (there's weird bug that starts with introducing a MusicState struct.
  //           Basically somewhere before that func 0 magically changes into 1 and therefore I made Play equal 3 instead of 0 (like a Persona 3 y'know))
  // 0 or 1 - Pause (is kinda weird, i should choose another char)
  // 2 - Stop
  char stateChar = 0x21;
  switch (state)
  {
  case 3:
    stateChar = 0x10;
    break;
  case 1:
    stateChar = 0xC7;
    break;
  case 2:
    stateChar = 0xFE;
    break;
  }
  display.setCursor(offset[0] + 2, offset[1] + 32 - 16); // Current state display
  display.setTextSize(2);
  display.write(stateChar);
}

char *timeIntToChar(uint16_t time)
{
  char *res = (char *)malloc(sizeof(char) * 2);
  char buf[3] = {(char)(time / 10 + '0'), (char)(time % 10 + '0'), '\0'};
  strcpy(res, buf);
  return res;
}

void drawTrackTime(uint16_t time, int16_t offset[2])
{
  byte minutes = time / 60;
  byte seconds = time % 60; // or time - minutes * 60
  if (minutes > 99)
  {
    minutes = 99;
  }

  char *mins = timeIntToChar(minutes);
  char *secs = timeIntToChar(seconds);

  mins[2] = '\0'; // bug fix: i seriously don't know wtf is going on, there's always garbage 3rd letter

  display.setCursor(offset[0] + 14, offset[1] + 32 - 16); // Time display
  display.setTextSize(2);
  display.write(mins);
  display.write(":");
  display.write(secs);
  free(mins);
  free(secs);
}

void drawVolume(uint8_t level, int16_t offset[2])
{
  // only from 0 to 5
  if (level > 5)
  {
    level = 5;
  }
  display.setCursor(offset[0] + 78, offset[1] + 32 - 10); // volume display
  display.setTextSize(1);
  display.write("VOL");
  uint8_t i = 0;
  for (; i < level; i++)
  {
    display.write(0xDB);
  }
}

void drawBattery(uint8_t level, int16_t offset[2])
{
  // only from 0 to 4
  if (level > 4)
  {
    level = 4;
  }
  uint8_t startPos = 128 - 21;
  display.setTextSize(1);
  uint8_t i = 0;
  for (; i < level; i++)
  {
    display.setCursor(offset[0] + startPos + i * 4, offset[1] + 2);
    display.write(0xDD);
  }
  display.setCursor(offset[0] + startPos + 16 - 2, offset[1] + 2);
  display.write(0xF9);
}

void drawPlayMode(PlayMode mode, int16_t offset[2])
{
  const int16_t start_x = 128 - 21 - 16;
  const int16_t start_y = 2;
  switch (mode)
  {
  case LoopFolder:
    display.drawBitmap(start_x + offset[0], start_y + offset[1], bitmap_icon_loop, 14, 11, BLACK);
    break;

  case Loop1:
    display.drawBitmap(start_x + offset[0], start_y + offset[1], bitmap_icon_loop, 14, 11, BLACK);
    display.drawBitmap(start_x + 5 + offset[0], start_y + 2 + offset[1], bitmap_icon_loop1, 3, 6, BLACK);
    break;

  case Shuffle:
    display.drawBitmap(start_x + offset[0], start_y + offset[1], bitmap_icon_shuffle, 14, 11, BLACK);
    break;

  default:
    display.setCursor(start_x + offset[0], start_y + offset[1]);
    display.write(0x21);
  }
}

// very debug, maybe not so sure
void drawMediaBar(int16_t offset[2], struct MusicState *stat)
{
  display.fillRect(0, 0, 128, 32, WHITE);
  drawTrackName(stat->trackName, offset);
  drawPlaybackState(stat->PlayState, offset);

  drawTrackTime(stat->timer, offset);

  drawVolume(stat->volume, offset);
  drawBattery(stat->battery, offset);

  drawPlayMode(stat->playMode, offset);
}

// Animation internal functions

// Render function for Menu Corner sliding across the screen on menu open @note WARNING: Unstable. Can cause crashes of whole display. That was when I did full refresh, but now circles still might be too much for arduino to handle (looks cooler and P3 like though)
void renderAnim_MenuCornerSlide(struct AnimSeq *self)
{
  uint8_t travel = 152; // in pixels

  if (self->step > self->duration)
  {
    // Serial.println("menu");
    self->offset[1] = 0;
    drawMenuCorner("MENU");
    drawUpperOption("First", self->offset);
    drawCenterOption("Second", self->offset, false);
    drawLowerOption("Third", self->offset);
    PopAnim();
    return;
  }

  self->offset[1] = map(self->step, 0, self->duration, 12, travel);
  display.fillRect(0, 0, 128, 32, WHITE);
  if (self->offset[1] <= 112)
  {
    display.drawCircle(112, 0, self->offset[1] + 1, BLACK);
    display.drawCircle(112, 0, self->offset[1], BLACK);
  }
  if (self->step > self->duration / 4)
  {
    display.drawCircle(32, 0, self->offset[1] - 50, BLACK);
  }
  self->step++;
}

// Render function for swiping down menu options
void renderAnim_SwipeDownMenu(struct AnimSeq *self)
{
  uint8_t travel = 11; // in pixels
  uint8_t travelX = 12;

  if (self->step > self->duration)
  {
    PopAnim();
    return;
  }

  display.fillRect(20, 0, 64, 32, WHITE); // fix drawing where not needed

  if (self->step < self->duration / 2)
  {
    self->offset[0] = map(self->step, 0, self->duration / 2, 0, travelX);
    self->offset[1] = map(self->step, 0, self->duration / 2, 0, travel);
    drawUpperOption("First", self->offset);
    drawCenterOption("Second", self->offset, false);
    drawLowerOption("Third", self->offset);
  }
  /*
  if (self->step = self->duration / 2){
    // change func
  }
  */
  if (self->step > self->duration / 2 - 2)
  {
    self->offset[0] = -map(self->step, self->duration / 2, self->duration, travelX, 0);
    self->offset[1] = -map(self->step, self->duration / 2, self->duration, travel, 0);
    drawUpperOption("Third", self->offset);
    drawCenterOption("First", self->offset, false);
    drawLowerOption("Second", self->offset);
  }

  display.fillRect(14, 10, 2, 11, WHITE); // fix drawing where not needed

  self->step++;
}

// Render function for swiping up menu options
void renderAnim_SwipeUpMenu(struct AnimSeq *self)
{
  uint8_t travel = 11; // in pixels
  uint8_t travelX = 12;

  if (self->step > self->duration)
  {
    PopAnim();
    return;
  }

  display.fillRect(20, 0, 64, 32, WHITE); // fix drawing where not needed

  if (self->step < self->duration / 2)
  {
    self->offset[0] = -map(self->step, 0, self->duration / 2, 0, travelX);
    self->offset[1] = -map(self->step, 0, self->duration / 2, 0, travel);
    drawUpperOption("First", self->offset);
    drawCenterOption("Second", self->offset, false);
    drawLowerOption("Third", self->offset);
  }
  /*
  if (self->step = self->duration / 2){
    // change func
  }
  */
  if (self->step > self->duration / 2 - 2)
  {
    self->offset[0] = map(self->step, self->duration / 2, self->duration, travelX, 0);
    self->offset[1] = map(self->step, self->duration / 2, self->duration, travel, 0);
    drawUpperOption("Third", self->offset);
    drawCenterOption("Second", self->offset, false);
    drawLowerOption("First", self->offset);
  }
  display.fillRect(14, 10, 2, 11, WHITE); // fix drawing where not needed
  display.fillRect(16, 0, 16, 2, WHITE);
  self->step++;
}

/*
void renderAnim_OpenMenuOption(struct AnimSeq *self) {
  uint8_t travel = 32; //in pixels

  if (self->step > self->duration) {
    PopAnim();
    return;
  }

  self->offset[1] = map(self->step, 0, self->duration, 0, travel);


  self->step++;
}*/

// Render function for swiping out the current song
void renderAnim_SwipeOutSong(struct AnimSeq *self)
{
  uint8_t travel = 32; // in pixels

  if (self->step > self->duration)
  {
    PopAnim();
    return;
  }

  self->offset[1] = map(self->step, 0, self->duration, 0, travel);
  drawMediaBar(self->offset, stat);
  self->step++;
}

// Render function for swiping in the next song
void renderAnim_SwipeInSong(struct AnimSeq *self)
{
  uint8_t travel = 32; // in pixels

  if (self->step > self->duration)
  {
    PopAnim();
    return;
  }

  self->offset[1] = -map(self->step, 0, self->duration, travel, 0);
  drawMediaBar(self->offset, stat);
  self->step++;
}

// animation methods

/* Only started
void Anim_OpenMenuOption() {
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 10;
  uint8_t i = getFirstEmptyAnim();
  if (i < 4) {
    AnimQueue[i]->renderFunc = &renderAnim_OpenMenuOption;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}*/

void Anim_SwipeUpMenu()
{
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 10;
  uint8_t i = getFirstEmptyAnim();
  if (i < 4)
  {
    AnimQueue[i]->renderFunc = &renderAnim_SwipeUpMenu;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}

void Anim_SwipeDownMenu()
{
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 10;
  uint8_t i = getFirstEmptyAnim();
  if (i < 4)
  {
    AnimQueue[i]->renderFunc = &renderAnim_SwipeDownMenu;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}

// Function that initiates the animation for Menu Corner sliding across the screen on menu open
void Anim_MenuCornerSlide()
{
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 16;
  uint8_t i = getFirstEmptyAnim();
  if (i < 4)
  {
    AnimQueue[i]->renderFunc = &renderAnim_MenuCornerSlide;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}

// Function that initiates the animation for swiping out the current song
void Anim_SwipeOutSong()
{
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 12;
  uint8_t i = getFirstEmptyAnim();
  if (i < 4)
  {
    AnimQueue[i]->renderFunc = &renderAnim_SwipeOutSong;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}

// Function that initiates the animation for swiping in the next song
void Anim_SwipeInSong()
{
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 12;
  uint8_t i = getFirstEmptyAnim();
  if (i < 4)
  {
    AnimQueue[i]->renderFunc = &renderAnim_SwipeInSong;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = -32;
  }
}

/*!
    @brief  Function for a "functional" animation which launches animation to switch songs in and out.
    @note    There's no function to actually switch current song to another one
*/
void funcAnim_SwitchSongs(struct AnimSeq *self)
{
  self->step++;
  if (self->step > self->duration)
  {
    Anim_SwipeInSong();
    PopAnim();
    return;
  }
  /*
  if (self->step = 13){
    //do a change text func
  }
  */
  if (self->step == 1)
  {
    Anim_SwipeOutSong();
  }
}

// Function that initiates the animation which launches animation to switch songs in and out.
void Anim_SwitchSongs()
{
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 12 + 6; // durations of SwipeIn and SwipeOut + delay
  uint8_t i = getFirstEmptyAnim();
  if (i < 4)
  {
    AnimQueue[i]->renderFunc = &funcAnim_SwitchSongs;
    AnimQueue[i]->step = 0;
    AnimQueue[i]->duration = duration / multiplier;
    AnimQueue[i]->offset[0] = 0;
    AnimQueue[i]->offset[1] = 0;
  }
}
