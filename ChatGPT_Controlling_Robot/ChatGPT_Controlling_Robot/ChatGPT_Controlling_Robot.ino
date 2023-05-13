#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <NewPing.h>

// Replace with your network credentials
const char* ssid     = "The Promised LAN";
const char* password = "Boots128";

// Replace with your OpenAI API key
const char* apiKey = "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";

int dirPin1 = 12;
int dirPin2 = 13;

int dirPin3 = 27;
int dirPin4 = 14;

bool initialPrompt = true;
int initialDistance;
int wallDistance;
int goalDistance = 10;

String apiUrl = "https://api.openai.com/v1/chat/completions";

String inputText1 = "I am infront of a wall, I want to be less than 10cm away from the wall. Everytime you say forward I will move forward for to seconds and then respond with my new position. If you say small forward I will move forward for half a second and I will respond with my new position. If you say smallbackward I will move backward for half a second and respond with my new position. Only respond with one word unless I've reached my desired distance.";
String inputText2 = "I am infront of a wall, I want to be less than 10cm away from the wall. \n Everytime you say forward I will move forward for to seconds and then respond with my new position. \n If you say small forward I will move forward for half a second and I will respond with my new position. \n If you say smallbackward I will move backward for half a second and respond with my new position. \n Only respond with one word unless I've reached my desired distance.";

String intPayload = "{\"model\": \"gpt-3.5-turbo\",\"messages\": [{\"role\": \"user\", \"content\": \"" + inputText1 + "\"}]}";
String finalPayload;

#define TRIGGER_PIN  25  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     26  // Arduino pin tied to echo pin on the ultrasonic sensor.
int MAX_DISTANCE = 200;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
HTTPClient http;
  

void setup() {
  // Initialize Serial
  Serial.begin(9600);

  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(dirPin3, OUTPUT);
  pinMode(dirPin4, OUTPUT);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Connect to Wi-Fi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      delay(1000);
  }
  Serial.println("Connected");
  http.begin(apiUrl);
}
void loop() {
  if(initialPrompt)
  {
    Serial.print("ROBOT: ");
    Serial.println(inputText2);
    chatGptCall(intPayload);
    initialPrompt = false;
    wallDistance = getPing();
  }

    String tempPayload = "{\"role\": \"user\", \"content\": \"I am" + String(wallDistance) + "cm away from the wall\"}]}";
    chatGptCall(tempPayload);
    wallDistance = getPing();

}
void chatGptCall(String payload)
{
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + String(apiKey));
  
  if(initialPrompt)
  {
    finalPayload = payload;
  }
  else{
    finalPayload = finalPayload + "," + payload;
    //Serial.print("User:");
    //Serial.println(finalPayload);
  }
  int httpResponseCode = http.POST(finalPayload);
  
  if (httpResponseCode == 200) {
    String response = http.getString();
  
    // Parse JSON response
    DynamicJsonDocument jsonDoc(1024);
    deserializeJson(jsonDoc, response);
    String outputText = jsonDoc["choices"][0]["message"]["content"];
    outputText.remove(outputText.indexOf('\n'));
    Serial.print("CHATGPT: ");
    Serial.println(outputText);
    String returnResponse = "{\"role\": \"assistant\", \"content\": \"" + outputText + "\"}";
    getAction(outputText);
 //  if(initialPrompt)
 // {
    finalPayload = removeEndOfString(finalPayload);
 // }
  finalPayload = finalPayload + "," + returnResponse;
  //Serial.println(finalPayload);
  } 
  else {
    Serial.printf("Error %i \n", httpResponseCode);
  }
getDelay();

}
int getPing(){
  unsigned int uS = sonar.ping(); // Send ping, get ping time in microseconds (uS).
  int distance = uS / US_ROUNDTRIP_CM;
  Serial.print("ROBOT: ");
  Serial.print(distance); // Convert ping time to distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");
  return distance;
}
void getAction(String response)
{
  response.toLowerCase();
  if (response.indexOf("forward") >= 0) {
    //Serial.println("Forward!");
    digitalWrite(dirPin1, HIGH);
    digitalWrite(dirPin2, LOW);
    digitalWrite(dirPin3, HIGH);
    digitalWrite(dirPin4, LOW);
    getDelay();
        digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, LOW);
    digitalWrite(dirPin3, LOW);
    digitalWrite(dirPin4, LOW);
  
} 
  else if (response.indexOf("smallforward") >= 0) {
    //Serial.println("Small Forward!");
    digitalWrite(dirPin1, HIGH);
    digitalWrite(dirPin2, LOW);
    digitalWrite(dirPin3, HIGH);
    digitalWrite(dirPin4, LOW);
    getHalfDelay();
        digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, LOW);
    digitalWrite(dirPin3, LOW);
    digitalWrite(dirPin4, LOW);
}
  else if (response.indexOf("smallbackward") >= 0){
   // Serial.println("Small Backward!");
    digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, HIGH);
    digitalWrite(dirPin3, LOW);
    digitalWrite(dirPin4, HIGH);
    getHalfDelay(); 
        digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, LOW);
    digitalWrite(dirPin3, LOW);
    digitalWrite(dirPin4, LOW);
} 
  else {
    //Serial.println("Stoppped");
    digitalWrite(dirPin1, LOW);
    digitalWrite(dirPin2, LOW);
    digitalWrite(dirPin3, LOW);
    digitalWrite(dirPin4, LOW);
        while(1>0)
    {
      
    }
}
}
void getDelay(){
  unsigned long initialMillis = millis();
  while ((initialMillis + 500) >= millis()) {    
  }
}
void getHalfDelay()
{
    unsigned long initialMillis = millis();
  while ((initialMillis + 250) >= millis()) {    
  }
}
String removeEndOfString(String originalString)
{
  int stringLength = originalString.length();
  String newString = originalString.substring(0, stringLength - 2);
  return(newString);
}
