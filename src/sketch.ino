#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"
#include "call.h"

CallGSM call;
SMSGSM sms;

word nextMillis = 0;
word nextCallCheck = 0;

// Off-limits pins: 2, 3, 8, 9

#define INPUT_PHASE 4
#define INPUT_THERMAL 5
#define INPUT_PRESSURE 6
#define INPUT_OPERATION 7

#define INPUT_LENGTH 4
char inputPins[] = {INPUT_PHASE, INPUT_THERMAL, INPUT_PRESSURE, INPUT_OPERATION};
char pinStates[INPUT_LENGTH] = {0};

#define TimeLap(t1) (int)((word)millis()-(word)t1)

#define PIN_ON 11
#define PIN_OFF 10
#define PIN_ON_INTERVAL 500

#define ON LOW
#define OFF HIGH

#define INPUT_ENABLE 12

#define CALL_CHECK_INTERVAL 1000

char sendSMS(char *message_str) {
    // Send an SMS.
    Serial.print("Sending SMS: ");
    Serial.println(message_str);

    if (digitalRead(INPUT_ENABLE) == LOW) {
        sms.SendSMS(1, message_str);
        Serial.println("Message sent.");
    } else {
        Serial.println("Enable is off, message not sent.");
    }

}

void initPins() {
    pinMode(PIN_ON, OUTPUT);
    pinMode(PIN_OFF, OUTPUT);

    digitalWrite(PIN_ON, OFF);
    digitalWrite(PIN_OFF, OFF);

    for (int i=0; i < INPUT_LENGTH; i++) {
        pinMode(inputPins[i], INPUT_PULLUP);
        pinStates[i] =  digitalRead(inputPins[i]);
    }
    pinMode(INPUT_ENABLE, INPUT_PULLUP);
}

void initGSM() {
    // Initialize the GSM shield.
    if (gsm.begin(2400)) {
        Serial.println("\nGSM ready.");
    } else {
        Serial.println("\nGSM idle.");
    }
}

void toggleOutput() {
    // Toggle the output relays on or off.
    if (digitalRead(INPUT_OPERATION) == OFF) {
        digitalWrite(PIN_ON, ON);
        Serial.println("Pushing ON button.");
    } else {
        digitalWrite(PIN_OFF, ON);
        Serial.println("Pushing OFF button.");
    }
    nextMillis = (word)millis() + PIN_ON_INTERVAL;
}

void checkToggle() {
    // Check if we need to reset the output relays.
    if ((nextMillis != 0) && (TimeLap(nextMillis) >= 0)) {
        digitalWrite(PIN_ON, OFF);
        digitalWrite(PIN_OFF, OFF);
        nextMillis = 0;
    }
}

void checkRelays() {
    // Check the state of the relays that are connected to the pins.

    // The state of the current pin.
    char pinState = 0;
    // Whether the state of pin i changed.
    char statesChanged[INPUT_LENGTH] = {0};

    for (char i=0; i < INPUT_LENGTH; i++) {
        pinState = digitalRead(inputPins[i]);
        if (pinState != pinStates[i]) {
            // If the pin's state changed, note it.
            statesChanged[i] = 1;
        }
        // Update the state regardless.
        pinStates[i] = pinState;
    }

    // Decide what happens.
    if (statesChanged[0] == 1 && pinStates[0] == OFF) {
        // Phase dropped, send message regardless.
        sendSMS("Phase dropped.");
    } else if (statesChanged[1] == 1 && pinStates[1] == OFF) {
        sendSMS("Thermal dropped.");
    } else if ((statesChanged[2] == 1 && pinStates[2] == OFF) &&
               (statesChanged[3] == 1 &&  pinStates[3] == OFF)) {
        // Pressure dropped *and operation dropped simultaneously*.
        sendSMS("Pressure dropped.");
    } else if (statesChanged[3] == 1 && pinStates[3] == OFF) {
        sendSMS("Pump shut down.");
    } else if (statesChanged[3] == 1 && pinStates[3] == ON) {
        sendSMS("Pump started.");
    }
}

void checkCall() {
    byte status = 0;
    char number[20];

    // Only check for a call every CALL_CHECK_INTERVAL milliseconds.
    if (TimeLap(nextCallCheck) < 0) {
        return;
    }

    // Check whether we're receiving an authorized call.
    status = call.CallStatusWithAuth(number, 1, 2);
    if (status == CALL_INCOM_VOICE_AUTH) {
        Serial.println("Authorized call detected, changing state.");
        toggleOutput();
        call.HangUp();
    }
    nextCallCheck = (word)millis() + CALL_CHECK_INTERVAL;
}

void setup() {
    initPins();
    initGSM();

    Serial.begin(9600);
}

void loop() {
    checkCall();
    checkToggle();
    checkRelays();
    delay(500);
}
