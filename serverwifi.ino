#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>   // Include the WebServer library
#include <Servo.h>


IPAddress server_addr(37,59,55,185);  // IP of the MySQL *server* here
#define db_user "85oFV6Veiy"
#define ssid "Ako Budoy"
#define pass "@Rj09058491368"
const char* serverName = "https://smartbinhome.herokuapp.com/"

//ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
#define echoPin D0 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin D1 //attach pin D3 Arduino to pin Trig of HC-SR04
#define echoPin2 D2
#define trigPin2 D3
#define servo D4
#define greenled D5
#define redled D6

//global variables
Servo myservo;  // create servo object to control a servo
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
long duration2; // variable for the duration outside sensor
int distance2; // variable for the distance outside sensor
int inside=0;
int servo=0;
ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80
WiFiClient client;
MySQL_Connection conn((Client *)&client);

//function prototypes
void ledStatus();
void servoStatus();
void startBin();
void binstatus();
void outsideSensor();
void handleRoot();              
void handleNotFound();
void sendDataToDB();
void changeLedStatus();

void setup(void) {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');
  myservo.attach(servo);
  myservo.write(180);       //initialize servo position
  //led pins
  pinMode(greenled, OUTPUT);
  pinMode(redled, OUTPUT);
  //inside sensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  //outside sensor
  pinMode(trigPin2, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin2, INPUT); // Sets the echoPin as an INPUT
  
  WiFi.begin(ssid, pass);

  Serial.println("Connecting ...");
  
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print('.');
  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
  if (conn.connect(server_addr, 3306, db_user,"Lw00hmh6eQ")) {
      delay(1000);
  }else{
      Serial.println("Connection failed.");
  }
  startBin();
  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  server.on("/ledstatus", ledStatus);
  server.on("/servostatus", servoStatus);
  server.on("/binstatus", binstatus);
  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();         // Listen for HTTP requests from clients
  digitalWrite(greenled, LOW);
  digitalWrite(redled, LOW);
  // condition: not full - continue reading
  if(inside==0){
    digitalWrite(greenled, HIGH);
    // Clears the trigPin condition
    digitalWrite(trigPin, LOW);
    delay(10);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin, HIGH);
    delay(1000);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2;
    if(distance<5 && distance>=2){
      inside=1;
      sendDataToDB();
      changeLedStatus();
      digitalWrite(trigPin, LOW);
    }
      outsideSensor();
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  }else{
    digitalWrite(trigPin2, LOW);
    digitalWrite(redled, HIGH);
  }
  
}
void outsideSensor(){
    digitalWrite(trigPin2, LOW);
    delayMicroseconds(10);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(500);
    digitalWrite(trigPin2, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration2 = pulseIn(echoPin2, HIGH);
    // Calculating the distance
    distance2 = duration2 * 0.034 / 2;
    if(distance2<20 && distance2>5){
      myservo.write(0);
      delay(3000);
    }else{
      myservo.write(180);
    }
    Serial.print("Distance2: ");
    Serial.print(distance2);
    Serial.println(" cm");
}
void handleRoot() {
  server.send(200, "text/plain", "Hello world!");   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
void startBin(){
  row_values *row = NULL;
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  char *query="SELECT * FROM smart_bin.led_status LIMIT 1;";
  cur_mem->execute(query);
  // Fetch the columns (required) but we don't use them.
  column_names *columns = cur_mem->get_columns();
  do {
    row = cur_mem->get_next_row();
    if (row != NULL) {
      inside = atol(row->values[1]);
    }
  } while (row != NULL);
  Serial.print("Inside status: ");
  Serial.println(inside);
  delete cur_mem;
}
void changeLedStatus(){
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  char *query="UPDATE smart_bin.led_status SET status = 1";
  cur_mem->execute(query);
  delete cur_mem;
}
void ledStatus(){
  String val = server.arg("val");
  if(val=="1"){
    inside=1;
  }else{
    inside=0;
  }
  server.send(200, "text/plain", "Success");
}
void servoStatus(){
  String val = server.arg("val");
  if(val=="1"){
    myservo.write(0);
  }else{
    myservo.write(180);
  }
  server.send(200, "text/plain", "Success");
}
void binstatus(){
  String stat="not full";
  if(distance<5 && distance>=2){
    stat="full";
  }
  String json = "{\"distance\":\"" + String(distance) + "\",";
  json += "\"status\":\"" + stat + "\"}";
  server.send(200, "application/json", json);
}
void sendDataToDB(){
  char *stat="not full";
  if(distance<5 && distance>=2){
    stat="full";
  }
  char query[150];
  sprintf(query, "INSERT INTO smart_bin.bin_status(cm_value, status) VALUES('%d','%s');", distance, stat);
  Serial.print(query);
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  cur_mem->execute(query);
  delete cur_mem;
}
//#include <ESP8266WiFiMulti.h>
//  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
  //    delay(250);
  //    Serial.print('.');
  //  }
 //  wifiMulti.addAP(ssid, pass);   // add Wi-Fi networks you want to connect to
//
//  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
//    Serial.println("mDNS responder started");
//  } else {
//    Serial.println("Error setting up MDNS responder!");
//  }
//#include "ESP8266WiFi.h"
//const char* ssid = "Ako Budoy"; //Enter SSID
//const char* password = "@Rj09058491368"; //Enter Password
//
//void setup(void)
//{ 
//  Serial.begin(115200);
//  // Connect to WiFi
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) 
//  {
//     delay(500);
//     Serial.print("*");
//  }
//  
//  Serial.println("");
//  Serial.println("WiFi connection Successful");
//  Serial.print("The IP Address of ESP8266 Module is: ");
//  Serial.print(WiFi.localIP());// Print the IP address
//}
//
//void loop() 
//{
//  // EMPTY
//}
