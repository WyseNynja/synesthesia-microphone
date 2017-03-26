#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define PIN            20
#define NUMPIXELS      10
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// GUItool: begin automatically generated code
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
//#include <SerialFlash.h>

AudioInputI2S             i2s1;           //xy=61,319
AudioFilterStateVariable  filter1;        //xy=176,319
AudioFilterStateVariable  filter2;        //xy=294,339
AudioFilterStateVariable  filter3;        //xy=422,358
AudioFilterStateVariable  filter4;        //xy=545,352
AudioOutputI2S            i2s2;           //xy=662,295
AudioAnalyzeNoteFrequency notefreq1;      //xy=670,339
AudioConnection           patchCord1(i2s1, 0, filter1, 0);
AudioConnection           patchCord2(filter1, 2, filter2, 0);
AudioConnection           patchCord3(filter2, 2, filter3, 0);
AudioConnection           patchCord4(filter3, 0, filter4, 0);
AudioConnection           patchCord5(filter4, 0, notefreq1, 0);
AudioConnection           patchCord6(filter4, 0, i2s2, 0);
AudioConnection           patchCord7(filter4, 0, i2s2, 1);
AudioControlSGTL5000      sgtl5000_1;     //xy=671,465

AudioAnalyzeRMS           rms_L;
AudioConnection           patchCord8(filter4, rms_L);
// GUItool: end automatically generated code

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
    sgtl5000_1.micGain(45);

    /*
     *  Initialize the yin algorithm's absolute
     *  threshold, 0.15 is good number.
     */
    notefreq1.begin(0.15);
}

elapsedMillis fps;
uint16_t red_current = 0;
uint16_t green_current = 0;
uint16_t blue_current = 0;

uint16_t red_destination = 0;
uint16_t green_destination = 0;
uint16_t blue_destination = 0;

const uint16_t incrementAmount = 8000;

unsigned int currentColor = 0;
float level = 0.0;

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

    // get rid of any noise
    if (current <= incrementAmount && dest <= incrementAmount) {
      current = 0;
    }

    return current;
}

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
            case 0:   currentColor = COLOR_C; break; // C
            case 1:   currentColor = COLOR_C; break; // C#
            case 2:   currentColor = COLOR_D; break; // D
            case 3:   currentColor = COLOR_D; break; // D#
            case 4:   currentColor = COLOR_E; break; // E
            case 5:   currentColor = COLOR_F; break; // F
            case 6:   currentColor = COLOR_F; break; // F#
            case 7:   currentColor = COLOR_G; break; // G
            case 8:   currentColor = COLOR_G; break; // G#
            case 9:   currentColor = COLOR_A; break; // A
            case 10:  currentColor = COLOR_B; break; // A#
            case 11:  currentColor = COLOR_B; break; // B
        }        
    }

    if (fps > 24) {
        fps = 0;

        level = rms_L.read() * 1.0;
        red_destination   = uint16_t((((currentColor >> 16) & 0xff) << 8 ) * level);
        green_destination = uint16_t((((currentColor >> 8)  & 0xff) << 8 ) * level);
        blue_destination  = uint16_t((((currentColor >> 0)  & 0xff) << 8 ) * level);

        red_current = increment(red_current, red_destination);
        green_current = increment(green_current, green_destination);
        blue_current = increment(blue_current, blue_destination);

        for(int i=0; i<NUMPIXELS; i++){
            pixels.setPixelColor(i, pixels.Color(byte(red_current >> 8), byte(green_current >> 8), byte(blue_current >> 8)));
        }
//        Serial.printf("%i %i %i\n", red_current, green_current, blue_current);
        pixels.show(); // This sends the updated pixel color to the hardware.
    }
}
