#include <Bluepad32.h>
#include <ESP32Servo.h>

// ====== Pins ======
#define ESC_PIN 14
#define STEERING_PIN 27
#define TRIG_PIN 32
#define ECHO_PIN 33

// ====== ESC values ======
#define ESC_NEUTRAL 1500
#define ESC_FORWARD_MAX 1900
#define ESC_REVERSE_MAX 1100

ControllerPtr myController;
Servo esc;
Servo steering;

// ====== Distance Function ======
float getDistanceCM() {

    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms timeout

    float distance = duration * 0.034 / 2.0;
    return distance;
}

// ====== Controller Callbacks ======
void onConnectedController(ControllerPtr ctl) {
    myController = ctl;
}

void onDisconnectedController(ControllerPtr ctl) {
    myController = nullptr;
}

void setup() {
    Serial.begin(115200);

    esc.attach(ESC_PIN);
    steering.attach(STEERING_PIN);

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    esc.writeMicroseconds(ESC_NEUTRAL);
    delay(3000);

    BP32.setup(&onConnectedController, &onDisconnectedController);
}

void loop() {

    BP32.update();

    float distance = getDistanceCM();

    if (myController && myController->isConnected()) {

        int r2 = myController->throttle();  // 0–1023
        int l2 = myController->brake();     // 0–1023

        int escSignal = ESC_NEUTRAL;

        // ===== FORWARD =====
        if (r2 > 20) {

            if (distance > 15 || distance == 0) {
                escSignal = map(r2, 0, 1023, ESC_NEUTRAL, ESC_FORWARD_MAX);
            } else {
                escSignal = ESC_NEUTRAL;  // AUTO BRAKE
            }

        }
        // ===== REVERSE =====
        else if (l2 > 20) {
            escSignal = map(l2, 0, 1023, ESC_NEUTRAL, ESC_REVERSE_MAX);
        }
        else {
            escSignal = ESC_NEUTRAL;
        }

        esc.writeMicroseconds(escSignal);

        // ===== STEERING =====
        int joyX = myController->axisX();
        int steerAngle = map(joyX, -512, 512, 45, 135);
        steering.write(steerAngle);
    }

    delay(20);
}
