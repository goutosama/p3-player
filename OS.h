#include <Arduino.h>

#include "defines.h"
#include "structs.h"
#include "animation.h"
#include "dfplayerMini.h"
#include "buttonHandler.h"

void startup()
{
    initAnimations();   // Handle errors. EVERYWHERE
    initButtons();
    initPlayer();


}
