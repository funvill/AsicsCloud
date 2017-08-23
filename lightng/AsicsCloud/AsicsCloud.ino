
#include "FastLED.h" // https://github.com/FastLED/FastLED
#include <ArduinoLog.h> // https://github.com/thijse/Arduino-Log

FASTLED_USING_NAMESPACE

// Settings
// =============================================================================
const unsigned int SETTING_LOGGER_BAUDRATE = 9600;

/** 
* Logging levels: 
* 0 - LOG_LEVEL_SILENT     no output 
* 1 - LOG_LEVEL_FATAL      fatal errors 
* 2 - LOG_LEVEL_ERROR      all errors  
* 3 - LOG_LEVEL_WARNING    errors, and warnings 
* 4 - LOG_LEVEL_NOTICE     errors, warnings and notices 
* 5 - LOG_LEVEL_TRACE      errors, warnings, notices & traces 
* 6 - LOG_LEVEL_VERBOSE    all 
*/
#define SETTING_LOGGER_LOG_LEVEL LOG_LEVEL_VERBOSE
#define LOGGER_SERIAL Serial

// LEDS 
#define SETTING_LED_TYPE WS2811
#define SETTING_LED_COLOR_ORDER GRB
const unsigned char SETTING_LED_BRIGHTNESS = 255;
const unsigned char SETTING_LED_FRAMES_PER_SECOND = 120;
const unsigned char SETTING_LED_DATA_PIN = 6; 
static const unsigned short SETTING_BLOB_COUNT = 3 ;  
const unsigned short NUM_LEDS = 60;
CRGB leds[NUM_LEDS];

// Cloud 
const unsigned char SETTING_BLOB_FADE_SPEED = 160  ; 
const unsigned char SETTING_CLOUD_FADE_SPEED = 255 / 10 ; 

CRGB one = CRGB(0x33, 0xff, 0x33) ;
CRGB two = CRGB(0xFF, 0x48, 0x00) ;
CRGB three = CRGB(0x30, 0xa7, 0xff) ;

static CRGBPalette16 AsicsPalette ; 

class CBlob {
    public:
        CBlob() {
            this->Reset(); 
            // this->colorindex = random16(255) ; // Start the inital colorindex in a ramdom location. 
        }
        void Reset() {
            this->startingIndex = random16(NUM_LEDS);
            this->colorindex = 0 ; 
            Log.notice("CBlob.Reset startingIndex=[%d], colorindex=[%d]" CR, startingIndex, colorindex);    
        }
        void Step() {
            this->colorindex++;  // slowly cycle the "base color" through the rainbow
            if( this->colorindex >= 255 ) {
                this->Reset() ; 
            }  
        }

        unsigned short startingIndex; 
        unsigned char colorindex ; 
        
};
static CBlob blobs[SETTING_BLOB_COUNT];





void setup() {
  delay(3000); // 3 second delay for recovery

    // Start serial logging system
    // Set up serial port and wait until connected
    LOGGER_SERIAL.begin(SETTING_LOGGER_BAUDRATE);
    Log.begin(SETTING_LOGGER_LOG_LEVEL, &LOGGER_SERIAL);

    // Print version info
    Log.notice(F(CR "******************************************" CR));
    Log.notice("***          ASiCS Cloud                " CR);
    Log.notice("******************************************" CR);
    Log.notice("Written by: Steven Smethurst" CR);
    Log.notice("Last Updated: 2017 Aug 19th" CR CR);    

    // SEt up the LEDs
    Log.notice("FYI: Setting up the LEDs. SETTING_LED_DATA_PIN=%d, NUM_LEDS=%d" CR, SETTING_LED_DATA_PIN, NUM_LEDS);
    FastLED.addLeds<NEOPIXEL, SETTING_LED_DATA_PIN>(leds, 0, NUM_LEDS);

    // set master brightness control
    Log.notice("FYI: Setting brigtness to %d" CR, SETTING_LED_BRIGHTNESS);
    FastLED.setBrightness(SETTING_LED_BRIGHTNESS);

    CRGB oneHalf = one ; 
    oneHalf.fadeToBlackBy(255/2);
    CRGB twoHalf = one ; 
    twoHalf.fadeToBlackBy(255/2);
    CRGB threeHalf = one ; 
    threeHalf.fadeToBlackBy(255/2);
   AsicsPalette = CRGBPalette16(CRGB::Black,  CRGB::Black,  oneHalf,  oneHalf,
                                one, one, twoHalf, twoHalf,  
                                two, two, threeHalf,  threeHalf,    
                                three, three, CRGB::Black,  CRGB::Black );

    // Done set up
    Log.notice("FYI: Done setup" CR);    
}


void SetPixel(unsigned short pixelOffset, CRGB value)
{
    if (pixelOffset < NUM_LEDS) {
        leds[pixelOffset] = value;
    }
}
void SetPixelRangeWithTapper(unsigned short pixelOffsetStart, unsigned short length, unsigned char tapper, CRGB value)
{
    CRGB onValue = value;
    for (unsigned int sizeOffset = length / 2; sizeOffset > 0; sizeOffset--) {
        onValue.fadeToBlackBy(tapper);
        SetPixel(pixelOffsetStart + sizeOffset, onValue);
        SetPixel(pixelOffsetStart - sizeOffset + length, onValue);
    }
    SetPixel(pixelOffsetStart + length / 2, value);
}

void Fade() {
        // Fade everything by a bit 
    EVERY_N_MILLISECONDS( 100 ) { fadeToBlackBy(leds, NUM_LEDS, SETTING_CLOUD_FADE_SPEED); }
}

void TestColors() {
    fill_solid(leds+0, 10, CRGB::CRGB(0x30, 0xa7, 0xff));
    fill_solid(leds+10, 10, CRGB::CRGB(0x33, 0xff, 0x33));
    fill_solid(leds+20, 10, CRGB::CRGB(0xFF, 0x48, 0x00));
}

void loop() {

   for( unsigned short blobOffset = 0 ; blobOffset < SETTING_BLOB_COUNT ; blobOffset++) {
       // From Ben: Slowly cycling through white, blue, orange and green. Illuminating part of strip at any given time.
       SetPixelRangeWithTapper( blobs[blobOffset].startingIndex , 10, 255/3, ColorFromPalette(AsicsPalette, blobs[blobOffset].colorindex)) ; 
   }  

   EVERY_N_MILLISECONDS( SETTING_BLOB_FADE_SPEED ) { 
        for( unsigned short blobOffset = 0 ; blobOffset < SETTING_BLOB_COUNT ; blobOffset++) {
            blobs[blobOffset].Step();
        }
    }

    // send the 'leds' array out to the actual LED strip
    FastLED.show();
    // Fade(); // Ensure that everything fades to black eventually.

    // insert a delay to keep the framerate modest
    FastLED.delay(1000 / SETTING_LED_FRAMES_PER_SECOND);
    return;    
}




