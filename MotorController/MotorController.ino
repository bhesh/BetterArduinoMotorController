/**
 * Advanced Motor Controller
 * 
 * Uses Timer0 or Timer2 to generate the PWM. Has 255 different speeds (0-254).
 * Fails low if the compare register is set too late. Fixes on next count
 * cycle.
 */

#include "MotorController.h"


MOTOR_REF motors;
int aSpeed = 0;
int bSpeed = 0;

void setup() {
  motorInit(motors, T0_MOTOR, 6, 5);
  motorASpeed(motors, aSpeed);
}

void loop() {
  delay(10);
  if (aSpeed >= 255)
    aSpeed = 0;
  motorASpeed(motors, ++aSpeed);
}

