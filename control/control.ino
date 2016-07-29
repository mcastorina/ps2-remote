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

// #define DEBUG
#define LED_PIN         13      // LED pin
#define RECV_PIN        11      // IR pin
#define BUTTON_PIN      10      // Button input (negative logic)
#define POT_ADDR        0x3e    // I2C address of digital potentiometer

void reset(void);
void power_off(void);
void blink(int count);

/* Map of IR code to function call */
static struct {
    long ir_code;
    void (*fn)(void);
} ir_map[] = {
    {0, reset},
    {0, power_off},
    {0, NULL}
};

IRrecv irrecv(RECV_PIN);
decode_results results; // IR Input

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
    press_button(false);
    /* Button for recording IR code */
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    /* LED for visual confirmation */
    pinMode(LED_PIN, OUTPUT);
#ifdef DEBUG
    /* Make sure the number of codes we have fits in EEPROM */
    if (sizeof(ir_map) / sizeof(ir_map[0])-1 > EEPROM.length() * sizeof(long)) {
        Serial.println("Warning: ir_map exceeds EEPROM length");
    }
#endif /* DEBUG */
    /* Set codes to value in EEPROM */
    get_values();
}

/*
 * If button was pressed, update the codes.
 * If we got an IR code, check against ir_map and call function.
 */
void loop() {
    if (!digitalRead(BUTTON_PIN))
        update_codes();
    if (irrecv.decode(&results)) {
        long code = get_code(&results);
        for (int index = 0; ir_map[index].fn != NULL; index++) {
            if (code == ir_map[index].ir_code) {
                ir_map[index].fn();
            }
        }
        delay(100);
        irrecv.resume();
    }
}

/*
 * Simulates a button press by controlling a digital potentiometer via
 * I2C.
 */
void press_button(boolean down) {
    int val = 0xff * down;  // low val = high resistance
    Wire.beginTransmission(POT_ADDR);
    Wire.write(0);          // command
    Wire.write(val);        // data (0x00 - 0xff)
    Wire.endTransmission();
    digitalWrite(LED_PIN, down);    // visual confirmation
#ifdef DEBUG
    Serial.print("press_button: ");
    Serial.println(down);
#endif /* DEBUG */
}

/*
 * Updates the IR code to listen for.
 */
void update_codes() {
#ifdef DEBUG
    Serial.println("updating codes");
#endif /* DEBUG */
    blink(3);                                       // Visual cue
    /* Update the ir_map */
    for (int index = 0; ir_map[index].fn != NULL; index++) {
        while (!irrecv.decode(&results));           // Wait for signal
        long val = get_code(&results);
        ir_map[index].ir_code = val;
        set_value(index, ir_map[index].ir_code);    // Store result
        delay(100);
        irrecv.resume();
        blink(1);                                   // Visual cue
#ifdef DEBUG
        Serial.print("update_code ");
        Serial.print(index);
        Serial.print(": ");
        Serial.println(val, HEX);
#endif /* DEBUG */
    }
}

/*
 * Gets the values stored in EEPROM, and populates ir_map.
 */
void get_values() {
    for (int index = 0; ir_map[index].fn != NULL; index++) {
        long val = 0;
        for (int i = 0; i < sizeof(long) &&
                        i + index * sizeof(long) < EEPROM.length(); ++i) {
            int offset = (i << 3);
            byte n = EEPROM.read(i + index * sizeof(long));
            val |= ((long)n << offset);
        }
        ir_map[index].ir_code = val;
#ifdef DEBUG
        Serial.print("get_value: ");
        Serial.print(index);
        Serial.print(", ");
        Serial.println(val, HEX);
#endif /* DEBUG */
    }
}

/*
 * Sets the value stored in EEPROM.
 */
void set_value(int index, long val) {
    for (int i = 0; i < sizeof(long) &&
                    i + index * sizeof(long) < EEPROM.length(); ++i) {
        int offset = (i << 3);
        byte n = (val & (0xffL << offset)) >> offset;
        EEPROM.write(i + index * sizeof(long), n);
    }
#ifdef DEBUG
    Serial.print("set_value: ");
    Serial.println(val, HEX);
#endif /* DEBUG */
}

/*
 * Records the code from the IR receiver.
 */
long get_code(decode_results *ptr) {
    long val = ptr->value;
#ifdef DEBUG
    Serial.print("get_code: ");
    Serial.println(val, HEX);
#endif /* DEBUG */
    return val;
}

/*
 * Blink LED_PIN count times.
 */
void blink(int count) {
    for (int i = 0; i < count; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
}

/*
 * PS2 reset is a quick press.
 */
void reset(void) {
    press_button(true);
    delay(100);
    press_button(false);
}

/*
 * PS2 power off is a press and hold.
 */
void power_off(void) {
    press_button(true);
    delay(2500);
    press_button(false);
}
