#include <Button.h>

Button volUp(4); // Volume Up button
Button volDn(4); // Volume Down button
Button prev(4); // Previous track button
Button next(4); // Next track button

Button play(4); // Play button
Button mode(4); // Switch play mode button
Button disp(4); // Display button

void initButtons(){
    volUp.begin();
    volDn.begin();
    prev.begin();
    next.begin();
    play.begin();
    mode.begin();
    disp.begin();
}

void handleButtons(){
    
}