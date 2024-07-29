#include <Button.h>

Button volUp(4); // Volume Up Button
Button volDn(4); // Volume Down Button
Button prev(4);  // Previous track Button
Button next(4);  // Next track Button

Button play(4); // Play Button
Button mode(4); // Switch play mode Button
Button disp(4); // Display Button

typedef enum Buttons
{
    VOLUP,
    VOLDN,
    PREV,
    NEXT,
    PLAY,
    MODE,
    DISP,
   // RESERVED
} Buttons;

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
struct ButtonState
{
    byte ButtonJustPressedState = 0b00000000;
    byte ButtonPressedState = 0b00000000;
} state;

struct ButtonState handleButton()
{
    getButtonJustPressedState();
    getButtonPressed();
    return state;
}

void getButtonJustPressedState()
{
    if (volUp.pressed())
    {
        state.ButtonJustPressedState += 0b10000000;
    }
    if (volDn.pressed())
    {
        state.ButtonJustPressedState += 0b01000000;
    }
    if (prev.pressed())
    {
        state.ButtonJustPressedState += 0b00100000;
    }
    if (next.pressed())
    {
        state.ButtonJustPressedState += 0b00010000;
    }
    if (play.pressed())
    {
        state.ButtonJustPressedState += 0b00001000;
    }
    if (mode.pressed())
    {
        state.ButtonJustPressedState += 0b00000100;
    }
    if (disp.pressed())
    {
        state.ButtonJustPressedState += 0b00000010;
    }
}

void getButtonPressed()
{
    if (volUp.PRESSED)
    {
        state.ButtonPressedState += 0b10000000;
    }
    if (volDn.PRESSED)
    {
        state.ButtonPressedState += 0b01000000;
    }
    if (prev.PRESSED)
    {
        state.ButtonPressedState += 0b00100000;
    }
    if (next.PRESSED)
    {
        state.ButtonPressedState += 0b00010000;
    }
    if (play.PRESSED)
    {
        state.ButtonPressedState += 0b00001000;
    }
    if (mode.PRESSED)
    {
        state.ButtonPressedState += 0b00000100;
    }
    if (disp.PRESSED)
    {
        state.ButtonPressedState += 0b00000010;
    }
}