/*
 * Theory of operation
 * a) blink a LED every 8 seconds
 * c) set PowerOn pin every 30 minutes (for 8 seconds)
 * d) if PowerOn set, reset after 8 seconds
 */

// !!!! disable millis for use of WDT_vect:
// /home/hgode/.arduino15/packages/MicroCore/hardware/avr/1.0.3/cores/microcore/core_settings.h
// https://github.com/MCUdude/MicroCore/issues/23

#include <avr/interrupt.h>
#include <avr/sleep.h>

#define LED_PIN PB0
#define POWER_PIN PB1

volatile int BlinkCounter=0;
#define LED_BLINK_INTERVAL 2

volatile byte PowerCounter=0;

#define MYDEBUG
#undef MYDEBUG

#ifdef MYDEBUG
  //15 = 2 Minutes = 8sec * 15
  #define POWER_INTERVAL 2
#else
  //225 = 30 Minuten = 8sec * 225
  #define POWER_INTERVAL 225
#endif
volatile bool powerIsOn=false;

//will set power on for 8 seconds
volatile byte PowerOnCounter=0;
#define POWER_ON_TIME 1

ISR(WDT_vect) {
  BlinkCounter++;
  PowerCounter++;
  if(powerIsOn)
    PowerOnCounter++;
  // Toggle Port B pin 0 output state
  PORTB ^= 1<<LED_PIN;
  delay(100);
  PORTB ^= 1<<LED_PIN;

  if (PowerCounter > POWER_INTERVAL) // if interrupted 225 times ( after 225*8sec = 30min )
  {
    PowerCounter=0;
    PowerOnCounter=0;
    digitalWrite(POWER_PIN, HIGH);
    powerIsOn=true;
    delay(2000);
    digitalWrite(POWER_PIN, LOW);
  }
/*
  if(PowerOnCounter>POWER_ON_TIME){
      digitalWrite(POWER_PIN, LOW);
      powerIsOn=false;
      PowerOnCounter=0;    
  }
*/ 
/* 
  if(BlinkCounter>LED_BLINK_INTERVAL){
    BlinkCounter=0;
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
  }
*/
}

void setup(){
  // Set up Port B as Input
  DDRB = 0; // usually not necessary but it will save current
  // Set Port B pin 3 mode back to output
  DDRB = 1<<DDB0;
  //set POWER_PIN as output
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, LOW);
  //WDPx watchdog prescaler (4bits), max = 1024K cycles @ 125kHz = 8 seconds
  //set timer to 1 sec
  //WDTCR |= (0<<WDP3) | (1<<WDP2) | (1<<WDP1) | (0<<WDP0);
  // set timer to 0.5s
  // WDTCR |= (1<<WDP2) | (1<<WDP0);
  // set timer to 4 sec
  //WDTCR |= (1<<WDP3);
  // set timer to 8 sec
  WDTCR |= (1<<WDP3 )|(0<<WDP2 )|(0<<WDP1)|(1<<WDP0); // 8s
  
  // Set watchdog timer in interrupt mode
  WDTCR |= (1<<WDTIE);
  WDTCR |= (0<<WDE);
  sei(); // Enable global interrupts
  
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

void loop() {
  sleep_mode();   // go to sleep and wait for interrupt...
}
