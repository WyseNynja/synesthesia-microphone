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
#include <SerialFlash.h>

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
unsigned int COLOR_C = 0xff0000;
unsigned int COLOR_D = 0x0000ff;
unsigned int COLOR_E = 0xffff00;
unsigned int COLOR_F = 0x00ff80;
unsigned int COLOR_G = 0x00ff00;
unsigned int COLOR_A = 0xff8000;
unsigned int COLOR_B = 0xff00ff;

void setup() {
    #if defined (__AVR_ATtiny85__)
        if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
    #endif
    pixels.begin(); // This initializes the NeoPixel library.

    AudioMemory(30);
    
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

byte red = 0;
byte green = 0;
byte blue = 0;

// Take a 24 bit color value and set the RGB values
void setColor(unsigned int COLOR) {
    red   = (COLOR >> 16) & 0xff;
    green = (COLOR >> 8)  & 0xff;
    blue  = COLOR         & 0xff;
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
            case 0:  setColor(COLOR_C); break; // C
            case 1:  setColor(COLOR_C); break; // C#
            case 2:  setColor(COLOR_D); break; // D
            case 3:  setColor(COLOR_D); break; // D#
            case 4:  setColor(COLOR_E); break; // E
            case 5:  setColor(COLOR_F); break; // F
            case 6:  setColor(COLOR_F); break; // F#
            case 7:  setColor(COLOR_G); break; // G
            case 8:  setColor(COLOR_G); break; // G#
            case 9:  setColor(COLOR_A); break; // A
            case 10: setColor(COLOR_B); break; // A#
            case 11: setColor(COLOR_B); break; // B
        }
    }
    if (fps > 24) {
        fps = 0;
        float level = rms_L.read() * 1.0;
        for(int i=0; i<NUMPIXELS; i++){
            pixels.setPixelColor(i, pixels.Color(int(red * level), int(green * level), int(blue * level)));
        }
        pixels.show(); // This sends the updated pixel color to the hardware.
    }
}
