#include "HX711.h"
#include <SPI.h>
#include <SD.h>
/*
 * This is a datalogger for the PSAS LV3.1 Recovery system team. It is designed
 * to read the input from a load cell connected to an HX711 breakout board and 
 * write to a .txt file saved to an SD card through the adafruit SD shield.
 * 
 * Output should be in the form of a .txt file with two columns:
 *    column 1: time in milliseconds of the reading
 *    column 2: load cell reading at time indicated
 */



File myFile;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 3;

HX711 scale;

void setup() {
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

}

void loop() {
//Use this line to name your text document. Make sure to keep the .txt
//file extention and keep it all in double quotes.
myFile = SD.open("Shamalam.txt", FILE_WRITE);

// if the file opened okay, Log Data:
  if (myFile) {
//But only if the load cell is working right.
    if (scale.is_ready()) {
      long reading = scale.read();
      long treading = millis();
//Print to the Serial Monitor (For insurance purposes)
      Serial.print(treading);
      Serial.print('\t');
      Serial.println(reading);
//Print to file (because that's what we want)
      myFile.print(treading);
      myFile.print('\t');
      myFile.println(reading);
      myFile.close();
    } else {
//Otherwise, start spittin' errors
      Serial.println("HX711 not found.");
    }
  } else {
    Serial.println("error opening test.txt");
  }

  delay(1000);
  
}
