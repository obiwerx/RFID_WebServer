
/* KIE RFID Web Server Test
   08/15/13 - T. O'Brien
   Designed for Arduino Ethernet
   and RFID board PN532 */
   
//LIBRARIES USED
#include <SPI.h>                    //This library allows communication with SPI devices
#include <Ethernet.h>               //Standard ethernet library
#include <Wire.h>                   //library allows communication with I2C / TWI devices.
#include <Adafruit_NFCShield_I2C.h> //NFC/RF Shield library

//MACROS
#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

Adafruit_NFCShield_I2C nfc(IRQ, RESET);


// Configure TCP/IP and Port Information
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xB9, 0xA2 };  //MAC address of Arduino Ethernet
IPAddress ip(10,9,8,83);   // dedicated IP address - note the use of commas (EU Style)
EthernetServer server(80); // Initialize the Ethernet server library


//SETUP function
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect.
  }
  
  nfc.begin();      // initialize RFID

  nfc.SAMConfig();  // configure board to read RFID tags

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

//LOOP function
void loop() {

  //ISO14443A objects
  uint8_t success;
  uint8_t uid[] = { 
    0, 0, 0, 0, 0, 0, 0       };    // Buffer to store the returned UID
  uint8_t uidLength;                // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {

    Serial.println("new client");
    
    boolean currentLineIsBlank = true;  // an http request ends with a blank line

    while (client.connected()) {

      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply

        if (c == '\n' && currentLineIsBlank) {
         
          client.println("HTTP/1.1 200 OK");            // send a standard http response header
          client.println("Content-Type: text/html");
          client.println("Connection: close");          // the connection will be closed after completion of the response
          client.println("Refresh: 5");                 // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          client.println("HTML WEBSERVER TEST - KIE RFID PROJECT");
          client.println("<br />"); 

          client.println("Code version: 1.0.8 alpha");
          client.println("<br />"); 
          client.println("<br />");

          // Wait for an ISO14443A type card.  When one is found
          // 'uid' will be populated with the UID, and uidLength will indicate
          // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
          
          success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

          if (success) {
            // Display some basic information about the card
            client.println("Found an ISO14443A card");
            client.println("<br />");
            client.println("  UID Length: ");
            client.print(uidLength, DEC);
            client.println(" bytes");
            client.println("<br />");
            client.println("  UID Value: ");
             for (uint8_t i=0; i < uidLength; i++) 
             {
             client.print(" 0x");client.print(uid[i], HEX); 
             }
            client.println("");
            client.println("<br />"); 
            client.println("<br />");
          }

          if (uidLength == 4)
          {

            // We probably have a Mifare Classic card ... 
            client.println("Seems to be a Mifare Classic card (4 byte UID)");
            client.println("<br />");

            // Now we need to try to authenticate it for read/write access
            // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
            client.println("Trying to authenticate block 4 with default KEYA value");
            uint8_t keya[6] = {
              0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF                        };
            client.println("<br />");

            // Start with block 4 (the first block of sector 1) since sector 0
            // contains the manufacturer data and it's probably better just
            // to leave it alone unless you know what you're doing
            
            success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);

            if (success)
            {
              client.println("Sector 1 (Blocks 4..7) has been authenticated");
              uint8_t data[16];
              client.println("<br />");
              client.println("<br />");

              // Try to read the contents of block 4
              success = nfc.mifareclassic_ReadDataBlock(4, data);

              if (success)
              {
                // Data seems to have been read ... spit it out
                client.println("Reading Block 4:");
                client.println("<br />");
                for (uint8_t i=0; i < 16; i++) 
                 {
                   client.print(" 0x");client.print(data[i], HEX);
                   client.println("");
                   client.println("<br />");  
                 }
                // Wait a bit before reading the card again
                delay(1000);
              }
              else
              {
                client.println("Ooops ... unable to read the requested block.  Try another key?");
                client.println("<br />");
              }
            }
            else
            {
              client.println("Ooops ... authentication failed: Try another key?");
              client.println("<br />");
            }
          }

          if (uidLength == 7)
          {
            // We probably have a Mifare Ultralight card ...
            client.println("Seems to be a Mifare Ultralight tag (7 byte UID)");
            client.println("<br />");

            // Try to read the first general-purpose user page (#4)
            client.println("Reading page 4");
            client.println("<br />");
            uint8_t data[32];
            success = nfc.mifareultralight_ReadPage (4, data);
            if (success)
            {
              // Data seems to have been read ... spit it out
              nfc.PrintHexChar(data, 4);
              client.println("");
              client.println("<br />");

              // Wait a bit before reading the card again
              delay(1000);
            }
            else
            {
              client.println("Ooops ... unable to read the requested page!?");
              client.println("<br />");
            }
          }

          client.println("</html>");
          break;
        }
        if (c == '\n') {
          
          currentLineIsBlank = true;   // you're starting a new line
        } 
        else if (c != '\r') {
          
          currentLineIsBlank = false;  // you've gotten a character on the current line
        }
      }

    }
    
    delay(1);        // give the web browser time to receive the data
    
    client.stop();   // close the connection:
    
    Serial.println("client disonnected");
  }
}



