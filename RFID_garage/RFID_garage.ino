#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

#define RELAY_PIN 7

void setup(){
    Serial.begin(9600); // Initialize serial communications with the PC
    SPI.begin();            // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522 card
    Serial.println("Scan PICC to see UID and type...");
    pinMode(RELAY_PIN, OUTPUT);
}

void loop() {  
    // Look for new cards
    if(!mfrc522.PICC_IsNewCardPresent()){
        return;
    }

    // Select one of the cards
    if(!mfrc522.PICC_ReadCardSerial()){
        return;
    }

    // Dump debug info about the card. PICC_HaltA() is automatically called.
    //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

    if(mfrc522.uid.uidByte[0] == 0x00 && 
       mfrc522.uid.uidByte[1] == 0x00 &&
       mfrc522.uid.uidByte[2] == 0x00 &&
       mfrc522.uid.uidByte[3] == 0x00){
      Serial.println("RFID CARD = OK");
      openGarage();
      delay(1000);
    }
    else if(mfrc522.uid.uidByte[0] == 0x00 && 
            mfrc522.uid.uidByte[1] == 0x00 &&
            mfrc522.uid.uidByte[2] == 0x00 &&
            mfrc522.uid.uidByte[3] == 0x00){
      Serial.println("Arduino white card = OK");
      openGarage();
      delay(1000);
    }
    else if(mfrc522.uid.uidByte[0] == 0x00 && 
            mfrc522.uid.uidByte[1] == 0x00 &&
            mfrc522.uid.uidByte[2] == 0x00 &&
            mfrc522.uid.uidByte[3] == 0x00){
      Serial.println("Arduino blue keychain = OK");
      openGarage();
      delay(1000);
    }
    else{
      Serial.println("Unknown = NOT OK");
    }

    delay(100);
}

void openGarage(){
  //singal is sent to relay for 75ms
  digitalWrite(RELAY_PIN, HIGH);
  delay(75);
  digitalWrite(RELAY_PIN, LOW);
}
