/*
  SD card read/write

 This example shows how to read and write data to and from an SD card file
 The circuit:
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>

File myFile;

int dataset = 0; // this way when the user triggers data collection in the serial monitor is begins writing at 1
boolean pause = false;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
 
}

void loop() {

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("testing.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to testing.txt...");//leting you know its started
    myFile.println("testing 1, 2, 3.");//logging to the SD card
    // close the file:
    myFile.close();
    Serial.println("done.");//letting you know that it's logged b/c you can't see it logging
    myFile = SD.open("testing.txt");
    Serial.println("testing.txt:");//printing what is on the SD card
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
    // pause the monitor
    pause = true;
    while(pause == true){ 
        if (Serial.read() == 'n'){ //"n" for next test
          pause = false;
          Serial.write("unpaused.");
          //else pause = true;
        }
    }
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

  
  /* test = SD.open("Newest.txt", FILE_WRITE);
  if(test){
      dataset += 1;
      Serial.print("Data logging started...");
      test.print("i don't understand what's going on...");
      test.print(dataset);

      // read from the file until there's nothing else in it:
      Serial.println("Newest.txt:");
      while (test.available()) { //printing what is on the SD card
        Serial.write(test.read());
      }
      // close the file:
      test.close();
      
      // pause the serial read and data log until you're ready for the next test
      pause = true;
      while(pause = true){ 
        if (Serial.read() == 'n')
          Serial.write("done.");
          pause = false;
          //else pause = true;
      }
  }
//  Serial.write(test.read());
}

 if (test) {
    Serial.print("Writing to Mtest.txt...");
    for (int i = 0; i <= 10; i++) {
    test.println(i);
    }
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
  
  Serial.println("done.");
  // close the file:
  test.close();

  // re-open the file for reading:
  test = SD.open("Mtest.txt");
  if (test) {
    Serial.println("Mtest.txt:");

    // read from the file until there's nothing else in it:
    while (test.available()) {
      Serial.write(test.read());
    }
    // close the file:
    test.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening Mtest.txt");
  }
}
*/

