#define BLYNK_TEMPLATE_ID "TMPL6sRGW4-kr"
#define BLYNK_TEMPLATE_NAME "cam light"
#define BLYNK_AUTH_TOKEN "txi44z5uJqMIsInthDJapMxFyCBTIdOt" 
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Sanjula";
char pass[] = "password";

// MOSFET Gate Pins (Wired to the ESP32 R, G, B pins)
const int redPin = 4;
const int greenPin = 5;
const int bluePin = 18;

// --- TIMERS & STATES ---
unsigned long previousStateMillis = 0;
const long stateInterval = 60000; // CHANGED: 60,000 milliseconds (1 minute)

unsigned long previousFadeMillis = 0;
const long fadeInterval = 20; // 20 milliseconds per transition step

// Prevents the fade timer from fighting your manual app choices
bool manualOverride = false; 

// --- COLOR DATA ---
int targetIndex = 0;

// The 10 Target Colors for the auto-fade sequence
int targetColors[10][3] = {
  {255, 0, 0},     // 1. Red
  {255, 128, 0},   // 2. Orange
  {255, 255, 0},   // 3. Yellow
  {0, 255, 0},     // 4. Green
  {0, 255, 255},   // 5. Cyan
  {0, 0, 255},     // 6. Blue
  {128, 0, 255},   // 7. Purple
  {255, 0, 255},   // 8. Magenta
  {255, 20, 147},  // 9. Deep Pink
  {255, 255, 255}  // 10. White
};

// Current active RGB values (Starting exactly on Red)
int currentR = 255;
int currentG = 0;
int currentB = 0;

void setup() {
  Serial.begin(115200);
  
  // Configure the pins to push power out to the LEDs/MOSFETs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  
  // Set the starting color on boot
  analogWrite(redPin, currentR);
  analogWrite(greenPin, currentG);
  analogWrite(bluePin, currentB);
  
  Serial.println("Connecting to Blynk...");
  Blynk.begin(auth, ssid, pass);
  Serial.println("Analog Smooth Auto-Fade Started.");
}

// --- BLYNK MANUAL OVERRIDE ---
// Listen for Color Picker changes on Virtual Pin 1
BLYNK_WRITE(V1) {
  // Grab the 0-255 values from the Blynk app
  currentR = param[0].asInt(); 
  currentG = param[1].asInt(); 
  currentB = param[2].asInt(); 
  
  // Instantly push the new manual color to the pins
  analogWrite(redPin, currentR);
  analogWrite(greenPin, currentG);
  analogWrite(bluePin, currentB);
  
  // Lock in the custom color and pause the auto-fader!
  manualOverride = true; 
  previousStateMillis = millis(); // Reset the 1-minute clock
  
  Serial.print("Manual Override -> R: "); 
  Serial.print(currentR);
  Serial.print(" | G: "); 
  Serial.print(currentG);
  Serial.print(" | B: "); 
  Serial.println(currentB);
}

void loop() {
  // Keep the cloud connection alive 
  Blynk.run(); 

  unsigned long currentMillis = millis();

  // 1. The 1-Minute Timer: Pick the next target color
  if (currentMillis - previousStateMillis >= stateInterval) {
    previousStateMillis = currentMillis; 
    
    // 1 minute is up! Turn off the manual override and resume fading
    manualOverride = false; 
    
    // Move to the next color in the array
    targetIndex++;
    if (targetIndex > 9) { 
      targetIndex = 0; // Loop back to the beginning
    }
    Serial.print("Fading to new color index: ");
    Serial.println(targetIndex);
  }

  // 2. The Fade Timer: Smoothly step the current color toward the target color
  if (currentMillis - previousFadeMillis >= fadeInterval) {
    previousFadeMillis = currentMillis;

    // Only run the math steps if the user HAS NOT taken manual control
    if (manualOverride == false) {
      // Grab the RGB breakdown of our current target
      int tR = targetColors[targetIndex][0];
      int tG = targetColors[targetIndex][1];
      int tB = targetColors[targetIndex][2];

      // Increment or decrement current Red, Green, and Blue by 1 tiny step
      if (currentR < tR) currentR++;
      else if (currentR > tR) currentR--;

      if (currentG < tG) currentG++;
      else if (currentG > tG) currentG--;

      if (currentB < tB) currentB++;
      else if (currentB > tB) currentB--;

      // Push the slightly adjusted analog voltage to the pins
      analogWrite(redPin, currentR);
      analogWrite(greenPin, currentG);
      analogWrite(bluePin, currentB);
    }
  }
}