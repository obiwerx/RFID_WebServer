#RFID WebServer Project - T. O'Brien (StudioOB)

This is a code mashup using the Arduino Ethernet libraries plus the 
ADAFRUIT PN532 library. Basically it reads basic MIFARE type RFID
tags and posts the UID and page 4 data to a web server.  I've
commented the code pretty well, but there is a few things you'll
need to do to get it to work:

*Copy the enclosed libraries to your Arduino Libraries folder.

*Change the MAC address to the MAC address of your Arduino Ethernet.

*Change the IP address to something reachable on your network.

##Happy hacking!