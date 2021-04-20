this is inspired by adafruit's ice tube clock but adopting that design
into a more modern, general-purpose display unit.

1. use serial for data input, use as a simple display
2. use a atmega328pb chip
3. don't include unnecessary components
4. run off of 5v (3 wires needed: 1 5v, 2 gnd, 3 serial data)
5. use surface mount components
6. (may be a mistake) keep the tube inline with the main circuit.

There have been 2 revisions to the hardware but the general pinout stays the same.
v1 of the hardware incorporates the high voltage circuits and a low profile high voltage driver.
v2 of the hardware removes the high voltage circuits off the main board and uses a much larger
  package for the high voltage driver.

The first use case of this design will be to connect it to a wifi co-processor to show data
from the internet in real time.

Code for the co-processor will be added alongside more documentation around the hardware itself.
