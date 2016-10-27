/**
 * Motor Controller Functions
 */

#include "MotorController.h"

#include <Arduino.h>

#define GET_DDR(P) ((P >= 2 && P <= 7) ? &DDRD : (P >= 8 && P <= 13) ? &DDRB : 0)
#define GET_PORT(P) ((P >= 2 && P <= 7) ? &PORTD : (P >= 8 && P <= 13) ? &PORTB : 0)
#define GET_MASK(P) (1 << (P % 8))
#define PIN_ON(PORT, M) (PORT | M)
#define PIN_OFF(PORT, M) (PORT & ~M)

/**
 * Initialize the motor pins
 *
 * Returns 0 on success, -1 on error
 */
int motorInit(MOTOR_REF &ref, TIMER timer, int pin_a, int pin_b) {

  // Get aitive pin info
  ref.a.ddr = GET_DDR(pin_a);
  ref.a.port = GET_PORT(pin_a);
  ref.a.mask = GET_MASK(pin_a);

  // Get bative pin info
  ref.b.ddr = GET_DDR(pin_b);
  ref.b.port = GET_PORT(pin_b);
  ref.b.mask = GET_MASK(pin_b);

  // Get timer info
  ref.t = timer;

  // Initialize pins
  if (ref.a.ddr == 0 || ref.a.port == 0 || ref.b.ddr == 0 || ref.b.port == 0)
    return -1;
  *ref.a.ddr = PIN_ON(*ref.a.ddr, ref.a.mask);
  *ref.b.ddr = PIN_ON(*ref.b.ddr, ref.b.mask);

  /////////////////////////////////////////////////////////////
  // Initialize timer
  if (ref.t.TCCRnA == 0 || ref.t.TCCRnB == 0 || ref.t.OCRnA == 0 || ref.t.OCRnB == 0)
    return -1;

  // Clear everything
  *ref.t.TCCRnA = 0;
  *ref.t.TCCRnB = 0;

  /////////////////////////////////////////////////////////////
  // TCCRnA
  // Set the COM0A and COM0B bits to an inverse wave (fail low)
  // COMnA[1:0] = TCCRnA[7:6]
  // COMnB[1:0] = TCCRnA[5:4]
  // 00 - Normal port operation, OC0x disconnected
  // 01 - Toggle OC0x on compare (50% duty)
  // 10 - Clear OC0x on compare (high to low)
  // 11 - Set 0C0x on compare (low to high)
  //
  // Set WGM bits to use fast pwm and go from BOTTOM to MAX (overflow)
  // WGMn[2] = TCCRnB[3]
  // WGMn[1:0] = TCCRnA[1:0]
  //
  // 000 - Normal mode
  // 001 - PWM phase correct
  // 010 - CTC, OCRx to MAX
  // 011 - Fast PWM
  // 100 - Reserved
  // 101 - PWM phase correct, BOTTOM to OCRx (update OCRx at TOP)
  // 110 - Reserved
  // 111 - Fast PWM, BOTTOM to OCRx (update OCRx at BOTTOM)
  *ref.t.TCCRnA = (*ref.t.TCCRnA) | 0b11000011;

  /////////////////////////////////////////////////////////////
  // TCCRnB
  // Enable the timer and set the prescaler
  // WGMn[2] = TCCRnB[3]
  // CSn[2:0] = TCCRnB[2:0]
  //
  // 000 - No clock source, Timer disabled
  // 001 - No prescaling
  // 010 - clk/8
  // 011 - clk/64
  // 100 - clk/256
  // 101 - clk/1024
  // 110 - External clock (falling edge)
  // 111 - External clock (rising edge)
  *ref.t.TCCRnB = (*ref.t.TCCRnB) | 0b00000100;

  // Set compare registers to MAX (always low)
  *ref.t.OCRnA = 255;
  *ref.t.OCRnB = 255;

  return 0;
}

/**
 * Unsets the motor pins and turns off the timer
 *
 * Returns 0 on success, -1 on error
 */
int motorDel(MOTOR_REF &ref) {
  if (ref.a.ddr == 0 || ref.a.port == 0 || ref.b.ddr == 0 || ref.b.port == 0)
    return -1;
  if (ref.t.TCCRnA == 0 || ref.t.TCCRnB == 0 || ref.t.OCRnA == 0 || ref.t.OCRnB == 0)
    return -1;

  // Unset the pins
  *ref.a.port = PIN_OFF(*ref.a.port, ref.a.mask);
  *ref.b.port = PIN_OFF(*ref.b.port, ref.b.mask);
  *ref.a.ddr = PIN_OFF(*ref.a.ddr, ref.a.mask);
  *ref.b.ddr = PIN_OFF(*ref.b.ddr, ref.b.mask);

  // Turn off the timer
  *ref.t.TCCRnA = 0;
  *ref.t.TCCRnB = 0;
  *ref.t.OCRnA = 0;
  *ref.t.OCRnB = 0;
  
  return 0;
}

/**
 * Sets the speed of motor A
 *
 * Returns 0 on success, -1 on error
 */
int motorASpeed(MOTOR_REF &ref, int s) {
  if (ref.a.ddr == 0 || ref.a.port == 0 || ref.b.ddr == 0 || ref.b.port == 0)
    return -1;
  if (ref.t.TCCRnA == 0 || ref.t.TCCRnB == 0 || ref.t.OCRnA == 0 || ref.t.OCRnB == 0)
    return -1;

  // Set the speed
  int v = 255 - s;
  if (v <= 0) v = 1;
  else if (v > 255) v = 255;
  *ref.t.OCRnA = v;
  
  return 0;
}

/**
 * Sets the speed of motor B
 *
 * Returns 0 on success, -1 on error
 */
int motorBSpeed(MOTOR_REF &ref, int s) {
  if (ref.a.ddr == 0 || ref.a.port == 0 || ref.b.ddr == 0 || ref.b.port == 0)
    return -1;
  if (ref.t.TCCRnA == 0 || ref.t.TCCRnB == 0 || ref.t.OCRnA == 0 || ref.t.OCRnB == 0)
    return -1;

  // Set the speed
  int v = 255 - s;
  if (v <= 0) v = 1;
  else if (v > 255) v = 255;
  *ref.t.OCRnB = v;
  
  return 0;
}

/**
 * Stops motor A
 *
 * Returns 0 on success, -1 on error
 */
int motorAStop(MOTOR_REF &ref) {
  if (ref.a.ddr == 0 || ref.a.port == 0 || ref.b.ddr == 0 || ref.b.port == 0)
    return -1;
  if (ref.t.TCCRnA == 0 || ref.t.TCCRnB == 0 || ref.t.OCRnA == 0 || ref.t.OCRnB == 0)
    return -1;

  // Stop the motor
  *ref.t.OCRnA = 255;

  return 0;
}

/**
 * Stops motor B
 *
 * Returns 0 on success, -1 on error
 */
int motorBStop(MOTOR_REF &ref) {
  if (ref.a.ddr == 0 || ref.a.port == 0 || ref.b.ddr == 0 || ref.b.port == 0)
    return -1;
  if (ref.t.TCCRnA == 0 || ref.t.TCCRnB == 0 || ref.t.OCRnA == 0 || ref.t.OCRnB == 0)
    return -1;

  // Stop the motor
  *ref.t.OCRnB = 255;

  return 0;
}

