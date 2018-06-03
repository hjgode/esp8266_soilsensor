# esp8266_soilsensor
A wireless soil humidity sensor with ESP 8266, ATtiny13A and solar cell

Solar cell->batteries->ATtiny and step-up booster pololu->ESP8266 (esp-12) and capactive soil sensor.

The ATtiny runs in low power mode using the watchdog wakeup every 8 seconds. After 225 wakeups, the ATtiny sets a port pin to high and enables the step-up booster. The esp-12 and the soil sensor are only powered evry 30 minutes for a single, successful MQTT push.

ATtiny only consumes very low power (5µA or so) and down to 1.8V. The battery will last a long time.

A solar cell (4V/35mA) will charge the batteries for an even longer run time.

The esp-12 will consume up to 250mA, so it is only powered for a short time. Same for the capacitive soil sensor, which consumes about 20mA. Both devices are powered by the pololu step-up booster, which will run starting at 0.9V and is enabled only for a very short time (about 4-8 seconds).
