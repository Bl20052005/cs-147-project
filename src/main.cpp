#include <SPI.h>
#include "SparkFunLSM6DSO.h"
#include <Arduino.h>
#include <HttpClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

#define RED_PIN 32
#define GREEN_PIN 33
#define YELLOW_BUTTON 25
#define RED_BUTTON 26
#define BLUE_BUTTON 27
#define RED_STATE 0
#define GREEN_STATE 1
#define RED_GREEN_STATE 2

volatile int onOff = 0;
volatile int steps = 0;

int led_state = RED_STATE;
int pressed = 0;

LSM6DSO myIMU;

uint8_t count = 0;

// This example downloads the URL "http://arduino.cc/"
char ssid[50]; // your network SSID (name)
char pass[50]; // your network password (use for WPA, or use
// as key for WEP)
// Name of the server we want to connect to
const char kHostname[] = "worldtimeapi.org";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
const char kPath[] = "/api/timezone/Europe/London.txt";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;
void nvs_access()
{
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  // Open
  Serial.printf("\n");
  Serial.printf("Opening Non-Volatile Storage (NVS) handle... ");
  nvs_handle_t my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    Serial.printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else
  {
    Serial.printf("Done\n");
    Serial.printf("Retrieving SSID/PASSWD\n");
    size_t ssid_len;
    size_t pass_len;
    err = nvs_get_str(my_handle, "ssid", ssid, &ssid_len);
    err |= nvs_get_str(my_handle, "pass", pass, &pass_len);
    switch (err)
    {
    case ESP_OK:
      Serial.printf("Done\n");
      // Serial.printf("SSID = %s\n", ssid);
      // Serial.printf("PASSWD = %s\n", pass);
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      Serial.printf("The value is not initialized yet!\n");
      break;
    default:
      Serial.printf("Error (%s) reading!\n", esp_err_to_name(err));
    }
  }
  // Close
  nvs_close(my_handle);
}

void setup()
{
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_BUTTON, INPUT);
  pinMode(RED_BUTTON, INPUT);
  pinMode(YELLOW_BUTTON, INPUT);
  // digitalWrite(RED_PIN, 1);
  // digitalWrite(GREEN_PIN, 1);
  led_state = RED_STATE;
  Serial.begin(9600);
  delay(1000);
  // Retrieve SSID/PASSWD from flash before anything else
  nvs_access();
  // We start by connecting to a WiFi network
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  // while (WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);
  //   Serial.print(WiFi.status());
  //   Serial.print(".");
  // }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("MAC address: ");
  Serial.println(WiFi.macAddress());

  Wire.begin();
  delay(1000);

  if (myIMU.begin())
    Serial.println("Ready.");
  else
  {
    Serial.println("Could not connect to IMU.");
    Serial.println("Freezing");
  }

  if (myIMU.initialize(BASIC_SETTINGS))
    Serial.println("Loaded Settings.");
  Serial.println("Starting BLE work!");
  // send to /login with the user
  std::string login_template = "?user=";
  std::string user = "WALDO";
  int err = 0;
  // WiFiClient c;
  // HttpClient http(c);
  // err = http.get("3.86.191.26", 5000, login_template + user, NULL);
}

void loop()
{
  int err = 0;
  int err1 = 0;
  WiFiClient c;
  HttpClient http(c);

  // std::string login_template = "?user=";
  std::string user = "WALDO";
  user = "?user" + user;
  err1 = http.get("3.86.191.26", 5000, user.c_str(), NULL);
  Serial.print(err1);

  Serial.print("\nAccelerometer:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatAccelX(), 3);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatAccelY(), 3);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatAccelZ(), 3);

  Serial.print("\nGyroscope:\n");
  Serial.print(" X = ");
  Serial.println(myIMU.readFloatGyroX(), 3);
  Serial.print(" Y = ");
  Serial.println(myIMU.readFloatGyroY(), 3);
  Serial.print(" Z = ");
  Serial.println(myIMU.readFloatGyroZ(), 3);

  switch (led_state)
  {
  case RED_STATE:
    // Turn red light on
    digitalWrite(RED_PIN, 1);
    break;
  case GREEN_STATE:
    // Turn green light on.
    digitalWrite(GREEN_PIN, 1);
    break;
  case RED_GREEN_STATE:
    digitalWrite(RED_PIN, 1);
    digitalWrite(GREEN_PIN, 1);
    break;
  }

  if (digitalRead(BLUE_BUTTON) == HIGH)
  {
    Serial.print("BLUE IS PRESSED");
  }
  if (digitalRead(RED_BUTTON) == HIGH)
  {
    Serial.print("RED IS PRESSED");
  }
  if (digitalRead(YELLOW_BUTTON) == HIGH)
  {
    Serial.print("YELLOW IS PRESSED");
  }

  // if (digitalRead(BLUE_BUTTON) == HIGH || digitalRead(RED_BUTTON) == HIGH /*|| digitalRead(YELLOW_BUTTON) == HIGH*/)
  // { // Start timer
  //   if (led_state = RED_STATE)
  //   {
  //     led_state = GREEN_STATE;
  //   }
  //   else
  //   {
  //     led_state = RED_STATE;
  //   }
  // }

  if (abs(myIMU.readFloatGyroX()) + abs(myIMU.readFloatGyroY()) + abs(myIMU.readFloatGyroZ()) > 50)
  {
    steps++;
  }

  Serial.print("\nThermometer:\n");
  Serial.print(" Degrees F = ");
  Serial.println(myIMU.readTempF(), 3);

  err = http.get("3.86.191.26", 5000, "?var=i", NULL);
  if (err == 0)
  {
    Serial.println("startedRequest ok");
    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);
      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get
      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        Serial.println("Body returned follows:");
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ((http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout))
        {
          if (http.available())
          {
            c = http.read();
            // Print out this character
            Serial.print(c);
            bodyLen--;
            // We read something, reset the timeout counter
            timeoutStart = millis();
          }
          else
          {
            // We haven't got any data, so let's pause to allow some to
            // arrive
            delay(kNetworkDelay);
          }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {
      Serial.print("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);
  }
  delay(500);
  // http.stop();
  // And just stop, now that we've tried a download

  delay(1000);
}
