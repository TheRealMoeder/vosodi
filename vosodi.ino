// #include <SoftwareSerial.h>
#include <DoubleResetDetect.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// maximum number of seconds between resets that
// counts as a double reset
#define DRD_TIMEOUT 2.0

// address to the block in the RTC user memory
// change it if it collides with another usage 
// of the address block
#define DRD_ADDRESS 0x00

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// SoftwareSerial mySerial; // RX, TX

DoubleResetDetect drd(DRD_TIMEOUT, DRD_ADDRESS);

static const byte testpacket[] = { 0xAA, 0x1A, 0x46, 0x05, 0xE6, 0x0D, 0x25, 0x00, 0x00, 0x00, 0x00, 0x30, 0x2F, 0x02, 0x19, 0x93 };
byte packet[16], debug;
float pv, battery, current, power;

void setup() {
//  mySerial.begin(1000, SWSERIAL_8N1, 14, 12, false, 95, 11);
  if (drd.detect()) debug = 1;
    else debug = 0;
  
  Serial.begin(9600);
  Serial.println("Debugging Votronic Displaydatendecoder");
  Serial.println("--------------------------------------");

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(200); // Pause for 0,2 seconds

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
}

void loop() {
//  mySerial.write(0x93);
  if (getvotronic()) parsevotronic();
  display.clearDisplay();
  kopfzeile();
  datenanzeige();
  if (debug) printdebugpacket();
  display.display();
  delay(2000);
}

byte getvotronic() {
  // testdaten, hier noch softserial read ergänzen
  uint8_t checksum = 0;
  
  packet[15] = testpacket[15];
  for (uint16_t i = 1; i < sizeof(testpacket)-1; i++) {
    packet[i] = testpacket[i];
    checksum ^= packet[i];
  }
  
  if(checksum == packet[15]) return 1;
    else return 0;
}


void parsevotronic() {
  battery = ((packet[3] << 8) | packet[2]);
//  battery = random(1450);
  pv = ((packet[5] << 8) | packet[4]);
//  pv = random(3800);
  current = ((packet[7] << 8) | packet[6]);
//  current = random(20000);
  power = current * battery;
}

void kopfzeile() {
  display.setFont(&FreeSansBold9pt7b);
  display.setTextSize(1);
  display.setCursor(0,13);
  display.println("Wohnmobil PV");
}

void datenanzeige() {
  display.setFont();
  display.setTextSize(1);
  display.setCursor(0,19);
  display.print(" PV-Modul:  ");
  display.print(pv/100);
  display.println(" V");
  display.print(" Batterie:  ");
  display.print(battery/100);
  display.println(" V");
  display.print(" Ladestrom: ");
  display.print(current/1000);
  display.println(" A");
  display.print(" Leistung:  ");
  display.print(power/100000);
  display.println(" W");
}

void printdebugpacket(void) {
  display.setFont();
  display.setTextSize(1);
  display.cp437(true);
  display.setCursor(0,55);
  for (int16_t i=1; i < sizeof(packet)-1; i++) {
      display.print(packet[i],HEX);
      Serial.print(packet[i],HEX);
    }
    Serial.println(" ");
}
