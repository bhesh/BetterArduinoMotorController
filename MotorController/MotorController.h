#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <stdint.h>

struct TIMER {
  volatile uint8_t *TCCRnA;
  volatile uint8_t *TCCRnB;
  volatile uint8_t *OCRnA;
  volatile uint8_t *OCRnB;
};

const TIMER T0_MOTOR = { &TCCR0A, &TCCR0B, &OCR0A, &OCR0B };
const TIMER T2_MOTOR = { &TCCR2A, &TCCR2B, &OCR2A, &OCR2B };

struct PIN_REF {
 volatile uint8_t *ddr;
  volatile uint8_t *port;
  uint8_t mask;
};

struct MOTOR_REF {
  PIN_REF a;
  PIN_REF b;
  TIMER t;
};

int motorInit(MOTOR_REF &ref, TIMER timer, int pin_a, int pin_b);
int motorDel(MOTOR_REF &ref);
int motorASpeed(MOTOR_REF &ref, int s);
int motorBSpeed(MOTOR_REF &ref, int s);
int motorAStop(MOTOR_REF &ref);
int motorBStop(MOTOR_REF &ref);

#endif
