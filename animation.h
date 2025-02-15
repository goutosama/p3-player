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
AnimQueue animQueue = AnimQueue();

void initAnimations()
{
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
const Point winPos{16,10};
const Point winSize{96, 11};

void drawCenterOption(char *text, int16_t offset[2], bool isFullOffset)
{
  // center option
  int8_t offX = winPos.x;
  int8_t offY = winPos.y;
  display.setTextColor(WHITE);
  if (isFullOffset)
  {
    offX += offset[0];
    offY += offset[1];
  }
  display.fillRect(offX, offY, winSize.x,winSize.y, BLACK);
  display.setCursor(winPos.x + 2 + offset[0], winPos.y + 2 + offset[1]);
  display.print(text);
}

void drawUpperOption(char *text, int16_t offset[2])
{
  display.setTextColor(BLACK);
  display.setCursor(winPos.x, winPos.y - 8);
  display.print((char)0x1E);
  display.print(" ");
  display.setCursor(winPos.x + offset[0] + 2 * 6, winPos.y - 8 + offset[1]);
  display.print(text);
}

void drawLowerOption(char *text, int16_t offset[2])
{
  display.setTextColor(BLACK);
  display.setCursor(winPos.x, winPos.y + 12);
  display.print((char)0x1F);
  display.print(" ");
  display.setCursor(winPos.x + offset[0] + 2 * 6, winPos.y + 12 + offset[1]);
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

void drawPlaybackState(byte state, int16_t offset[2])
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
  // display.fillRect(start_x + offset[0], start_y + offset[1], 14, 11, WHITE);
  switch (mode)
  {
  case LoopFolder:
    display.drawBitmap(start_x + offset[0], start_y + offset[1], bitmap_icon_loop, 14, 11, BLACK);
    break;

  case Loop1:
    display.drawBitmap(start_x + offset[0], start_y + offset[1], bitmap_icon_loop, 14, 11, WHITE);
    display.drawBitmap(start_x + 5, start_y + 2, bitmap_icon_loop1, 3, 6, BLACK);
    break;

  case Shuffle:
    display.drawBitmap(start_x + offset[0], start_y + offset[1], bitmap_icon_shuffle, 14, 11, WHITE);
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

// todo: move to the seperate category/file/folder/whatever
// seems like it can be compressed by moving to a single array with whole circle coords
Point hole1[9] = {{0, -22}, {-6, -21}, {-11, -19}, {-16, -16}, {-19, -11}, {-21, -6}, {-22, 0}, {-21, 5}, {-19, 11}};
Point hole2[9] = {{-20, -12}, {-23, -6}, {-23, 0}, {-22, 6}, {-20, 12}, {-16, 16}, {-11, 20}, {-6, 23}, {0, 23}};
Point hole3[9] = {{-20, 12}, {-16, 16}, {-11, 20}, {-6, 22}, {0, 23}, {6, 22}, {11, 20}, {16, 16}, {20, 11}};
Point hole4[9] = {{0, 23}, {5, 23}, {11, 20}, {16, 17}, {20, 12}, {23, 6}, {23, 0}, {23, -6}, {20, -11}};
Point hole5[9] = {{20, 12}, {22, 6}, {23, 0}, {22, -5}, {20, -11}, {17, -16}, {12, -19}, {6, -22}, {0, -23}};
Point edge4[9] = {{23, 40}, {33, 33}, {40, 23}, {44, 12}, {46, 0}, {45, -12}, {40, -23}, {33, -33}, {23, -40}};
Point edge3[9] = {{-23, 40}, {-12, 45}, {0, 46}, {12, 44}, {23, 40}, {33, 32}, {-40, 23}, {45, 12}, {46, 0}};
Point edge2[9] = {{-46, 0}, {-44, 12}, {-40, 23}, {-33, 33}, {-23, 40}, {-11, 44}, {0, 46}, {12, 45}, {23, 40}};
Point edge1[9] = {{-23, -40}, {-33, -33}, {-40, -23}, {-44, -12}, {-46, 0}, {-44, 11}, {-40, 23}, {-33, 33}, {-23, 40}};

WheelAnim wheelAnim =
    {
        &hole1[4],
        &hole2[4],
        &hole3[4],
        &hole4[4],
        &hole5[4],
        &edge1[4],
        &edge2[4],
        &edge3[4],
        &edge4[4]};

void drawWheel(uint8_t step, int16_t offset[2]) // default offset (centered) is x: 64, y: -9
{
  if (step >= 5 or step <= -5)
  {
    step = 0;
  }
  display.fillCircle(offset[0], offset[1], 36, BLACK);                                                             // Base cylinder
  display.fillCircle((wheelAnim.hole4 + step)->x + offset[0], (wheelAnim.hole4 + step)->y + offset[1], 9, WHITE);  // Hole 4
  display.fillCircle((wheelAnim.hole3 + step)->x + offset[0], (wheelAnim.hole3 + step)->y + offset[1], 9, WHITE);  // Hole 3
  display.fillCircle((wheelAnim.hole2 + step)->x + offset[0], (wheelAnim.hole2 + step)->y + offset[1], 9, WHITE);  // Hole 2
  display.fillCircle((wheelAnim.edge2 + step)->x + offset[0], (wheelAnim.edge2 + step)->y + offset[1], 14, WHITE); // Edge 2
  display.fillCircle((wheelAnim.edge3 + step)->x + offset[0], (wheelAnim.edge3 + step)->y + offset[1], 14, WHITE); // Edge 3
  if (step != 0)
  {
    display.fillCircle((wheelAnim.hole1 + step)->x + offset[0], (wheelAnim.hole1 + step)->y + offset[1], 9, WHITE);  // Hole 1
    display.fillCircle((wheelAnim.hole5 + step)->x + offset[0], (wheelAnim.hole5 + step)->y + offset[1], 9, WHITE);  // Hole 5
    display.fillCircle((wheelAnim.edge1 + step)->x + offset[0], (wheelAnim.edge1 + step)->y + offset[1], 14, WHITE); // Edge 1
    display.fillCircle((wheelAnim.edge4 + step)->x + offset[0], (wheelAnim.edge4 + step)->y + offset[1], 14, WHITE); // Edge 4
  }
}

// Animation internal functions
void renderAnim_wheelRotationRight(struct AnimSeq *self) // idk if it's left btw
{
  uint8_t travel = 4; // in frames (actual animation, 30 FPS)

  if (self->step > self->duration)
  {
    animQueue.PopAnim();
    return;
  }

  int8_t frameStep = map(self->step, 0, self->duration, 0, travel);
  drawWheel(frameStep, self->offset);
  self->step++;
}

void renderAnim_wheelRotationLeft(struct AnimSeq *self) // idk if it's left btw
{
  uint8_t travel = 4; // in frames (actual animation, 30 FPS)

  if (self->step > self->duration)
  {
    animQueue.PopAnim();
    return;
  }

  int8_t frameStep = -map(self->step, 0, self->duration, 0, travel);
  drawWheel(frameStep, self->offset);
  self->step++;
}
// Render function for bubbles or circles sliding across the screen on media bar closing
// @note  WARNING: Unstable. Can cause crashes of whole display. That was when I did full refresh, but now circles still might be too much for arduino to handle (looks cooler and P3 like though)
void renderAnim_BarBubbleTransition(struct AnimSeq *self)
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
    animQueue.PopAnim();
    return;
  }

  self->offset[1] = map(self->step, 0, self->duration, 12, travel);
  if (self->offset[1] <= 112)
  {
    display.drawCircle(112, 0, self->offset[1] + 1, BLACK);
    display.drawCircle(112, 0, self->offset[1], BLACK);
    display.fillCircle(112, 0, self->offset[1] - 1, WHITE);
  }
  if (self->step > self->duration / 4)
  {
    display.drawCircle(32, 0, self->offset[1] - 50, BLACK);
    display.fillCircle(32, 0, self->offset[1] - 51, WHITE);
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
    animQueue.PopAnim();
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
    animQueue.PopAnim();
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

void renderAnim_OpenMenuOption(struct AnimSeq *self)
{
  uint8_t travel = 11; // in pixels

  if (self->step > self->duration)
  {
    animQueue.PopAnim();
    return;
  }

  self->offset[1] = map(self->step, 0, self->duration, 0, travel);

  self->step++;
}

// Render function for swiping out the current song
void renderAnim_SwipeOutSong(struct AnimSeq *self)
{
  uint8_t travel = 32; // in pixels

  if (self->step > self->duration)
  {
   animQueue.PopAnim();
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
   animQueue.PopAnim();
    return;
  }

  self->offset[1] = -map(self->step, 0, self->duration, travel, 0);
  drawMediaBar(self->offset, stat);
  self->step++;
}

// animation objects
//these two are in old format cuz idk what to do with them yet
void Anim_WheelSlideIn(bool isReversed)
{
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 8;
  uint8_t i = animQueue.getFirstEmptyAnim();
  if (i < 4)
  {
    animQueue.slots[i].renderFunc = &renderAnim_wheelRotationLeft; // The slide in animation left? or right?
    animQueue.slots[i].step = 0;
    animQueue.slots[i].duration = duration / multiplier;
    animQueue.slots[i].offset[0] = 64;
    animQueue.slots[i].offset[1] = -9;
  }
}

/*
  @brief Animation of rotating revolver cylinder (wheel for short)
  @note Use bool variable to determine flow of the animation: True - Counter-clockwise , False - Clockwise
*/
void Anim_wheelRotation(bool isReversed)
{
  uint8_t multiplier = 60 / FPS;
  uint8_t duration = 8;
  uint8_t i = animQueue.getFirstEmptyAnim();

  void (*func)(struct AnimSeq *self);
  if (isReversed)
  {
    func = &renderAnim_wheelRotationRight;
  }
  else
  {
    func = &renderAnim_wheelRotationLeft;
  }

  if (i < 4)
  {
    animQueue.slots[i].renderFunc = func;
    animQueue.slots[i].step = 0;
    animQueue.slots[i].duration = duration / multiplier;
    animQueue.slots[i].offset[0] = 64;
    animQueue.slots[i].offset[1] = -9;
  }
}

Animation Anim_SwipeUpMenu = Animation(animQueue, uint16_t(10), &renderAnim_SwipeUpMenu, (int16_t[2]){0,0});

Animation Anim_SwipeDownMenu = Animation(animQueue, uint16_t(10), &renderAnim_SwipeDownMenu, (int16_t[2]){0,0});

// Function that initiates the animation for bubbles or circles sliding across the screen on media bar close
Animation Anim_BarBubbleTransition = Animation(animQueue, uint16_t(16), &renderAnim_BarBubbleTransition, (int16_t[2]){0,0});

// Function that initiates the animation for swiping out the current song
Animation Anim_SwipeOutSong = Animation(animQueue, uint16_t(12), &renderAnim_SwipeOutSong, (int16_t[2]){0,0});

// Function that initiates the animation for swiping in the next song
Animation Anim_SwipeInSong = Animation(animQueue, uint16_t(12), &renderAnim_SwipeInSong, (int16_t[2]){0,-32});

Animation Anim_OpenMenuOption = Animation(animQueue, uint16_t(10), &renderAnim_OpenMenuOption, (int16_t[2]){0,0});

void funcAnim_Trans_MediaBarToHome(struct AnimSeq *self)
{
  self->step++;
  if (self->step > self->duration)
  {
    Anim_WheelSlideIn(false); // still an object TODO
    animQueue.PopAnim();
    return;
  }
  if (self->step == 1)
  {
    Anim_BarBubbleTransition.start();
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
   Anim_SwipeInSong.start();
   animQueue.PopAnim();
    return;
  }
  /*
  if (self->step = 13){
    //do a change text func
  }
  */
  if (self->step == 1)
  {
    Anim_SwipeOutSong.start();
  }
}

// Function that initiates the animation which launches animation to switch songs in and out.
const Animation Anim_SwitchSongs = Animation(animQueue, uint16_t(12 + 6), &funcAnim_SwitchSongs, (int16_t[2]){0,0});