#ifndef structs_h
#define structs_h

#include <Arduino.h>

struct MusicState
{                  // A struct defining current state of playing track
  char *trackName; // Name of the track currently playing (if not working try [])
  uint8_t PlayState;  // That icon close to the timer, shows the status of playing (Play, Pause, Stop)
  uint16_t timer;  // A timer of current song. Note that it uses uint16 (I need more digits!!!)
  uint8_t volume;  // Current volume of player from 0 to 5
  uint8_t battery; // Current battery level of player from 0 to 4
};

struct AnimSeq
{                                               // A struct for intercepting every animation at the same time synced with 60hz timer
    void (*renderFunc)(struct AnimSeq *self); // A function that executes every frame of animation. The whole AnimSeq object is considered empty if this pointer equals NULL. @note However, calling this function when it's NULL may cause instant crash, be careful.
    uint8_t step;                                 // Current frame of animation (gets updated with every frame)
    uint8_t duration;                             // Duration in 60 frames per second (interpolated, if FPS is set to less than that)
    int16_t offset[2];                            // Should be used to offset drawing current animation if whole object moves 
};

#endif