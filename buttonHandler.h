#include <Button.h>

Button volUp(4); // Volume Up button
Button volDn(4); // Volume Down button
Button prev(4);  // Previous track button
Button next(4);  // Next track button

Button play(4); // Play button
Button mode(4); // Switch play mode button
Button disp(4); // Display button

enum operationMode
{
    PLAY,
    SLEEP,
    HOME,
    MENU
};

operationMode currMode = PLAY;

void initButtons()
{
    volUp.begin();
    volDn.begin();
    prev.begin();
    next.begin();
    play.begin();
    mode.begin();
    disp.begin();
}

void handleButtons()
{
    switch (currMode)
    {
    case PLAY:
        // Volume
        if (volUp.pressed())
        {
            volumeChange(1);
        }
        else if (volDn.pressed())
        {
            volumeChange(-1);
        }
        // Tracks
        if (prev.pressed())
        {
            trackSwipe(-1);
        }
        else if (next.pressed())
        {
            trackSwipe(1);
        }
        // Action buttons
        if (play.pressed()){
            pauseFunc();
        }
        if (mode.pressed()){
            switchPlayMode();
        }
        if (disp.pressed())
        {
            currMode = HOME;
        }
        // Actually handling long press
        // Should switch to Sleep mode
        break;

    case SLEEP:
        // Volume
        if (volUp.pressed())
        {
            volumeChange(1);
        }
        else if (volDn.pressed())
        {
            volumeChange(-1);
        }
        // Tracks
        if (prev.pressed())
        {
            trackSwipe(-1);
        }
        else if (next.pressed())
        {
            trackSwipe(1);
        }
        // Action buttons
        if (play.pressed()){
            pauseFunc();
        }
        if (mode.pressed()){
            switchPlayMode();
        }
        // Actually handling long press
        // Should switch to Play mode
        break;
    
    case HOME:
        // Volume
        if (volUp.pressed())
        {
            volumeChange(1);
        }
        else if (volDn.pressed())
        {
            volumeChange(-1);
        }
        // Changing options
        if (prev.pressed())
        {
            optionChange(-1);
        }
        else if (next.pressed())
        {
            optionChange(1);
        }
        // Action buttons
        if (play.pressed()){
            selectOption();
        }
        if (mode.pressed()){
            currMode = PLAY;
        }
        if (disp.pressed())
        {
            currMode = PLAY;
        }
        break;

    case MENU:
        // Volume
        if (volUp.pressed())
        {
            volumeChange(1);
        }
        else if (volDn.pressed())
        {
            volumeChange(-1);
        }
        // Changing options
        if (prev.pressed())
        {
            optionChange(-1);
        }
        else if (next.pressed())
        {
            optionChange(1);
        }
        // Action buttons
        if (play.pressed()){
            selectOption();
        }
        if (mode.pressed()){
            prevMenu();
        }
        if (disp.pressed())
        {
            currMode = HOME;
        }
        break;
    }
}
