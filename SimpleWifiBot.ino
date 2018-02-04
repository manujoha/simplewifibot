#include <ESP8266WiFi.h>

/// **************************************
// CHANGE HERE YOUR WIFI SSID and PASSWORD
/// **************************************
const char* ssid = "WIFI SSID";
const char* password = "WIFI PASSWORD";

WiFiServer server(80);

// Modes for status led etc..
#define MODE_RUN 1
#define MODE_READY 2
int currentMode = MODE_READY;

#define RMS D1 // Right Motor Speed
#define RMD D3  // Right Motor Direction
#define LMS D2  // Left Motor Speed
#define LMD D4 // Left Motor Direction

#define LOW_SPEED_R 100
#define LOW_SPEED_L 100
#define HIGH_SPEED_R 400
#define HIGH_SPEED_L 400
#define STOP 0

#define VERSION "0.2"

// Command array 
int commands[100];

void setup() {
  Serial.begin(115200);
  Serial.println("-----------------------");
  Serial.println("  ROBOT INITIALIZING   ");
  Serial.println("-----------------------");

  // Set motor pins to output
  pinMode(RMS, OUTPUT);
  pinMode(LMS, OUTPUT);
  pinMode(RMD, OUTPUT);
  pinMode(LMD, OUTPUT);

  // Shut down motors and change direction to forward
  digitalWrite(RMS, LOW);
  digitalWrite(LMS, LOW);
  digitalWrite(RMD, HIGH);
  digitalWrite(LMD, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // wait for connection and blink led when connecting
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  digitalWrite(LED_BUILTIN, HIGH);

  // start the server
  server.begin();
  Serial.println("Server started");

  // all ready
  // Print the IP address to serial output
  Serial.print("Robot ready. Open this URL in browser: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  setMode(MODE_READY);
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends request
  Serial.println("Rew request");
  while (!client.available()) {
    delay(1);
  }

  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Beginning of the http response
  client.println("HTTP/1.1 200 OK");
  client.println("Server: Simple Wifi Bot v0.1");
  client.println("Connection: close");
  client.println("Content-Type: text/html");
  client.println("");

  // Handle the request with commands
  if (request.indexOf("/execute?command=") != -1) {
    // send response
    client.println("OK");
    delay(1);
    
    // Read commands from parameters
    parseCommands(request, request.indexOf("=")+1, request.indexOf(" HTTP"));
    executeCommands();
  } else {
    // Default response, print control panel HTML
    client.println("<!DOCTYPE html>");
    client.println("<html>");
    client.println("<head>");
    client.println("<meta http-equiv='content-type' content='text/html; charset=UTF-8'>");
    client.println("<meta name='robots' content='noindex, nofollow'>");
    client.println("<meta name='googlebot' content='noindex, nofollow'>");
    client.println("<link rel='stylesheet' type='text/css' href='//maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta.2/css/bootstrap.min.css'>");
    client.println("<link rel='stylesheet' type='text/css' href='//maxcdn.bootstrapcdn.com/font-awesome/4.5.0/css/font-awesome.min.css'>");
    client.println("<script type='text/javascript' src='https://code.jquery.com/jquery-3.2.1.min.js'></script>");
    client.println("<script type='text/javascript' src='//cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.3/umd/popper.min.js'></script>");
    client.println("<script type='text/javascript' src='//maxcdn.bootstrapcdn.com/bootstrap/4.0.0-beta.2/js/bootstrap.min.js'></script>");
    client.println("<link rel='icon' href='data:;base64,iVBORw0KGgo='>");
    client.println("<style type='text/css'>");
    client.println("body { background-color: #242b51; color: white;}");
    client.println(".btn { font-size: 2rem!important;} ");
    client.println("#cmd-play { } ");
    client.println("#command-selection { padding: 10px; font-size: 3em;}");
    client.println("#command-area { padding: 10px; min-height: 3em; border: 1px dashed #4e95b6;margin-bottom: 20px;}");
    client.println("#command-area i{ display: inline-block; color: #c5da3a; font-size: 1.5rem; margin-right: 20px;}");
    client.println("#call, #response { color: #8cb7c3;}");
    client.println("</style>");
    client.println("<title>Robotti-ohjain v");
    client.println(VERSION);
    client.println("</title>");
    client.println("<script type='text/javascript'>");
    client.println("var BASEURL = '';");
    client.println("var commands = '';");
    client.println("");
    client.println("$(document).ready(function() {");
    client.println("$('.cmd').click(function() {");
    client.println("var icon = $(this).html();");
    client.println("$('#command-area').append(icon);");
    client.println("if (commands.length <= 100) {");
    client.println("commands += $(this).attr('id').substring(4);");
    client.println("} else {");
    client.println("alert('100 komentoa on maksimi.');");
    client.println("}");
    client.println("});");
    client.println("");
    client.println("$('#cmd-play').click(function() {");
    client.println("if (commands.length == 0) { return;}");
    client.println("var requestUrl = BASEURL + '/execute?command=' + commands;");
    client.println("$('#call').append('GET ' + requestUrl+'\\n');");
    client.println("$.ajax({");
    client.println("url: requestUrl,");
    client.println("success: function(result) {");
    client.println("$('#response').append(result);");
    client.println("},");
    client.println("error: function(result) {");
    client.println("$('#response').append('ERROR\\n');");
    client.println("},");
    client.println("timeout:(commands.length*3000)");
    client.println("});");
    client.println("});");
    client.println("");
    client.println("$('#cmd-stop').click(function() {");
    client.println("commands = '';");
    client.println("$('#command-area').html('');");
    client.println("$('#response').html('');");
    client.println("$('#call').html('');");
    client.println("});");
    client.println("});");
    client.println("</script>");
    client.println("</head>");
    client.println("<body>");
    client.println("<div class='cover-container'>");
    client.println("<div class='container'>");
    client.println("<h1 class='display-4'>");
    client.println("Robotti-ohjain v");
    client.println(VERSION);
    client.println("</h1>");
    client.println("<p class='lead'>Ohje: Valitse korkeintaan 100 ohjauskomentoa.</p><p class='lead'><i class='fa fa-play' aria-hidden='true'></i> suorittaa komennot.</p>");
    client.println("<p class='lead'><i class='fa fa-trash' aria-hidden='true'></i> tyhjentää komennot</p>");
    client.println("<p class='lead'><i class='fa fa-repeat' aria-hidden='true'></i> toistaa edelliset liikkumiskomennot x kertaa</p>");
    client.println("</div>");
    client.println("</div>");
    client.println("<div class='container'>");
    client.println("<div id='command-selection' class='row'>");
    client.println("<div class='col-sm-8'>");
    client.println("<button id='cmd-1' class='cmd btn btn-info'>");
    client.println("<i class='fa fa-arrow-up' aria-hidden='true'></i>");
    client.println("</button>");
    client.println("<button id='cmd-2' class='cmd btn btn-info'>");
    client.println("<i class='fa fa-arrow-left' aria-hidden='true'></i>");
    client.println("</button>");
    client.println("<button id='cmd-3' class='cmd btn btn-info'>");
    client.println("<i class='fa fa-arrow-right' aria-hidden='true'></i>");
    client.println("</button>");
    client.println("<button id='cmd-4' class='cmd btn btn-info'>");
    client.println("<i class='fa fa-arrow-down' aria-hidden='true'></i>");
    client.println("</button>");
    client.println("<button id='cmd-5' class='cmd btn btn-info'>");
    client.println("<i class='fa fa-repeat' aria-hidden='true'>x2</i>");
    client.println("</button>");
    client.println("<button id='cmd-6' class='cmd btn btn-info'>");
    client.println("<i class='fa fa-repeat' aria-hidden='true'>x5</i>");
    client.println("</button>");
    client.println("<button id='cmd-7' class='cmd btn btn-info'>");
    client.println("<i class='fa fa-stop' aria-hidden='true'></i>");
    client.println("</button>");
    client.println("</div>");
    client.println("<div class='col-sm-4 text-right'>");
    client.println("<button id='cmd-play' class='btn btn-success'>");
    client.println("<i class='fa fa-play' aria-hidden='true'></i>");
    client.println("</button>");
    client.println("<button id='cmd-stop' class='btn btn-danger'>");
    client.println("<i class='fa fa-trash' aria-hidden='true'></i>");
    client.println("</button>");
    client.println("</div>");
    client.println("</div>");
    client.println("<div class='panel panel-default'>");
    client.println("<div class='panel-heading'>");
    client.println("<br/>");
    client.println("<h4 class='panel-title'>Komennot</h4>");
    client.println("</div>");
    client.println("<div class='panel-body' id='command-area' >");
    client.println("</div>");
    client.println("</div>");
    client.println("<div class='row'>");
    client.println("<div class='col-sm-6'>");
    client.println("<pre id='call'></pre>");
    client.println("</div>");
    client.println("<div class='col-sm-6'>");
    client.println("<pre id='response'></pre>");
    client.println("</div>");
    client.println("</div>");
    client.println("</div>");
    client.println("</body>");
    client.println("</html>");
  }

  delay(1);
  Serial.println("Client disconnected");
  Serial.println("");
}

/**
 * Parses commands from string input to integer array
 */
String c = "";
void parseCommands(String data, int startIndex, int endIndex)
{
  int counter = 0;
  Serial.print("Data: ");
  Serial.print(data);
  Serial.print(" ");
  Serial.print(startIndex);
  Serial.print(" ");
  Serial.println(endIndex);
  Serial.print("Parsing commands to integer: ");
  for (int i = startIndex; i < endIndex; i++) {
      c += data[i];
      commands[counter] = c.toInt();
      
      Serial.print(c);
      Serial.print("=");
      Serial.print(commands[counter]);
      Serial.print(", ");
      counter ++;
      c = "";
  }
  Serial.println(".");

  // fill rest with minus one 
  for (int i = counter; i < 100; i++) {
    commands[i] = -1;
  }
}

/**
 * Turns right for given duration in ms
 */
void turnRight(int duration) {
  Serial.println(">> Turn left");
  digitalWrite(RMS, LOW_SPEED_R);
  digitalWrite(LMS, LOW_SPEED_L);
  digitalWrite(RMD, LOW);
  digitalWrite(LMD, HIGH);
  if (duration > 0) {
    delay(duration);
    brake();
  }
}

/**
 * Turns left for given duration in ms
 */
void turnLeft(int duration) {
  Serial.println(">> Turn right");
  digitalWrite(RMS, LOW_SPEED_R);
  digitalWrite(LMS, LOW_SPEED_L);
  digitalWrite(RMD, HIGH);
  digitalWrite(LMD, LOW);

  if (duration > 0) {
    delay(duration);
    brake();
  }
}

/**
 * Stop all motors and sets robot to ready state
 */
void stopMotors() {
  Serial.println(">> Stop motors");
  digitalWrite(LMS, STOP);
  digitalWrite(RMS, STOP);
   // Forward
  digitalWrite(RMD, HIGH);
  digitalWrite(LMD, HIGH);
  setMode(MODE_READY);
}

/**
 * Brake with 50ms delay. Used as break between commands
 */
void brake() {
  Serial.println(">> Brake");
  
  digitalWrite(RMS, STOP);
  digitalWrite(LMS, STOP);
  delay(50);
  // Forward
  digitalWrite(RMD, HIGH);
  digitalWrite(LMD, HIGH);
}

int rerunx2 = -1; // -1 == loop can be activated
int rerunx5 = -1; // 0 == loop is stopped
int rerunx20 = -1; // 0 < loop is active

/**
 * Command handler
 * Executes basic commands represented in array of integers  
 */
void executeCommands() {
  Serial.println("> Executing commands");

  rerunx2 = -1; // -1 == loop can be activated
  rerunx5 = -1; // -2 & 0 == loop is stopped
  rerunx20 = -1; // 0 < loop is active

  int lastLoopIndex = -1;
  
  setMode(MODE_RUN);
  for (int i = 0; i < sizeof(commands) ; i ++) {
    Serial.print("> Action number: ");
    Serial.print(i);
    Serial.print(" action value: ");
    Serial.println(commands[i]);
    switch (commands[i]) {
      case 0:
        brake();
        break;
      case 1:
        forward(150);
        break;
      case 2:
        turnLeft(50);
        break;
      case 3:
        turnRight(50);
        break;
      case 4:
        backward(150);
        break;
      case 5:
        Serial.println(">> Loop x2");
        // reset loop counter if this is new loop
        if(i != lastLoopIndex && rerunx2 == -2){
          rerunx2 = -1;
        }
        
        rerunx2 = performLoop(rerunx2, 1);
        if(rerunx2 >= 0){
          i = lastLoopIndex;
          Serial.print(">> Jump to ");
          Serial.println(i);
        } else {
          lastLoopIndex = i;
        }
        break;  
      case 6:
        Serial.println(">> Loop x5");
        // reset loop counter if this is new loop
        if(i != lastLoopIndex && rerunx5 == -2){
          rerunx5 = -1;
        }
        rerunx5 = performLoop(rerunx5, 5);
        if(rerunx5 >= 0){
          i = lastLoopIndex;
          Serial.print(">> Jump to ");
          Serial.println(i);
        } else {
          // current loop has ended, mark this index as last loop
          lastLoopIndex = i;
        }
        break;
     case 7: 
        brake();
        delay(400);
        break;
      case -1:
        Serial.println(">> Stop");
        return;
    }
    delay(200); // let bot rest a little bit before next command
  }
}

/**
 * Loop handler c = current loop status, l = maximum
 * Returns loop status as: 
 * -2 == loop is stopped
 * -1 == loop can be activated
 *  0 < loop is active
 * 
 */
int performLoop(int c, int l) {
  Serial.print("Current = ");
  Serial.print(c);
  Serial.print(" Total = ");
  Serial.print(l);
  if(c > 0){
    // continue loop
    Serial.println(" --> Continue loop");
    c--;
    return c;  
  } else if (c == -1){
    Serial.println(" --> Start loop");
    // start loop
    c = l; // set current counter to max
    c--; // reduce counter
    return c;
  } else {
    Serial.println(" --> Stop loop");
    // stop loop (c==0)
    return -2;
  }
}

/**
 * Moves robot forward for given duration in milliseconds. Stops after duration (milliseconds) and starts smoothly.
 * If duration=0, keeps running forward and doesn't stop.
 */
void forward(int duration) {
  Serial.println(">> Forward");
  
  if (duration > 0) {
    digitalWrite(RMD, HIGH);
    digitalWrite(LMD, HIGH);
    for(int i = 1 ;  i < 11 ; i ++ ){
      //Serial.print("Speed ");
      //Serial.println((LOW_SPEED_R / 10) * i);
      digitalWrite(RMS, (LOW_SPEED_R / 10) * i);
      digitalWrite(LMS, (LOW_SPEED_L / 10) * i);
      delay(duration / 10);
    }
    brake();
  } else {
    digitalWrite(RMS, LOW_SPEED_R);
    digitalWrite(LMS, LOW_SPEED_L);
    digitalWrite(RMD, HIGH);
    digitalWrite(LMD, HIGH);
   }
}

/**
 * Moves robot backwards. Stops after duration (milliseconds) and starts smoothly.
 * If duration=0, keeps running and doesn't stop.
 */
void backward(int duration) {
  if (duration > 0) {
    digitalWrite(RMD, LOW);
    digitalWrite(LMD, LOW);
    for(int i = 1 ;  i < 11 ; i ++ ){
      Serial.print("Speed ");
      Serial.println((LOW_SPEED_R / 10) * i);
      digitalWrite(RMS, (LOW_SPEED_R / 10) * i);
      digitalWrite(LMS, (LOW_SPEED_L / 10) * i);
      delay(duration / 10);
    }
    brake();
  } else {
    digitalWrite(RMS, LOW_SPEED_R);
    digitalWrite(LMS, LOW_SPEED_L);
    digitalWrite(RMD, LOW);
    digitalWrite(LMD, LOW);
   }
}

/**
 * Mode helper
 */
void setMode(int mod) {
  currentMode = mod;

  if (currentMode == MODE_RUN) {
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (currentMode == MODE_READY) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
