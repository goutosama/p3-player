#ifndef defines_h
#define defines_h

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define ANIM_QUEUE_LENGTH 4 // Length of buffer for animations. Basically is how many animations can be played at the same time
#define FPS 30 //Recommended to be divisible by 5 and equal or less than 60

#endif