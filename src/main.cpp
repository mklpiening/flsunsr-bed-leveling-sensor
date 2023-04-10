#include <Arduino.h>
#include "HX711.h"
HX711 loadcell;

const int MAX_VALUE = 800000;
const double NOISE_FACTOR = 3.5;
const int MIN_HIGH_STEPS = 4;

const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 2;

const double NOISE_SMOOTHING = 0.999;
const double LOAD_SMOOTHING = 0.5;

double last_value = 0.0; 

double noise = 180.0;

int high_steps = 0;

double smoothed_value = 0.0;

void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  Serial.begin(9600);

  loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  loadcell.set_scale(-459.542);
  loadcell.tare(); 
}

void loop() {
  double value = loadcell.get_value(1);
  
  high_steps++;

  if (abs(value) > MAX_VALUE)
  {
    return;
  }

  smoothed_value = LOAD_SMOOTHING * smoothed_value + (1.0 - LOAD_SMOOTHING) * value;

  double delta = smoothed_value - last_value;
  double absdelta = abs(delta);
  last_value = smoothed_value;

  Serial.print("last reading:\t");
  Serial.println(delta, 5);
  Serial.print("noise:\t");
  Serial.println(NOISE_FACTOR * noise, 5);

  if (-delta > NOISE_FACTOR * noise)
  {
    digitalWrite(13, HIGH);
    high_steps = 0;
  }

  if (absdelta > NOISE_FACTOR * noise || high_steps < MIN_HIGH_STEPS)
  {
    return;
  }

  digitalWrite(13, LOW);
  noise = NOISE_SMOOTHING * noise + (1.0 - NOISE_SMOOTHING) * absdelta;
}