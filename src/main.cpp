#include <timer.h>
auto timer = timer_create_default(); // create a timer with default settings

#include <ESP8266WiFi.h>
const char *ssid = "Nigeria";
const char *password = "hondamsx125";

WiFiServer server(80);

int totalseconds = 600; // 10 mins
int displayseconds = 0; // seconds with formatting

bool timerOn = false; // Timer on/off

int displayminutes = 0; // Minutes with formatting

int outputPin = 14; // Output D5

bool timer1(void *)
{ // Timer function

  totalseconds--; // Sets totalseconds(without formatting) to totalseconds - 1

  displayseconds = totalseconds % 60; // Formats the totalseconds to displayseconds
  displayminutes = totalseconds / 60; // Formats the totalseconds to displayminutes

  if (displayseconds == 0)
  { // Change displayseconds to 60 when arrive 0
    if (totalseconds > 0)
    {
      displayseconds = 60;
    }
  }
  if (totalseconds == 0)
  { // Prevents timer going into negative after totalseconds arriving 0
    timerOn = false;
  }

  return true; // keep timer active? true
}

void setup()
{
  Serial.begin(115200);

  pinMode(outputPin, OUTPUT);   // Sets LED pin(D0) as a output
  digitalWrite(outputPin, LOW); // Sets LED OFF

  Serial.print("Connecting to the Newtork");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  server.begin(); // Starts the Server
  Serial.println("Server started");

  Serial.print("IP Address of network: "); // Prints IP address on Serial Monitor
  Serial.println(WiFi.localIP());

  timer.every(1000, timer1); // Calls the timer1 function every 1000ms(1s)
}

void StartTimer()
{
  digitalWrite(outputPin, HIGH); // Turn ON LED
  Serial.println("Timer started");
  timerOn = true;               // Set timer to active
  totalseconds = 600;           // Set totalseconds to 600 to start over
  delay(500);                   // Make LED blink for 10ms
  digitalWrite(outputPin, LOW); // Turn OFF LED
}

void loop()
{

  if (timerOn == true)
  {               // Checks if the Timer is ON, if ON then:
    timer.tick(); // tick the timer
  }

  WiFiClient client = server.available(); // If client is connected then return checking it again, if not connected then print "Waiting for new client"
  if (!client)
  {
    return;
  }

  Serial.println(timerOn);

  String request = client.readStringUntil('\r'); // Reads request from client, for example reads that client clicked button
  Serial.println(request);
  client.flush();

  if (request.indexOf("/startTimer") != -1) // If the request is turning the LED on then:
  {
    StartTimer();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("");
    client.println("OK");
    return;
  }
  else if (request.indexOf("/getRemainingTime") != -1)
  {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("");
    client.println("{");
    client.print("\"displayMinutes\": ");
    client.print(displayminutes);
    client.println(",");
    client.print("\"displaySeconds\": ");
    client.println(displayseconds);
    client.println("}");
    return;
  }

  /*------------------HTML Page Creation---------------------*/

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("");
  const char *html = R"~~~(
  <!DOCTYPE html>
  <html>
    <head>
      <meta charSet="utf-8"/><meta http-equiv="x-ua-compatible" content="ie=edge"/>
      <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no"/>
      <meta name=”format-detection” content=”telephone=no”>
      <meta name=”format-detection” content=”date=no”>
      <meta name=”format-detection” content=”address=no”>
      <script>
setInterval(loadDoc, 1000);

function loadDoc() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            console.log(this.responseText);
            try{
              let response = JSON.parse(this.responseText);
              console.log(response);
              document.getElementById("minutes").textContent = response.displayMinutes;
              document.getElementById("seconds").textContent = response.displaySeconds;
            }catch(e){
              alert(e);
            }
        }
    }
    xhttp.open("GET", "/getRemainingTime", true);
    xhttp.send();
};

function startTimer() {
    var oReq = new XMLHttpRequest();
    oReq.open("GET", "/startTimer");
    oReq.send();
};
      </script> 
<title>ESP8266 Water mixer</title>
<style>html { font-family: Courier; display: inline-block; margin: 0px auto; text-align: center;}
body{margin-top: 50px;} h1 {color: #E5E5E5;margin: 50px auto 30px;}
p {font-size: 30px;color: #E5E5E5;margin-bottom: 10px;}
body {
  background-color: #084C61;
}
.button  {
  background-color: #212121;
  color: #fafafa;
  border-radius: 12px;
  font-size: 64px;
  -webkit-transition-duration: 0.4s;
  transition-duration: 0.4s;
  background-color: #212121; 
  border: 3px solid #212121;
  font-family: Courier;
}
.button:hover {
  cursor: pointer;
}
.button:active {
  background-color: #505050;
  box-shadow: 0 5px #666;
  transform: translateY(4px);
}
</style>
</head>

<body>
  <div id="webpage">
    <h1>ESP8266 NodeMCU Remote control</h1>
    <p class="timeindicator">Minutes: <span id="minutes">0</span></p>
    <p class="timeindicator">Seconds: <span id="seconds">0</span></p>
    <br>
        <button class="button" onclick="startTimer()">Start Timer</button>
  </div>
</body>
</html>
  )~~~";
  client.println(html);
}