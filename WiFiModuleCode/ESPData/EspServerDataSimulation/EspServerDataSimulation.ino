/*The following project used some HTLM code from the following project found online:
 * https://randomnerdtutorials.com/esp32-access-point-ap-web-server/
 * 
 * 
 * 
 * 
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#define MedTimeDefault (20)
#define MedTimeMax (180)
#define songMax (5)

AsyncWebServer server(80);

// Set your access point network credentials
const char* ssid = "GalaxyS6";
const char* password = "qjxu3445";
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

// Handles the Inputs on the WebPage
// Base code taken from Project mentioned in description
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Meditation Time: <input type="text" name="input1">
    <br> <p> Maximum meditation (20 to 180 minutes) </p>
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

<form action="/getMusic">
    Music Selection: <input type="text" name="input3">
    <br> <p> Between 1 and 4 </p>
    <input type="submit" value="Submit">
  </form><br>
  
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
  Serial.begin(9600);
  // setup UART Tx PINS
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
  }
  //Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to esp module and get respective meditation time and
  // breathing pattern and send it through uart.
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String medTime = "0";
    String breathSet = "0";
    String inputParam1;
    String inputParam2;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      medTime = request->getParam(PARAM_INPUT_1)->value();
      inputParam1 = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      breathSet = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
    }
    else {
      medTime = "No message sent";
      breathSet = "No message sent";
      inputParam1 = "none";
      inputParam2 = "none";
    }
    request->send(200, "text/html", "HTTP GET request sent to your ESP on "
                                     "input field (" + inputParam1 + ")" +
                                     "with value: " + medTime +
                                     "<br> input field (" + inputParam1 + ") " +
                                     "with value: " + breathSet +
                                     "<br><a href=\"/\">Return to Home Page</a>");
     if(medTime != "" && breathSet != "")
     {                                
      handleUart(medTime.toInt(), breathSet.toInt()); // SEND IT TO UART HERE
     }
  });

  // sends the start request to the client device
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "Meditation started <br><a href=\"/\">Return to Home Page</a>");
    // send on information
    handleOnOff(true);
  });

  // sends the stop request to the client device
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", "Meditation stopped <br><a href=\"/\">Return to Home Page</a>");
    // Send off information
    handleOnOff(false);
  });

server.on("/getMusic", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String songSelect = "0";
    String inputParam1;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_3)) {
      songSelect = request->getParam(PARAM_INPUT_3)->value();
      inputParam1 = PARAM_INPUT_3;
    }
    else {
      songSelect = "0"; // default song to off
      inputParam1 = "none"; // no song selected
    }
    request->send(200, "text/html", "HTTP GET request sent to your ESP on "
                                     "Music field (" + inputParam1 + ")" +
                                     "with value: " + songSelect +
                                     "<br><a href=\"/\">Return to Home Page</a>");
     if(songSelect != "")
     {                                  
      handleMusicUart(songSelect.toInt()); // SEND IT TO UART HERE. returns 0 if non-numeric value
     }
  });
  
  server.onNotFound(notFound);
  server.begin();
}

// Handles the on/off data Rx to the kl25z
void handleOnOff(bool intendedOn)
{
  uint8_t zenData = 128; // default off.
  if(intendedOn)
  {
    zenData = 0x01; // On.
  }
  else
  {
    zenData = 0x00;
  }
  Serial.write(0);
  delay(50);
  Serial.write(zenData); // on off
  Serial.flush();
}

void handleMusicUart(uint8_t musicSel)
{
  uint8_t zenData = 0;

  zenData = musicSel % songMax;

  Serial.write(2); // send it a song
  delay(50);
  Serial.write(zenData);
  Serial.flush();
}

// Handles UART Rx to the KL25z
void handleUart(uint8_t med_Time, uint8_t breathPattern)
{
  uint8_t zenData = 0; // defaults
  // max is 180 Minutes
  if(med_Time > MedTimeMax)
  {
    // Default 20 minutes subtracted from 180
    med_Time = 150; // max time Add 10 when read by kl25z
  }
  else if(med_Time < MedTimeDefault)
  {
    med_Time = 0; // Add no time to the default. 
  }
  else
  {
    // do nothing
  }
  med_Time = uint8_t(round((float)med_Time / (float)10));
  // Get a value between 0-31. 

  if(breathPattern > 3)
  {
    breathPattern = 3;
  }
  else
  {
    // do nothing.
  }
  zenData = med_Time; // assign time to zenData
  zenData |= (breathPattern << 4); // shift to correct position
  //zenData &= 0x7F; // 8th bit is 0 to indicate a settings update only
  Serial.write(1); // zendata sent
  delay(50);
  Serial.write(zenData); // System is off, some time set
  Serial.flush();
}

void loop() {
  
}
