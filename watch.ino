#include <interrupt.h>
#include <power.h>
#include <sleep.h>
#include <wdt.h>
#define sbi(port, bit) {port |= (1 << bit);}
#define cbi(port, bit) {port &= ~(1 << bit);}
#define tbi(port, bit) {port ^= (1 << bit);}

int analog_treshold = 400;
int mode_change_delay = 1000;
bool button_pressed = false;
unsigned int pressed_button_counter = 0;
unsigned int actual_button_counter = 0;
unsigned int push_time = 0;
unsigned int time_seconds = 55;
unsigned int time_minutes = 59;
unsigned int time_hours = 11;
unsigned int time_seconds_alarm = 0;
unsigned int time_minutes_alarm = 0;
unsigned int time_hours_alarm = 0;
bool PM = false;
bool PM_alarm = true;
unsigned long time_counter = 0;
unsigned long temp = 0;
int mode = 0;
int delay_time = 200;
int button = 0;
int beep_counter = 0;
bool mode_was_changed = false;
bool turn_off_alarm = true;
bool alarm_set = true;
bool first_time = true;
int delay_alarm_count = 0;

void show_time_mode();
void set_hour_mode();
void set_minute_mode();
void show_time_to_set_hour();
void set_hour_to_set_minute();
void set_minute_to_show_time();
void show_time_alarm_to_set_alarm();
void show_time_alarm_mode();
void set_alarm_to_set_hour_alarm();
void set_hour_alarm_to_set_minute_alarm();
void set_minute_alarm_to_show_time();
void set_alarm_mode();
void set_hour_alarm_mode();
void set_minute_alarm_mode();


void setup() {
  //PCICR |= B00000010;                                                    
  //PCMSK1 |= B00000011;  
 // power_adc_enable(); // ADC converter
  ADCSRA &= ~(1 << 7);
  power_spi_disable(); // SPI
  power_usart0_disable();// Serial (USART) 
  power_timer2_disable();// Timer 2
  power_twi_disable(); // TWI (I2C)
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TIMSK1 = 0;
  TCCR1B |= (1 << WGM12) | (1 << CS12);
  OCR1A = 31250;
  TIMSK1 = (1 << OCIE1A);
  sei();
  DDRD = 0;   //Set pins D2-D7 as outputs
  DDRB = 0;   //Set pins D8-D13 as outputs
  time_counter = millis();
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
}

void beep() {
  sbi(DDRB, 5);
  sbi(PORTB, 5);
  delay(delay_time / 2);
  cbi(PORTB, 5);
  cbi(DDRB, 5);
  delay(delay_time / 2 );
}

void loop() {
  if (time_hours == time_hours_alarm && PM == PM_alarm && alarm_set) {
    if (first_time) {
      turn_off_alarm = false;
      first_time = false;
    }
  if (!turn_off_alarm && time_minutes == time_minutes_alarm) beep();
  else if (time_minutes == time_minutes_alarm + 1) {
      first_time = true;
      if (turn_off_alarm || delay_alarm_count == 5) {
        time_minutes_alarm -= 2 * delay_alarm_count;
        delay_alarm_count = 0;
      }
      else time_minutes_alarm += 2;
      turn_off_alarm = true;
    }
  }
  if (digitalRead(A0) == HIGH  && !button_pressed) {
    button_pressed = true;
    pressed_button_counter = millis();    
    button = 1;
  }
  if (digitalRead(A0) == LOW  && button_pressed && button == 1) {
    button_pressed = false;
    if (mode == 0 && !mode_was_changed) show_time_mode();
    if (mode == 1 && !mode_was_changed) set_hour_mode();
    if (mode == 2 && !mode_was_changed) set_minute_mode();
    mode_was_changed = false;
  }
  if (button_pressed && button == 1) {
    actual_button_counter = millis();
    push_time = actual_button_counter - pressed_button_counter;
  }
  if (push_time * 2 > mode_change_delay && button_pressed && button == 1 && mode == 0) show_time_to_set_hour();
  if (push_time * 2 > mode_change_delay && button_pressed && button == 1 && mode == 1) set_hour_to_set_minute();
  if (push_time * 2 > mode_change_delay && button_pressed && button == 1 && mode == 2) set_minute_to_show_time();

  if (digitalRead(A1) == HIGH  && !button_pressed) {
    button_pressed = true;
    pressed_button_counter = millis();    
    button = 2;
  }
  if (digitalRead(A1) == LOW  && button_pressed && button == 2) {
    button_pressed = false;
    if (mode == 0 && !mode_was_changed) show_time_alarm_mode();
    if (mode == 3 && !mode_was_changed) set_alarm_mode();
    if (mode == 4 && !mode_was_changed) set_hour_alarm_mode();
    if (mode == 5 && !mode_was_changed) set_minute_alarm_mode();
    mode_was_changed = false;
  }
  if (button_pressed && button == 2) {
    actual_button_counter = millis();
    push_time = actual_button_counter - pressed_button_counter;
  }
  
  if (push_time * 2 > mode_change_delay && button_pressed && button == 2 && mode == 0) {
    if (turn_off_alarm) show_time_alarm_to_set_alarm();
    else {
      turn_off_alarm = true;
      pressed_button_counter = millis();
    }
  }
  if (push_time * 2 > mode_change_delay && button_pressed && button == 2 && mode == 3) set_alarm_to_set_hour_alarm();
  if (push_time * 2 > mode_change_delay && button_pressed && button == 2 && mode == 4) set_hour_alarm_to_set_minute_alarm();
  if (push_time * 2 > mode_change_delay && button_pressed && button == 2 && mode == 5) set_minute_alarm_to_show_time();
}

ISR(TIMER1_COMPA_vect) {
  time_seconds ++;
  if (time_seconds > 59) {
    time_minutes ++;
    time_seconds = 0;
  }
  if (time_minutes > 59) {
    time_hours ++;
    time_minutes = 0;
  }
  if (time_hours > 11) {
    time_hours = 0;
    PM = !PM;
  }
}

void show_time_mode() {
  DDRB = B00000100;
  DDRD = 0;
  PORTB = 0;
  PORTD &= B00000011;
  if (PM) sbi(PORTB, 2);
  switch (time_hours) {
    case 0:  sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTD, 6); break;
    case 1:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 7); break;
    case 2:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 6); break;
    case 3:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 4:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTD, 7); break;
    case 5:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 6:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 0); break;
    case 7:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 8:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTD, 6); break;
    case 9:  sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 10: sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTD, 7); break;
    default: sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTB, 1); break;
  }
  switch ((int)(time_minutes / 5)) {
    case 0:  sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); break;
    case 1:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 3); break;
    case 2:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 2); break;
    case 3:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 4:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 3); break;
    case 5:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 6:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 4); break;
    case 7:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 8:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 2); break;
    case 9:  sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 10: sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 3); break;
    default: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 5); break;
  }
  delay(1000);
  DDRB = 0;
  DDRD = 0;
  PORTD &= B00000011;
  PORTB = 0;
}

void set_hour_mode() {
  time_hours ++;
  DDRB = B00000100;
  DDRD = 0;
  PORTB = 0;
  PORTD &= B00000011;
  if (PM) sbi(PORTB, 2);
  switch (time_hours) {
    case 0:  sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTD, 6); break;
    case 1:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 7); break;
    case 2:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 6); break;
    case 3:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 4:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTD, 7); break;
    case 5:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 6:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 0); break;
    case 7:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 8:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTD, 6); break;
    case 9:  sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 10: sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTD, 7); break;
    case 11: sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTB, 1); break;
    default: sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTD, 6); if (!PM) sbi(PORTB, 2) else cbi(PORTB, 2); break;
  }
}

void set_minute_mode() {
  time_minutes += 5;
  DDRB = 0;
  DDRD = 0;
  PORTD &= B00000011;
  PORTB = 0;
  switch ((int)(time_minutes / 5)) {
    case 0:  sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); break;
    case 1:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 3); break;
    case 2:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 2); break;
    case 3:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 4:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 3); break;
    case 5:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 6:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 4); break;
    case 7:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 8:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 2); break;
    case 9:  sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 10: sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 3); break;
    case 11: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 5); break;
    default: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); time_minutes %= 60; break;
  }
}

void show_time_to_set_hour() {
  mode = 1;
  DDRB = B00000100;
  DDRD = 0;
  PORTB = 0;
  PORTD &= B00000011;
  if (PM) sbi(PORTB, 2);
  switch (time_hours) {
    case 0:  sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTD, 6); break;
    case 1:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 7); break;
    case 2:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 6); break;
    case 3:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 4:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTD, 7); break;
    case 5:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 6:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 0); break;
    case 7:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 8:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTD, 6); break;
    case 9:  sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 10: sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTD, 7); break;
    default: sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTB, 1); break;
  }
  button_pressed = false;
  mode_was_changed = true;
}

void set_hour_to_set_minute() {
  mode = 2;
  DDRB = 0;
  DDRD = 0;
  PORTD &= B00000011;
  PORTB = 0;
  switch ((int)(time_minutes / 5)) {
    case 0:  sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); break;
    case 1:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 3); break;
    case 2:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 2); break;
    case 3:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 4:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 3); break;
    case 5:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 6:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 4); break;
    case 7:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 8:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 2); break;
    case 9:  sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 10: sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 3); break;
    case 11: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 5); break;
    default: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); break;
  }
  button_pressed = false;
  mode_was_changed = true;
}

void set_minute_to_show_time() {
  mode = 0;
  DDRB = 0;
  DDRD = 0;
  PORTD &= B00000011;
  PORTB = 0;
  button_pressed = false;
  mode_was_changed = true;
}

void show_time_alarm_mode() {
  DDRB = B00000100;
  DDRD = 0;
  PORTB = 0;
  PORTD &= B00000011;
  if (alarm_set) {
    sbi(DDRB, 4);
    sbi(PORTB, 4);
  }
  if (PM_alarm) sbi(PORTB, 2);
  switch (time_hours_alarm) {
    case 0:  sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTD, 6); break;
    case 1:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 7); break;
    case 2:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 6); break;
    case 3:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 4:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTD, 7); break;
    case 5:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 6:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 0); break;
    case 7:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 8:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTD, 6); break;
    case 9:  sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 10: sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTD, 7); break;
    default: sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTB, 1); break;
  }
  switch ((int)(time_minutes_alarm / 5)) {
    case 0:  sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); break;
    case 1:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 3); break;
    case 2:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 2); break;
    case 3:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 4:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 3); break;
    case 5:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 6:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 4); break;
    case 7:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 8:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 2); break;
    case 9:  sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 10: sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 3); break;
    default: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 5); break;
  }
  delay(1000);
  DDRB = 0;
  DDRD = 0;
  PORTD &= B00000011;
  PORTB = 0;
}

void set_alarm_mode() {
  DDRB = 0;
  DDRD = 0;
  PORTD &= B00000011;
  PORTB = 0;
  alarm_set = !alarm_set;
  if (alarm_set) {
    sbi(DDRB, 4);
    sbi(PORTB, 4);
    //turn_off_alarm = false;
  }
  else {
    cbi(PORTB, 4);
    cbi(DDRB, 4);
  }
}

void set_hour_alarm_mode() {
  time_hours_alarm ++;
  DDRB = B00000100;
  DDRD = 0;
  PORTB = 0;
  PORTD &= B00000011;
  if (PM_alarm) sbi(PORTB, 2);
  switch (time_hours_alarm) {
    case 0:  sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTD, 6); break;
    case 1:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 7); break;
    case 2:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 6); break;
    case 3:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 4:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTD, 7); break;
    case 5:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 6:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 0); break;
    case 7:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 8:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTD, 6); break;
    case 9:  sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 10: sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTD, 7); break;
    case 11: sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTB, 1); break;
    default: sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTD, 6); PM_alarm = !PM_alarm; if (PM_alarm) sbi(PORTB, 2) else cbi(PORTB, 2); time_hours_alarm = 0; break;
  }
}

void set_minute_alarm_mode() {
  time_minutes_alarm += 5;
  DDRB = 0;
  DDRD = 0;
  PORTD &= B00000011;
  PORTB = 0;
  switch ((int)(time_minutes_alarm / 5)) {
    case 0:  sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); break;
    case 1:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 3); break;
    case 2:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 2); break;
    case 3:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 4:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 3); break;
    case 5:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 6:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 4); break;
    case 7:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 8:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 2); break;
    case 9:  sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 10: sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 3); break;
    case 11: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 5); break;
    default: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); time_minutes_alarm %= 60; break;
  }
}

void show_time_alarm_to_set_alarm() {
  mode = 3;
  DDRB = 0;
  DDRD = 0;
  PORTD &= B00000011;
  PORTB = 0;
  if (alarm_set) {
    sbi(DDRB, 4);
    sbi(PORTB, 4);
  }
  button_pressed = false;
  mode_was_changed = true;
}

void set_alarm_to_set_hour_alarm() {
  mode = 4;
  DDRB = B00000100;
  DDRD = B00000100;
  PORTB = 0;
  PORTD &= B00000011;
  if (PM_alarm) sbi(PORTB, 2);
  switch (time_hours_alarm) {
    case 0:  sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTD, 6); break;
    case 1:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 7); break;
    case 2:  sbi(DDRD, 6); sbi(DDRD, 7); sbi(PORTD, 6); break;
    case 3:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 4:  sbi(DDRD, 7); sbi(DDRB, 0); sbi(PORTD, 7); break;
    case 5:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 6:  sbi(DDRB, 0); sbi(DDRB, 1); sbi(PORTB, 0); break;
    case 7:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTB, 0); break;
    case 8:  sbi(DDRD, 6); sbi(DDRB, 0); sbi(PORTD, 6); break;
    case 9:  sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTB, 1); break;
    case 10: sbi(DDRD, 7); sbi(DDRB, 1); sbi(PORTD, 7); break;
    default: sbi(DDRD, 6); sbi(DDRB, 1); sbi(PORTB, 1); break;
  }
  button_pressed = false;
  mode_was_changed = true;
}

void set_hour_alarm_to_set_minute_alarm() {
  mode = 5;
  DDRB = 0;
  DDRD = 0;
  PORTB = 0;
  PORTD &= B00000011;
  switch ((int)(time_minutes_alarm / 5)) {
    case 0:  sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); break;
    case 1:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 3); break;
    case 2:  sbi(DDRD, 2); sbi(DDRD, 3); sbi(PORTD, 2); break;
    case 3:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 4:  sbi(DDRD, 3); sbi(DDRD, 4); sbi(PORTD, 3); break;
    case 5:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 6:  sbi(DDRD, 4); sbi(DDRD, 5); sbi(PORTD, 4); break;
    case 7:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 4); break;
    case 8:  sbi(DDRD, 2); sbi(DDRD, 4); sbi(PORTD, 2); break;
    case 9:  sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 5); break;
    case 10: sbi(DDRD, 3); sbi(DDRD, 5); sbi(PORTD, 3); break;
    case 11: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 5); break;
    default: sbi(DDRD, 2); sbi(DDRD, 5); sbi(PORTD, 2); break;
  }
  button_pressed = false;
  mode_was_changed = true;
}

void set_minute_alarm_to_show_time() {
  mode = 0;
  DDRB = 0;
  DDRD = 0;
  PORTB = 0;
  PORTD &= B00000011;
  button_pressed = false;
  mode_was_changed = true;
}
