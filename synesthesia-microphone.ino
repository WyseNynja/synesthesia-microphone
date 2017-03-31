#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN            20
#define NUMPIXELS      10
#define NOISE_FLOOR    30 // aka -<x> decibels
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioInputI2S             i2s1;
AudioFilterStateVariable  filter1;
AudioFilterStateVariable  filter2;
AudioFilterStateVariable  filter3;
AudioFilterStateVariable  filter4;
AudioOutputI2S            i2s2;
AudioAnalyzeNoteFrequency notefreq1;
AudioAnalyzeRMS           rms_L;
AudioControlSGTL5000      sgtl5000_1;
AudioConnection           patchCord1(i2s1, 0, filter1, 0);
AudioConnection           patchCord2(filter1, 2, filter2, 0);
AudioConnection           patchCord3(filter2, 2, filter3, 0);
AudioConnection           patchCord4(filter3, 0, filter4, 0);
AudioConnection           patchCord5(filter4, 0, notefreq1, 0);
AudioConnection           patchCord6(filter4, 0, i2s2, 0); // L headphone jack
AudioConnection           patchCord7(filter4, 0, i2s2, 1); // R headphone jack
AudioConnection           patchCord8(filter4, rms_L);

// 24 Bit RGB values for the LEDs and associated musical note
const unsigned int COLOR_C = 0xff0000;
const unsigned int COLOR_D = 0x0000ff;
const unsigned int COLOR_E = 0xffff00;
const unsigned int COLOR_F = 0x00ff80;
const unsigned int COLOR_G = 0x00ff00;
const unsigned int COLOR_A = 0xff8000;
const unsigned int COLOR_B = 0xff00ff;

void setup() {
  AudioMemory(30);

  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  pixels.begin(); // This initializes the NeoPixel library.
    
  filter1.frequency(200);
  filter2.frequency(200);
  filter3.frequency(2000);
  filter4.frequency(2000);

  filter1.resonance(1.5);
  filter2.resonance(1.5);
  filter3.resonance(1.5);
  filter4.resonance(1.5);
    
  sgtl5000_1.enable();
  sgtl5000_1.volume(0.9);
  sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  sgtl5000_1.micGain(45); // value from 0 to 63

  // Initialize the yin algorithm's absolute threshold, 0.15 is good number.
  notefreq1.begin(0.15);
}

elapsedMillis fps;  // Keep track of the time

// The RGB values sent to the lights
uint16_t red_current = 0;
uint16_t green_current = 0;
uint16_t blue_current = 0;

// The value as read by the input
// We want to drive the current value towards the destination
uint16_t red_destination = 0;
uint16_t green_destination = 0;
uint16_t blue_destination = 0;

// Move the current value towards the destination by this amount
const uint16_t incrementAmount = 4000;

uint16_t increment(uint16_t current, uint16_t dest) {
  if (current != dest) {
    if (current < dest) {
      if (0xffff - incrementAmount < current) {
        current = 0xffff;
      } else {
        if (current + incrementAmount > dest) {
          current = dest;
        } else {
          current += incrementAmount;
        }
      }
    } else {
      if (current < incrementAmount) {
        current = dest;
      } else {
        if (current - incrementAmount < dest) {
          current = dest;
        } else {
          current -= incrementAmount;
        }
      }
    }
  }

  if (current <= 1000 && dest <= 1000) {
    current = 0;
  }
  
  return current;
}

unsigned int current_color = 0;
void loop() {
    if (notefreq1.available()) {
        // Read the fundamental frequency
        float note = notefreq1.read();

        // Convert the frequency into its MIDI equivalent where A440 = 69, A880 = 81, etc...
        float midi = 69 + 12 * log(note / 440.0) / log(2);

        // Break down the note to a single octave, where C = 0, C# = 1, etc...
        int finalNote = int(midi) % 12;

        // Set the RGB destination value based on the note read
        switch (finalNote) {
            case 0:  current_color = COLOR_C; break; // C
            case 1:  current_color = COLOR_C; break; // C#
            case 2:  current_color = COLOR_D; break; // D
            case 3:  current_color = COLOR_D; break; // D#
            case 4:  current_color = COLOR_E; break; // E
            case 5:  current_color = COLOR_F; break; // F
            case 6:  current_color = COLOR_F; break; // F#
            case 7:  current_color = COLOR_G; break; // G
            case 8:  current_color = COLOR_G; break; // G#
            case 9:  current_color = COLOR_A; break; // A
            case 10: current_color = COLOR_B; break; // A#
            case 11: current_color = COLOR_B; break; // B
        }        
    }

    // Will run roughly every 25 - 27 milliseconds
    if (fps > 24) {
        fps = 0;

        // Read the level
        float level = rms_L.read() * 1.0;

        // Convert 0.0 to 1.0 to a decibel value
        level = log10(level) * 20 + NOISE_FLOOR;

        // Convert decibel to a float value where 0 is silent 1.0 is max
        level = level <= 0 ? 0: level / NOISE_FLOOR;

        // Compensate for LED brightness with an S Curve
        level = 1 / (1 + pow(2.71828, (-10 * (level - 0.5))));
        
        red_destination   = uint16_t((((current_color >> 16) & 0xff) << 8 ) * level);
        green_destination = uint16_t((((current_color >> 8)  & 0xff) << 8 ) * level);
        blue_destination  = uint16_t((((current_color >> 0)  & 0xff) << 8 ) * level);

        // Increment RGB
        red_current   = increment(red_current  , red_destination);
        green_current = increment(green_current, green_destination);
        blue_current  = increment(blue_current , blue_destination);

        // Set each pixel
        for(int i=0; i<NUMPIXELS; i++){
            pixels.setPixelColor(i, pixels.Color(byte(red_current >> 8), byte(green_current >> 8), byte(blue_current >> 8)));
        }

        // Write the pixels
        pixels.show(); // This sends the updated pixel color to the hardware.
    }
}
