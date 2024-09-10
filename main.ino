#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>

// Define the pin where the NeoPixel data line is connected
#define PIN 6

// Define the number of pixels in the 8x8 matrix
#define NUMPIXELS 64
#define ROWS 8
#define COLS 8

// WiFi credentials
const char* ssid = "<your network>";
const char* password = "<your password>";

// NTP server and time zone
const char* ntpServer = "ptbtime1.ptb.de";
const long utcOffsetInSeconds = 3600; // Adjust for your time zone

// Create an instance of the Adafruit_NeoPixel class
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Define the words and their corresponding positions in the matrix
struct Word {
  const char* text;
  int positions[8][2]; // Maximum 8 positions for simplicity
};
/*
ESISTCA. 
FÜNFZEHN
VOR.NACH
HALBFÜNF
ZDWREINS
ACHTVIER
SIECHSUB
ZWÖEHLNF
*/

Word words[] = {
  {"ES", {{0, 0}, {0, 1}, {-1, -1}}},
  {"IST", {{0, 2}, {0, 3}, {0, 4}, {-1, -1}}},
  {"CA.", {{0, 5}, {0, 6}, {0, 7}, {-1, -1}}},
  {"fuenf", {{1, 0}, {1, 1}, {1, 2}, {1, 3}, {-1, -1}}},
  {"zehn", {{1, 4}, {1, 5}, {1, 6}, {1, 7}, {-1, -1}}},
  {"vor", {{2, 0}, {2, 1}, {2, 2}, {-1, -1}}},
  {"nach", {{2, 4}, {2, 5}, {2, 6}, {2, 7}, {-1, -1}}},
  {"halb", {{3, 0}, {3, 1}, {3, 2}, {3, 3}, {-1, -1}}},
  {"EINS", {{4, 4}, {4, 5}, {4, 6}, {4, 7}, {-1, -1}}},
  {"ZWEI", {{4, 0}, {4, 2}, {4, 4}, {4, 5}, {-1, -1}}},
  {"DREI", {{4, 1}, {4, 3}, {4, 4}, {4, 5}, {-1, -1}}},
  {"VIER", {{5, 4}, {5, 5}, {5, 6}, {5, 7}, {-1, -1}}},
  {"FUENF", {{3, 4}, {3, 5}, {3, 6}, {3, 7}, {-1, -1}}},
  {"SECHS", {{6, 0}, {6, 2}, {6, 3}, {6, 4}, {6, 5}, {-1, -1}}},
  {"SIEBEN", {{6, 0}, {6, 1}, {6, 2}, {6, 7}, {7, 3}, {7, 6}, {-1, -1}}},
  {"ACHT", {{5, 0}, {5, 1}, {5, 2}, {5, 3}, {-1, -1}}},
  {"NEUN", {{4, 6}, {5, 6}, {6, 6}, {7, 6}, {-1, -1}}},
  {"ZEHN", {{7, 0}, {7, 3}, {7, 4}, {7, 6}, {-1, -1}}},
  {"ELF", {{7, 3}, {7, 5}, {7, 7}, {-1, -1}}},
  {"ZWOELF", {{7, 0}, {7, 1}, {7, 2}, {7, 5}, {7, 7}, {-1, -1}}},
  {".", {{2, 3}, {-1, -1}}},
};

// WiFiUDP instance
WiFiUDP ntpUDP;

// NTPClient instance
NTPClient timeClient(ntpUDP, ntpServer);
// Function to initialize time with NTP and set timezone
void initTime() {
  configTime(0, 0, ntpServer);
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1); // Timezone for Berlin
  tzset();
}

void setup() {
  // Initialize the NeoPixel strip
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  Serial.begin(115200);

  // Initialize NTP client
  timeClient.begin();
  initTime();
}

#ifdef TEST
unsigned int testhour = 11;
unsigned int testminute = 0;
void loop() {
  unsigned int hours = timeinfo.tm_hour;
  unsigned int minutes = timeinfo.tm_min;

  testminute++;
  if (testminute >= 60){
    testminute = 0;
    testhour = (testhour +1 )%12;
  }
  // Display the current time in words
  displayTimeInWords(testhour, testminute);
  strip.show();
  delay(250);
}

#else
#define UPDATETIME 60
int timeclient = 1;
void loop() {
  // Update the NTP client
  unsigned long start_millis = millis();
  if (--timeclient <= 0){
    Serial.println("updating time");
    timeClient.update();
    Serial.println("done");
    timeclient=UPDATETIME;
  }

  // Get the current time
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  unsigned int hours = timeinfo.tm_hour;
  unsigned int minutes = timeinfo.tm_min;

  // Display the current time in words
  displayTimeInWords(hours, minutes);
  strip.show();

  Serial.print(hours);
  Serial.print(":");
  Serial.println(minutes);
  delay(1000-(millis()-start_millis)); 
}
#endif

uint32_t colors[] = {
  strip.Color(30, 0, 0),    // Red
  strip.Color(0, 30, 0),    // Green
  strip.Color(0, 0, 30),    // Blue
  strip.Color(30, 30, 0),  // Yellow
  strip.Color(0, 30, 30),  // Cyan
  strip.Color(30, 0, 30),  // Magenta
  strip.Color(15, 0, 15),  // Purple
  strip.Color(0, 15, 15),  // Teal
  strip.Color(15, 15, 0),  // Olive
  strip.Color(30, 23, 24),// Pink
  strip.Color(0, 15,30),// Gray
  strip.Color(0, 0, 15),    // Navy
  strip.Color(30, 20, 0),  // Orange
  strip.Color(15, 0, 0),    // Maroon
  strip.Color(0, 15, 0),    // Dark Green
  strip.Color(30, 30, 30) // White
};
int colorindex = 0;
// Function to display a word on the matrix
void displayWord(const char* word, bool inc_color = true) {
  for (unsigned int i = 0; i < sizeof(words) / sizeof(words[0]); i++) {
    if (strcmp(words[i].text, word) == 0) {
      for (int j = 0; j < 8; j++) {
        int col = 7-words[i].positions[j][0];
        int row = words[i].positions[j][1];
        if ((row <0 || row >7) || (col <0 || col > 7)) break; // End of positions
        int pixelIndex = row * COLS + col;
        strip.setPixelColor(pixelIndex, colors[colorindex]); // White color

      }
      colorindex = ((colorindex +1) % 16);
      break;
    }
  }
}

// Function to clear the matrix
void clearMatrix() {
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, 0); // Turn off all pixels
  }
}

bool dot = false;

// Function to display the time in words
void displayTimeInWords(unsigned int hours, unsigned int minutes) {
  colorindex = minutes%16;
  clearMatrix();
  displayWord("ES");
  displayWord("IST");
  dot = !dot;
  if (dot){
    displayWord(".");
  }else{
    colorindex= (colorindex + 1)%16;
  }

  if (minutes > 0 && minutes < 5) {
    displayWord("CA.");
  } else if (minutes >= 5 && minutes < 10) {
    displayWord("fuenf");
    displayWord("nach");
  } else if (minutes >= 10 && minutes < 15) {
    displayWord("zehn");
    displayWord("nach");
  } else if (minutes >= 15 && minutes < 20) {
    displayWord("fuenf");
    displayWord("zehn");
    displayWord("nach");
  } else if (minutes >= 20 && minutes < 25) {
    displayWord("zehn");
    displayWord("vor");
    displayWord("halb");
  } else if (minutes >= 25 && minutes < 30) {
    displayWord("fuenf");
    displayWord("vor");
    displayWord("halb");
  } else if (minutes >= 30 && minutes < 35) {
    displayWord("halb");
  } else if (minutes >= 35 && minutes < 40) {
    displayWord("fuenf");
    displayWord("nach");
    displayWord("halb");
  } else if (minutes >= 40 && minutes < 45) {
    displayWord("zehn");
    displayWord("nach");
    displayWord("halb");
  } else if (minutes >= 45 && minutes < 50) {
    displayWord("fuenf");
    displayWord("zehn");
    displayWord("vor");
  } else if (minutes >= 50 && minutes < 55) {
    displayWord("zehn");
    displayWord("vor");
  } else if (minutes >= 55 && minutes < 60) {
    displayWord("fuenf");
    displayWord("vor");
  }

  if (minutes >= 20) {
    hours = (hours + 1);
  }
  hours = hours % 12;
  
  switch (hours) {
    case 1:
      displayWord("EINS");
      break;
    case 2:
      displayWord("ZWEI");
      break;
    case 3:
      displayWord("DREI");
      break;
    case 4:
      displayWord("VIER");
      break;
    case 5:
      displayWord("FUENF");
      break;
    case 6:
      displayWord("SECHS");
      break;
    case 7:
      displayWord("SIEBEN");
      break;
    case 8:
      displayWord("ACHT");
      break;
    case 9:
      displayWord("NEUN");
      break;
    case 10:
      displayWord("ZEHN");
      break;
    case 11:
      displayWord("ELF");
      break;
    case 12:
    case 0:
      displayWord("ZWOELF");
      break;
  }
}
