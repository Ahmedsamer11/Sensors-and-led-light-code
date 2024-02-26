#include <Arduino.h>
#include <Wire.h>  // Include the I2C library (required)
#include <SparkFunSX1509.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

byte SX1509_ADDRESS[2] = { 0x3E, 0x3F };  // SX1509 I2C address
const byte SX1509_ADDRESS_2 = 0x3E;       // SX1509 I2C address
SX1509 io;                                // Create an SX1509 object to be used throughout
SX1509 io_2;
int wr = 0;

#define I2C_SDA 14
#define I2C_SCL 15
// SX1509 pin definitions:
// Note: these aren't Arduino pins. They're the SX1509 I/O:
const byte SX1509_LED_PIN[20] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 10, 11, 12, 13 };  // LED connected to 15 (source ing current)

const byte SX1509_SENSOR_PIN[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };  // Button connected to 0 (Active-low button)

const byte ARDUINO_INTERRUPT_PIN = 13;

bool ledState[10] = { 0 };
String documentPath;


#define WIFI_SSID "WiFi"
#define WIFI_PASSWORD "18/38*6450+HoMe"

/* 2. Define the API Key */
#define API_KEY "AIzaSyBjVSWfK-KRoao5ddtVI4pLzas9xyMEx4A"

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "smart-parking-5eff5"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "saifkamelae@gmail.com"
#define USER_PASSWORD "Saif123123"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  // Serial is used in this example to display the input value
  // of the SX1509_INPUT_PIN input:
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi");
  unsigned long ms = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
   delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  // fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  // // Limit the size of response payload to be collected in FirebaseData
  fbdo.setResponseSize(2048);

 Firebase.begin(&config, &auth);

  Wire.setPins(I2C_SDA, I2C_SCL);
Wire.begin();
  // Call io.begin(<address>) to initialize the SX1509. If it
  // successfully communicates, it'll return 1.
  if (io.begin(SX1509_ADDRESS[0]) == false) {
    Serial.println("Failed to communicate. Check wiring.");
    // digitalWrite(18, HIGH); // If we failed to communicate, turn the pin 13 LED on
    while (1)
      ;  // If we fail to communicate, loop forever.
  }

  if (io_2.begin(SX1509_ADDRESS[1]) == false) {
    Serial.println("Failed to communicate. Check wiring and address of SX1509.");
    // digitalWrite(18, HIGH); // If we failed to communicate, turn the pin 13 LED on
    while (1)
      ;  // If we fail to communicate, loop forever.
  }

  // Call io.pinMode(<pin>, <mode>) to set any SX1509 pin as
  // either an INPUT, OUTPUT, INPUT_PULLUP, or ANALOG_OUTPUT
  // Set output for LED:
  for (uint8_t outs = 0; outs < 16; outs++) {
    io_2.pinMode(SX1509_LED_PIN[outs], OUTPUT);
    delay(100);
  }
  for (uint8_t outs = 0; outs < 15; outs += 2) {
    io_2.digitalWrite(SX1509_LED_PIN[outs], LOW);
    Serial.println(outs);
    delay(100);
  }
  for (uint8_t outs = 1; outs < 16; outs += 2) {
    io_2.digitalWrite(outs, HIGH);
    Serial.println(outs);
    delay(100);
  }

    for (int outs =10; outs<14;outs++){
io.pinMode(SX1509_LED_PIN[outs], OUTPUT);
Serial.println(outs);
delay(100);
  }
  
    for (int outs =10; outs<13;outs++){  
io.digitalWrite(SX1509_LED_PIN[outs], HIGH);
delay(100);
  }
  for (int outs =11; outs<14;outs+=2){   
io.digitalWrite(SX1509_LED_PIN[outs], LOW);
delay(100);
  }

  for (uint8_t ins = 0; ins < 10; ins++) {
    io.pinMode(SX1509_SENSOR_PIN[ins], INPUT);
    Serial.println(ins);
    delay(100);
    io.enableInterrupt(SX1509_SENSOR_PIN[ins], CHANGE);
    delay(100);
  }


  pinMode(ARDUINO_INTERRUPT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ARDUINO_INTERRUPT_PIN), Int, FALLING);
}

void loop() {
  // Use io.digitalRead() to check if an SX1509 input I/O is
  // either LOW or HIGH.
  delay(1);

  if (wr == 1) {
    wr = 0;
    uint16_t int_out = io.interruptSource();
    //Serial.println(int_out, BIN);

    if (int_out & 0x1) {
      ledState[0] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[0], ledState[0]);
      io_2.digitalWrite(SX1509_LED_PIN[1], !ledState[0]);
      //send ledState[0];
       documentPath = "slots/A1";
      data_send(documentPath, ledState[0]);
      Serial.println(ledState[0]);
    }


    if (int_out & 0x2) {
      ledState[1] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[2], ledState[1]);
      io_2.digitalWrite(SX1509_LED_PIN[3], !ledState[1]);
       documentPath = "slots/A2";
      data_send(documentPath, ledState[1]);
      Serial.println(ledState[1]);
    }

    if (int_out & 0x4) {
      ledState[2] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[4], ledState[2]);
      io_2.digitalWrite(SX1509_LED_PIN[5], !ledState[2]);
       documentPath = "slots/A3";
      data_send(documentPath, ledState[2]);
      Serial.println(ledState[2]);
    }

    if (int_out & 0x8) {
      ledState[3] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[6], ledState[3]);
      io_2.digitalWrite(SX1509_LED_PIN[7], !ledState[3]);
       documentPath = "slots/A4";
      data_send(documentPath, ledState[3]);
      Serial.println(ledState[3]);
    }

    if (int_out & 0x10) {
      ledState[4] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[8], ledState[4]);
      io_2.digitalWrite(SX1509_LED_PIN[9], !ledState[4]);
       documentPath = "slots/A5";
      data_send(documentPath, ledState[4]);
      Serial.println(ledState[4]);
    }

        if (int_out & 0x20) {
      ledState[5] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[10], ledState[5]);
      io_2.digitalWrite(SX1509_LED_PIN[11], !ledState[5]);
       documentPath = "slots/B1";
      data_send(documentPath, ledState[5]);
      Serial.println(ledState[5]);
    }

        if (int_out & 0x40) {
      ledState[6] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[12], ledState[6]);
      io_2.digitalWrite(SX1509_LED_PIN[13], !ledState[6]);
       documentPath = "slots/B2";
      data_send(documentPath, ledState[6]);
      Serial.println(ledState[6]);
    }

        if (int_out & 0x80) {
      ledState[7] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[14], ledState[7]);
      io_2.digitalWrite(SX1509_LED_PIN[15], !ledState[7]);
       documentPath = "slots/B3";
      data_send(documentPath, ledState[7]);
      Serial.println(ledState[7]);
    }

        if (int_out & 0x100) {
      ledState[8] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[16], ledState[8]);
      io_2.digitalWrite(SX1509_LED_PIN[17], !ledState[8]);
       documentPath = "slots/B4";
      data_send(documentPath, ledState[8]);
      Serial.println(ledState[8]);
    }

        if (int_out & 0x200) {
      ledState[9] ^= 1;
      io_2.digitalWrite(SX1509_LED_PIN[18], ledState[9]);
      io_2.digitalWrite(SX1509_LED_PIN[19], !ledState[9]);
       documentPath = "slots/B5";
      data_send(documentPath, ledState[9]);
      Serial.println(ledState[9]);
    }
  }
}

void Int() {
  wr = 1;
}

void data_send(String documentPath, bool state) {
  String data;
  if (state) {
    data = "busy";
  } else {
    data = "available";
  }
  FirebaseJson content;
  content.clear();
  content.set("fields/status/stringValue", data.c_str());
  if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw(), "status" /* updateMask */))
    Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
  else
    Serial.println(fbdo.errorReason());
}
