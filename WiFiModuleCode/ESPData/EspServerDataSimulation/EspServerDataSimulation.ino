/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-input-data-html-form/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Ticker.h>
AsyncWebServer server(80);

Ticker tick;
// Set your access point network credentials
const char* ssid = "BeOurGuest";
const char* password = "idontknowit";
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";


// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Meditation Time: <input type="text" name="input1">
    <br> <p> Maximum meditation time is 180 min </p>
    <br> Breathing Pattern: <input type="text" name="input2">
    <br> <p> 1 = 2 seconds inhale and exhale </p>
    <br> <p> 2 = 3 seconds inhale and 2 seconds exhale </p>
    <br> <p> 3 = 4 seconds inhale and 3 seconds exhale </p>
    <input type="submit" value="Submit">
  </form><br>
  <form action="/start">
    <input type="submit" onclick="myFunction()" value="Meditation Start" name="start">
    <p id="demo"</p>
  </form>
  <form action="/stop">
    <input type="submit" onclick="myFunction()" value="Meditation Stop" name="stop">
  </form>

  <script>
  function medStart() {
    document.getElementById("demo").innerHTML = "Meditation Started";
  }
  function medStop() {
    document.getElementById("demo").innerHTML = "Meditation Stopped";
  }
</script>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1 = "0";
    String inputMessage2 = "0";
    String inputParam1;
    String inputParam2;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputParam1 = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
    }
    else {
      inputMessage1 = "No message sent";
      inputMessage2 = "No message sent";
      inputParam1 = "none";
      inputParam2 = "none";
    }
    Serial.println(inputMessage1);
    Serial.println(inputMessage2);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on "
                                     "input field (" + inputParam1 + ")" +
                                     "with value: " + inputMessage1 +
                                     "<br> input field (" + inputParam1 + ") " +
                                     "with value: " + inputMessage2 +
                                     "<br><a href=\"/\">Return to Home Page</a>");

     handleIt(); // SEND IT TO UART HERE
  });

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {
  tick.once_ms<AsyncWebServerRequest *>(500, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain", "Success");
  }, request);
});
  server.onNotFound(notFound);
  server.begin();
}

void handleIt()
{
  Serial.println("You'redumb");
}

void loop() {
  
}
