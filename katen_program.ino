/*
  WiFiAccessPoint.ino creates a WiFi access point and provides a web server on it.

  Steps:
  1. Connect to the access point "yourAp"
  2. Point your web browser to http://192.168.4.1/H to turn the LED on or http://192.168.4.1/L to turn it off
     OR
     Run raw TCP "GET /H" and "GET /L" on PuTTY terminal with 192.168.4.1 as IP address and 80 as port

  Created for arduino-esp32 on 04 July, 2018
  by Elochukwu Ifediora (fedy0)
*/

#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>

#ifndef LED_BUILTIN
#define LED_BUILTIN 2  // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED
#endif

// Set these to your desired credentials.
const char *ssid = "yourAP";
const char *password = "yourPassword";

long start_time = 0;
long delta_time;

int dousa_jikan = 10;  //秒数で動作時間を指定

bool isMoving = false;

NetworkServer server(80);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
}

void loop() {

  delta_time = millis() - start_time;
  if (delta_time > (dousa_jikan * 1000)) {
    digitalWrite(LED_BUILTIN, LOW);
    isMoving = false;
  }


  NetworkClient client = server.accept();  // listen for incoming clients


  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";        // make a String to hold incoming data from the client
    while (client.connected()) {    // loop while the client's connected
      if (client.available()) {     // if there's bytes to read from the client,
        char c = client.read();     // read a byte, then
        Serial.write(c);            // print it out the serial monitor
        if (c == '\n') {            // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("<!DOCTYPE html><html lang=\"ja\">");
            client.print("<head><meta charset=\"UTF-8\">");  // 日本語エンコーディング指定
            client.print("<style>");
            client.print("a.button { display: inline-block; padding: 160px 160px; font-size: 150px; color: white; text-align: center; text-decoration: none; border-radius: 8px; transition: background-color 0.3s; }");
            client.print("a.button-on { background-color: #4CAF50; }");  // ONボタンの緑色
            client.print("a.button-on:hover { background-color: #45a049; }");
            client.print("a.button-off { background-color: #f44336; }");  // OFFボタンの赤色
            client.print("a.button-off:hover { background-color: #e53935; }");
            client.print("</style>");

            client.print("<script>");

            // JavaScriptで確認ポップアップを表示
            client.print("function confirmStart() {");
            client.print("  if (confirm('安全を確認してください．本当に開始しますか？')) {");
            client.print("    window.location.href = '/H';");

            client.print("  }");
            client.print("}");


            client.print("</script>");



            client.print(" <a href=\"javascript:confirmStart()\" class=\"button button-on\">Start</a><br>");
            client.print(" <a href=\"/L\" class=\"button button-off\">非常停止</a><br>");


            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {  // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(LED_BUILTIN, HIGH);  // GET /H turns the LED on
          if (isMoving == false) {
            isMoving = true;
            start_time = millis();
          }
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(LED_BUILTIN, LOW);  // GET /L turns the LED off
          isMoving = false;
        }
      }

      delta_time = millis() - start_time;
      if (delta_time > (dousa_jikan * 1000)) {
        digitalWrite(LED_BUILTIN, LOW);
        isMoving = false;
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
