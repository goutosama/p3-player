#ifndef structs_h
#define structs_h

#include <Arduino.h>
#include <defines.h>

enum PlayMode {Shuffle, LoopFolder, Loop1};

struct MusicState
{                  // A struct defining current state of playing track
  char *trackName; // Name of the track currently playing (if not working try [])
  uint8_t PlayState;  // That icon close to the timer, shows the status of playing (Play - 3, Pause - 1, Stop - 2)
  uint16_t timer;  // A timer of current song. Note that it uses uint16 (I need more digits!!!)
  uint8_t volume;  // Current volume of player from 0 to 5
  uint8_t battery; // Current battery level of player from 0 to 4
  uint8_t folder; //The number of folder that music plays from
  uint16_t track; // The number of track that should play
  PlayMode playMode; // Current playback mode for player (see PlayMode enum)
};

struct MusicState *stat;
struct AnimSeq
{
    // A struct for intercepting every animation at the same time synced with 60hz timer
    void (*renderFunc)(struct AnimSeq *self); // A function that executes every frame of animation. The whole AnimSeq object is considered empty if this pointer equals NULL. @note However, calling this function when it's NULL may cause instant crash, be careful.
    uint8_t step;                                 // Current frame of animation (gets updated with every frame)
    uint8_t duration;                             // Duration in 60 frames per second (interpolated, if FPS is set to less than that)
    int16_t offset[2];                            // Should be used to offset drawing current animation if whole object moves 
};

class AnimQueue
{
public:
  AnimSeq slots[ANIM_QUEUE_LENGTH];
  bool isEmpty();
  uint8_t getFirstEmptyAnim();
  void PopAnim();
  AnimQueue();
  ~AnimQueue();
};

AnimQueue::AnimQueue()
{
  for (uint8_t i = 0; i < ANIM_QUEUE_LENGTH; i++)
  {
    this->slots[i] = *(struct AnimSeq *)malloc(sizeof(struct AnimSeq)); // I'm so not sure about these two lines
    this->slots[i].renderFunc = NULL;
  }
}

AnimQueue::~AnimQueue()
{
  free(this->slots); // Will this free all AnimSeq structs inside of this array? Who knows...
}

/*!
    @brief  Function that returns 0 when there's no animations in the queue and 1 elsewhere
    @return Returns bool value
            1 - no animations in the queue
            0 - at least one animation is in the queue
*/
bool AnimQueue::isEmpty()
{
  for (uint8_t i = 0; i < 4; i++)
  {
    if (this->slots[i].renderFunc != NULL)
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
uint8_t AnimQueue::getFirstEmptyAnim()
{
  // 4 means AnimQueue is full
  for (uint8_t i = 0; i < 4; i++)
  {
    if (this->slots[i].renderFunc == NULL)
    {
      return i;
    }
  }
  return 4;
}

/*!
    @brief  This function *deletes* every animation in the queue that is already finished
*/
void AnimQueue::PopAnim()
{
  for (uint8_t i = 0; i < 4; i++)
  {
    if (this->slots[i].duration < this->slots[i].step)
    {
      this->slots[i].renderFunc = NULL;
    }
  }
}



struct Point
{ // point in 2D space
  int16_t x;
  int16_t y;
};

struct WheelAnim
{ // Animation steps for revolver cylinder menu
  // Every elemnt is counted from left to right
  Point* hole1;
  Point* hole2;
  Point* hole3;
  Point* hole4;
  Point* hole5;
  Point* edge1;
  Point* edge2;
  Point* edge3;
  Point* edge4;
};

class Animation
{
private:
  AnimQueue queue;
  uint8_t duration;
  void (*renderFunc)(struct AnimSeq *self);
  int16_t offset[2];

public:
  Animation(AnimQueue queue, uint8_t duration, void (*renderFunc)(AnimSeq *self), int16_t offset[2]);
  ~Animation();
  void start();
};

Animation::Animation(AnimQueue queue, uint8_t duration, void (*renderFunc)(AnimSeq *self), int16_t offset[2])
{
  this->queue = queue;
  this->duration = duration;
  this->renderFunc = renderFunc;
  this->offset[0] = offset[0]; // Can someone explain if I did horrible mistake placing this instead of equivalent of this->offset = offset?
  this->offset[1] = offset[1];
}
Animation::~Animation(){}

void Animation::start()
{
  uint8_t multiplier = 60 / FPS;
  uint8_t i = queue.getFirstEmptyAnim();
  if (i < 4)
  {
    queue.slots[i].renderFunc = this->renderFunc; // The slide in animation left? or right?
    queue.slots[i].step = 0;
    queue.slots[i].duration = Animation::duration / multiplier;
    queue.slots[i].offset[0] = Animation::offset[0];
    queue.slots[i].offset[1] = Animation::offset[1];
  }
}

#endif
