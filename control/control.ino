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

#define DEBUG
#define RECV_PIN        11      // IR pin
#define BUTTON_PIN      10      // Button input (negative logic)
#define POT_ADDR        0x3e    // I2C address of digital potentiometer

IRrecv irrecv(RECV_PIN);
decode_results results; // IR Input
long remote_value;      // IR Code

static boolean pressed;

/*
 * Initialize IR receiver, I2C, and button input.
 */
void setup() {
#ifdef DEBUG
    Serial.begin(9600);
    while (!Serial);
#endif /* DEBUG */

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
 * If we got an IR code, check against remote_value and press button.
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
#ifdef DEBUG
    Serial.print("press_button: ");
    Serial.println(down);
#endif /* DEBUG */
}

/*
 * Updates the IR code to listen for.
 */
void update_code() {
#ifdef DEBUG
    Serial.println("updating code");
#endif /* DEBUG */
    while (!irrecv.decode(&results));   // Wait for signal
    remote_value = get_code(&results);
    set_value(remote_value);            // Store result
    delay(100);
    irrecv.resume();
#ifdef DEBUG
    Serial.print("update_code: ");
    Serial.println(remote_value);
#endif /* DEBUG */
}

/*
 * Gets the value stored in EEPROM.
 */
long get_value() {
    long val = 0;
    for (int i = 0; i < sizeof(long); ++i) {
        int offset = (i << 3);
        val |= (EEPROM.read(i) << offset);
    }
#ifdef DEBUG
    Serial.print("get_value: ");
    Serial.println(val);
#endif /* DEBUG */
    return val;
}

/*
 * Sets the value stored in EEPROM.
 */
void set_value(long val) {
    for (int i = 0; i < sizeof(long); ++i) {
        int offset = (i << 3);
        EEPROM.write(i, (val & (0xff << offset)) >> offset);
    }
#ifdef DEBUG
    Serial.print("set_value: ");
    Serial.println(val);
#endif /* DEBUG */
}

/*
 * Records the code from the IR receiver.
 */
long get_code(decode_results *ptr) {
    long val = ptr->value;
#ifdef DEBUG
    Serial.print("get_code: ");
    Serial.println(val);
#endif /* DEBUG */
    return val;
}
