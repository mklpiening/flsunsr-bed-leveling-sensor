#include <Arduino.h>
#include "HX711.h"
HX711 loadcell;

const int MAX_VALUE = 800000;
const double NOISE_FACTOR = 3.5; // threshold
const int MIN_HIGH_STEPS = 2;

const int LOADCELL_DOUT_PIN = 3;
const int LOADCELL_SCK_PIN = 2;

const double NOISE_SMOOTHING = 0.999;
const double LOAD_SMOOTHING = 0.7;

const double ABS_THRESHOLD = 3000;

const int TRIGGER_RESET_HIGH_STEPS = 100;
int reset_high_steps = 0;

const int KEEP_HIGH = 4;

double last_value = 0.0; 

double noise = 180.0;

int high_steps = 0;
int keep_high_steps = 0;

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
  if (reset_high_steps > TRIGGER_RESET_HIGH_STEPS)
  {
    loadcell.tare(); 

    smoothed_value = 0.0;
    noise = 180.0;
    high_steps = 0;
    last_value = 0.0;
    keep_high_steps = 0.0;

    reset_high_steps = 0;
    return;
  }

  double value = loadcell.get_value(2);
  

  if (abs(value) > MAX_VALUE)
  {
    reset_high_steps++;
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

  if (-delta > NOISE_FACTOR * noise || -smoothed_value > ABS_THRESHOLD)
  {
    high_steps++;
  }
  else
  {
    high_steps = 0;
  }


  if (high_steps >= MIN_HIGH_STEPS)
  {
    digitalWrite(13, HIGH);
    reset_high_steps++;
    keep_high_steps = 0;
    return;
  }

  // keep high signal for at least KEEP_HIGH
  if (keep_high_steps < KEEP_HIGH)
  {
    digitalWrite(13, HIGH);
    reset_high_steps++;
    keep_high_steps++;
    return;
  }

  digitalWrite(13, LOW);
  reset_high_steps = 0;
  noise = NOISE_SMOOTHING * noise + (1.0 - NOISE_SMOOTHING) * absdelta;
}
