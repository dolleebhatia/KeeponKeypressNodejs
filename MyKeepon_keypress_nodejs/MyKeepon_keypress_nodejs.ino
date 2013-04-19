

#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include <Servo.h> 
 

#define cbi(sfr, bit) _SFR_BYTE(sfr) &= ~_BV(bit)
#define sbi(sfr, bit) _SFR_BYTE(sfr) |= _BV(bit)

#define MK_FREQ 49600L // Set clock to 50kHz (actualy 49.6kHz seems to work better)
#define SOUND (byte)0x52  // Sound controller (device ID 82).  Write to 0xA4, read from 0xA5.
#define BUTTON (byte)0x50u // Button controller (device ID 80). Write to 0xA0, read from 0xA1.
#define MOTOR (byte)0x55  // Motor controller (device ID 85).  Write to 0xAA, read from 0xAB.


//////////////// WiFi + Server Stuff

char ssid[] = "itpsandbox"; //  your network SSID (name) 
char pass[] = "NYU+s0a!+P?";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;  // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
// if you don't want to use DNS (and reduce your sketch size)
// use the numeric IP instead of the name for the server:
IPAddress server(166,78,61,139);  //RACKSPACE

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
int port = 12002;
WiFiClient client;

///////////////// declare both shoulder servos
Servo shoulderLeft;
Servo shoulderRight;



int servoAngles[] = {0,0};
int shoulderLeftVal;
int shoulderRightVal;

void setup()
{
  
  
  
  
  
  //////////////////// Attach shoulder servos to their pins 
  shoulderLeft.attach(9);
  shoulderRight.attach(5);
  
  ///////////////////// Grounding keepon pins for a second to put in slave reciever mode
  
  pinMode(A4, OUTPUT); // Data wire on My Keepon
  pinMode(A5, OUTPUT); // Clock wire on My Keepon
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
    delay(1000);
  Serial.begin(115200);
  
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true);
  } 

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(3000);
  } 
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (client.connect(server, port)) {
    Serial.println("connected to server");
    client.println("A");
    Serial.println("reading..");
  //  inByte = client.read();
    //Serial.println(inByte);
    //client.flush();  
    
  }
}

void bootup()
{
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  Serial.print("Waiting for My Keepon... ");
  while (analogRead(0) < 512); // Wait until we see voltage on A0 pin
  Serial.println("My Keepon detected.");
  delay(1000);
  Wire.begin();
  TWBR = ((F_CPU / MK_FREQ) - 16) / 2;
  Serial.write((byte)0);
}

boolean isNextWord(char* msg, char* wordToCompare, int* i) {
  int j = 0;
  while (msg[j] == wordToCompare[j++]);
  if (j >= strlen(wordToCompare)) {
    *i = *i+j;
    return 1;
  } 
  else return 0;
}

int nextInt(char* msg) {
  int j = 0;
  int value = 0;
  int negative = 0;
  while (!(isDigit(msg[j]) || msg[j]=='-')) j++;
  if (msg[j] == '-') {
    negative = 1;
    j++;
  }
  while (isDigit(msg[j])) {
    value *= 10;
    value += msg[j++]-'0';
  }
  if (negative) value *= -1;
  return value;
}

boolean parseMsg(char* msg, byte* cmd, byte* device) {
  int i = 0, value;

  if (isNextWord(&msg[i], "SOUND", &i)) {
    *device = SOUND;
    if (isNextWord(&msg[i], "PLAY", &i)) {
      cmd[0] = 0x01;
      cmd[1] = B10000000 | (63 & nextInt(&msg[i]));
      shoulderLeftVal =  map( (63 & nextInt(&msg[i])), 0, 180, 0, 63);
      shoulderLeft.write(i); 
         shoulderRightVal=  map((63 & nextInt(&msg[i])), 0, 180, 0, 63);
       shoulderRight.write(shoulderRightVal);
    } 
    else if (isNextWord(&msg[i], "REPEAT", &i)) {
      cmd[0] = 0x01;
      cmd[1] = B11000000 | (63 & nextInt(&msg[i]));
      shoulderRightVal=  map((63 & nextInt(&msg[i])), 0, 180, 0, 63);
       shoulderRight.write(shoulderRightVal);
   shoulderLeftVal =  map((63 & nextInt(&msg[i])), 0, 180, 0, 63);
      shoulderLeft.write(shoulderLeftVal);  
    } 
    else if (isNextWord(&msg[i], "DELAY", &i)) {
      cmd[0] = 0x03;
      cmd[1] = (byte)nextInt(&msg[i]);
    } 
    else if (isNextWord(&msg[i], "STOP", &i)) {
      cmd[0] = 0x01;
      cmd[1] = B00000000;
    } 
    else {
      Serial.println("Unknown command.");
      return false;
    }
  } 
  else if (isNextWord(&msg[i], "SPEED", &i)) {
    *device = MOTOR;
    if (isNextWord(&msg[i], "PAN", &i)) {
      cmd[0] = 5;
      cmd[1] = (byte)nextInt(&msg[i]);
    } 
    else if (isNextWord(&msg[i], "TILT", &i)) {
      cmd[0] = 3;
      cmd[1] = (byte)nextInt(&msg[i]);
    } 
    else if (isNextWord(&msg[i], "PONSIDE", &i)) {
      cmd[0] = 1;
      cmd[1] = (byte)nextInt(&msg[i]);
    } 
    else {
      Serial.println("Unknown command.");
      return false;
    }
  } 
  else if (isNextWord(&msg[i], "MOVE", &i)) {
    *device = MOTOR;
    if (isNextWord(&msg[i], "PAN", &i)) {
      cmd[0] = 4;
      cmd[1] = (byte)(nextInt(&msg[i]) + 127);
    } 
    else if (isNextWord(&msg[i], "TILT", &i)) {
      cmd[0] = 2;
      cmd[1] = (byte)(nextInt(&msg[i]) + 127);
    } 
    else if (isNextWord(&msg[i], "SIDE", &i)) {
      cmd[0] = 0;
      if (isNextWord(&msg[i], "CYCLE", &i))
        cmd[1] = 0;
      else if (isNextWord(&msg[i], "CENTERFROMLEFT", &i))
        cmd[1] = 1;
      else if (isNextWord(&msg[i], "RIGHT", &i))
        cmd[1] = 2;
      else if (isNextWord(&msg[i], "CENTERFROMRIGHT", &i))
        cmd[1] = 3;
      else if (isNextWord(&msg[i], "LEFT", &i))
        cmd[1] = 4;
      else {
        Serial.println("Unknown command.");
        return false;
      }
    } 
    else if (isNextWord(&msg[i], "PON", &i)) {
      cmd[0] = 0;
      if (isNextWord(&msg[i], "UP", &i))
        cmd[1] = -1;
      else if (isNextWord(&msg[i], "HALFDOWN", &i))
        cmd[1] = -2;
      else if (isNextWord(&msg[i], "DOWN", &i))
        cmd[1] = -3;
      else if (isNextWord(&msg[i], "HALFUP", &i))
        cmd[1] = -4;
      else {
        Serial.println("Unknown command.");
        return false;
      }
    } 
    else if (isNextWord(&msg[i], "STOP", &i)) {
      cmd[0] = 6;
      cmd[1] = 16;
    } 
    else {
      Serial.println("Unknown command.");
      return false;
    }    
  } 
  else if (isNextWord(&msg[i], "MODE", &i)) {
    if (isNextWord(&msg[i], "DANCE", &i)) {
      cmd[0] = 6;
      cmd[1] = 0;
    } 
    else if (isNextWord(&msg[i], "TOUCH", &i)) {
      cmd[0] = 6;
      cmd[1] = 1;
    } 
    else if (isNextWord(&msg[i], "TEMPO", &i)) {
      cmd[0] = 6;
      cmd[1] = 2;
    } 
    else if (isNextWord(&msg[i], "SLEEP", &i)) {
      cmd[0] = 6;
      cmd[1] = 240;
    } 
    else {
      Serial.println("Unknown command.");
      return false;
    }
  } 
  else {
    Serial.println("Unknown command.");
    return false;
  }
  return true;
}

boolean buttonState[8];
char* buttonName[] = {
  "DANCE", "", "HEAD", "TOUCH",
  "RIGHT", "FRONT", "LEFT", "BACK"};

boolean motorState[8];
char* motorName[] = {
  "PON FINISHED", "SIDE FINISHED", "TILT FINISHED", "PAN FINISHED",
  "PON STALLED", "SIDE STALLED", "TILT STALLED", "PAN STALLED"};

int encoderState[4], audioState[5], emfState[3], positionState[3];
char* encoderName[] = {
  "TILT NOREACH", "TILT FORWARD", "TILT BACK", "TILT UP",
  "PON HALFDOWN", "PON UP", "PON DOWN", "PON HALFUP",
  "SIDE CENTER", "SIDE LEFT", "SIDE RIGHT", "SIDE CENTER",
  "PAN BACK", "PAN RIGHT", "PAN LEFT", "PAN CENTER"};

unsigned long updatedButton = 0, updatedMotor = 0;
void query() {
  int i;
  byte buttonResponse, motorResponse;
  int intResponse;

  if (millis() - updatedButton > 100) {
    updatedButton = millis();
    Wire.requestFrom((int)BUTTON, 1);
    if (Wire.available() >= 1) {
      buttonResponse = Wire.read();
      for (i = 0; i < 8; i++) {
        if (i != 1) {
          if (buttonResponse & (1<<i)) {
            if (!buttonState[i]) {
              Serial.print("BUTTON ");
              Serial.print(buttonName[i]);
              Serial.println(" ON");
              buttonState[i] = 1;
            }
          }
          else if (buttonState[i]) {
            Serial.print("BUTTON ");
            Serial.print(buttonName[i]);
            Serial.println(" OFF");
            buttonState[i] = 0;
          }
        }
      }
    }
  }

  if (millis() - updatedMotor > 300) {
    updatedMotor = millis();
    Wire.requestFrom((int)MOTOR, 13);
    if (Wire.available() >= 13) {
      motorResponse = Wire.read();
      for (i = 0; i < 8; i++) {
        if (motorResponse & (1<<i)) {
          if (!motorState[i]) {
            Serial.print("MOTOR ");
            Serial.println(motorName[i]);
            motorState[i] = 1;
          }
        } 
        else if (motorState[i]) {
          motorState[i] = 0;
        }
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[0]) {
     //   Serial.print("AUDIO TEMPO ");
      //  Serial.println(motorResponse);
        audioState[0] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[1]) {
      //  Serial.print("AUDIO MEAN ");
      //  Serial.println(motorResponse);
        audioState[1] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[2]) {
      //  Serial.print("AUDIO RANGE ");
      //  Serial.println(motorResponse);
        audioState[2] = motorResponse;
      }
      motorResponse = Wire.read();
      for (i = 0; i < 4; i++) {
        if ((motorResponse & (3<<(2*i))) != encoderState[i]) {
          encoderState[i] = motorResponse & (3<<(2*i));
          Serial.print("ENCODER ");
          Serial.println(encoderName[4*i+(encoderState[i]>>(2*i))]);
        }
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[0]) {
        Serial.print("EMF PONSIDE ");
        Serial.println(intResponse);
        emfState[0] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[1]) {
        Serial.print("EMF TILT ");
        Serial.println(intResponse);
        emfState[1] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse;
      if (intResponse > 0) intResponse -= 127;
      if (intResponse != emfState[2]) {
        Serial.print("EMF PAN ");
        Serial.println(intResponse);
        emfState[2] = intResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[3]) {
        //      Serial.print("AUDIO ENVELOPE ");
        //      Serial.println(motorResponse);
        audioState[3] = motorResponse;
      }
      motorResponse = Wire.read();
      if (motorResponse != audioState[4]) {
      //  Serial.print("AUDIO BPM ");
      //  Serial.println(motorResponse);
        audioState[4] = motorResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[0]) {
        Serial.print("POSITION PONSIDE ");
        Serial.println(intResponse);
        positionState[0] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[1]) {
        Serial.print("POSITION TILT ");
        Serial.println(intResponse);
        positionState[1] = intResponse;
      }
      motorResponse = Wire.read();
      intResponse = motorResponse - 127;
      if (intResponse != positionState[2]) {
        Serial.print("POSITION PAN ");
        Serial.println(intResponse);
        positionState[2] = intResponse;
      }
    }
  }
}

void loop() {
  char msg[32];
  byte device, cmd[2];
  bootup();

  while (analogRead(0) > 512) {
        //sayHI();
    query();
    
    
      // if there are incoming bytes available 
  // from the server, read them and print them:
// while (client.connected()) {
//    if (client.available()) {
//    char c = client.read();
//    Serial.print(c);
//  }
 //}
    
    
 while (client.connected()) {
    if (client.available()>0) {
 //   if (Serial.available() > 0) {
      int i = 0;
      while ((msg[i++] = client.read()) != ';' && i < 30 && analogRead(0) > 512) {
        while (client.available() <= 0 && analogRead(0) > 512);
      }
      msg[i] = '\0';
      if (parseMsg(msg, cmd, &device)) {
        int result = 1;
        int attempts = 0;
        while (result != 0 && attempts++ < 5000) {
          Wire.beginTransmission(device);
          Wire.write((byte)cmd[0]);
          Wire.write((byte)cmd[1]);
          Serial.println(cmd[0]);
          result = (int)Wire.endTransmission();
        }
        attempts =0;
      }
    }
  }
 }

  
  
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
    client.flush();
    // do nothing forevermore:
    while(true);
  }
  
  
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}



