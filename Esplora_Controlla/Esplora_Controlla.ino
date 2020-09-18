#include <Keyboard.h>
#include <Esplora.h>


boolean buttonStates[8];

const byte buttons[] = {

JOYSTICK_DOWN,

JOYSTICK_LEFT,

JOYSTICK_UP,

JOYSTICK_RIGHT,

SWITCH_RIGHT,

SWITCH_LEFT,

SWITCH_UP,

SWITCH_DOWN,

};

const char keystrokes[] = {

'S',

'A',

'W',

'D',

'Q ',

'X',

'T',

'Z'

};

//You can change the keys to whatever you want.

void setup() {

Keyboard.begin();

}

void loop() {

for(byte thisButton = 0; thisButton < 8; thisButton++) {

boolean lastState = buttonStates[thisButton];

boolean newState = Esplora.readButton(buttons[thisButton]);

if (lastState != newState) {

if (newState == PRESSED) {

Keyboard.press(keystrokes[thisButton]);

}

else if (newState ==RELEASED) {

Keyboard.release(keystrokes[thisButton]);

}

}

buttonStates[thisButton] = newState;

}

delay(50);

int xAxis = Esplora.readAccelerometer(X_AXIS);

if (xAxis < 0) { Keyboard.press('E'); delay(50);

};

if (xAxis > 50) {

Keyboard.press('Q');

delay(50);

};

delay(50);

}
