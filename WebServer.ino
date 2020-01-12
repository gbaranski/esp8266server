#include <timer.h>
auto timer = timer_create_default(); // create a timer with default settings

#include <ESP8266WiFi.h>
const char* ssid = "SSID";
const char* password = "PASS";

WiFiServer server(80);

int totalseconds = 60; // 10 mins
int displayseconds = 0; // seconds with formatting

bool TimerON = false; // Timer on/off

int displayminutes = 0; // Minutes with formatting

int LED = 16; // Output D0

void setup() {
  Serial.begin(115200); 
  
  
  pinMode(LED, OUTPUT); // Sets LED pin(D0) as a output
  digitalWrite(LED, LOW); // Sets LED OFF
  
  
  Serial.print("Connecting to the Newtork");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");  
  server.begin();  // Starts the Server
  Serial.println("Server started");
 
  Serial.print("IP Address of network: "); // Prints IP address on Serial Monitor
  Serial.println(WiFi.localIP());

  timer.every(1000, timer1); // Calls the timer1 function every 1000ms(1s)
}

void StartTimer() {
    digitalWrite(LED, HIGH); // Turn ON LED
    Serial.println("Timer started");
    TimerON = true; // Set timer to active
    totalseconds = 60; // Set totalseconds to 600 to start over
    delay(10); // Make LED blink for 10ms
    digitalWrite(LED, LOW); // Turn OFF LED
}

bool timer1(void *) { // Timer function
 
  totalseconds--; // Sets totalseconds(without formatting) to totalseconds - 1
 
  displayseconds = totalseconds%60;         // Formats the totalseconds to displayseconds
  displayminutes = totalseconds/60;         // Formats the totalseconds to displayminutes
  
  if(displayseconds==0){ // Change displayseconds to 60 when arrive 0
   if(totalseconds>0) {
   displayseconds = 60;
}
  }
  if(totalseconds==0){ // Prevents timer going into negative after totalseconds arriving 0
   TimerON = false;
}  
  
  //Serial.println(totalseconds);  
 // Serial.println(displayseconds); 
 // Serial.println(displayminutes);
  return true; // keep timer active? true
}

void loop() {
  
  if(TimerON == true) { // Checks if the Timer is ON, if ON then: 
  timer.tick(); // tick the timer
  }
  
  WiFiClient client = server.available(); // If client is connected then return checking it again, if not connected then print "Waiting for new client"
  if (!client)
  {
    return;
  }


  Serial.println(TimerON);

  String request = client.readStringUntil('\r'); // Reads request from client, for example reads that client clicked button
  Serial.println(request);
  client.flush();

  if(request.indexOf("/startTimer") != -1) // If the request is turning the LED on then:
  {
    StartTimer();
    const char *html = R"(
HTTP/1.1 200 OK
Content-Type: text/html

OK
    )";
    client.println(html); 
    return;
  }else if(request.indexOf("/getRemainingTime") != -1){
  const char *html = R"(
HTTP/1.1 200 OK
Content-Type: application/json
    )";
    client.println(html);
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
  const char *html = R"~~~(
HTTP/1.1 200 OK
Content-Type: text/html

  <!DOCTYPE html>
  <title>ESP8266 Water mixer</title>
  <html>
    <head>
      <meta charSet="utf-8"/><meta http-equiv="x-ua-compatible" content="ie=edge"/>
      <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no"/>
      <script>
setInterval(loadDoc, 1000);
function loadDoc() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            console.log(this.responseText);
            let response = JSON.parse(this.responseText);
            console.log(response);
            document.getElementById("minutes").innerHTML = response.displayMinutes;
            document.getElementById("seconds").innerHTML = response.displaySeconds;
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
<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}
p {font-size: 24px;color: #444444;margin-bottom: 10px;}
</style>
</head>

<body>
  <div id="webpage">
    <h1>ESP8266 NodeMCU Remote control</h1>
    <p>Minutes: <span id="minutes"></span></p>
    <p>Seconds: <span id="seconds"></span></p>
    <br/>
        <button type="button" onclick="startTimer()">Start Timer</button>
  </div>
</body>
</html>
  )~~~";
  client.println(html);
}
