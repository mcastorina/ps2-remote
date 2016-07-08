/*
 * Remote control button.
 *
 * This project converts a physical button into a remotely controlled
 * button. It does this using an infrared (IR) receiver and a digital
 * potentiometer. The code below will wait for an IR signal (e.g. sent
 * from a television remote control) and change the resistance on the
 * potentiometer to simulate a physical button press.
 *
 * Data Flow:
 *      IR )))      Receiver ----> Button
 */
#include <EEPROM.h>
#include <IRremote.h>
#include <Wire.h>

#define RECV_PIN        11      // IR pin
#define BUTTON_PIN      2       // Button input (negative logic)
#define POT_ADDR        0x3e    // I2C address of digital potentiometer

IRrecv irrecv(RECV_PIN);
decode_results results; // IR Input
long remote_value;      // IR Code

static boolean pressed;

/*
 * Initialize IR receiver, I2C, and button input.
 */
void setup() {
    /* Setup IR receiver and I2C */
    irrecv.enableIRIn();
    Wire.begin();
    /* Button for recording IR code */
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    /* Set code to value in EEPROM */
    remote_value = get_value();

    pressed = false;
}

/*
 * If button was pressed, update the code.
 * If we got an IR code, check against remote_code and press button.
 * If we pressed the button in the last iteration, unpress it.
 */
void loop() {
    if (!digitalRead(BUTTON_PIN))
        update_code();
    if (irrecv.decode(&results)) {
        long code = get_code(&results);
        if (code == remote_value) {
            pressed = true;
            press_button(true);
        }
        delay(100);
        irrecv.resume();
    }
    else if (pressed) {
        pressed = false;
        press_button(false);
    }
}

/*
 * Simulates a button press by controlling a digital potentiometer via
 * I2C.
 */
void press_button(boolean down) {
    int val = 0xff * down;  // low val = high resistance
    Wire.beginTransmission(POT_ADDR);
    Wire.write(0);      // command
    Wire.write(val);    // data (0x00 - 0xff)
    Wire.endTransmission();
}

/*
 * Updates the IR code to listen for.
 */
void update_code() {
    while (!irrecv.decode(&results));   // Wait for signal
    remote_code = get_code(&results);
    set_value(remote_code);             // Store result
    delay(100);
    irrecv.resume();
}

/*
 * Gets the value stored in EEPROM.
 */
long get_value() {
    long val = 0;
    for (int i = 0; i < sizeof(long); ++i)
        val |= (EEPROM.read(i) << (i << 3));
    return val;
}

/*
 * Sets the value stored in EEPROM.
 */
void set_value(long val) {
    for (int i = 0; i < sizeof(long); ++i)
        EEPROM.write(i, val & (0xFF << (i << 3)));
}

/*
 * Records the code from the IR receiver.
 */
long get_code(decode_results *ptr) {
    int codeLen = ptr->rawlen - 1;
    long val = 0;
    for (int i = 2; i <= codeLen; i+=2) {
        boolean v = (ptr->rawbuf[i]*USECPERTICK >= 1100);
        val |= (v << (i-2)/2);
    }
    return val;
}
