#include "firebase_send_data.h" 
#include "hive_mqtt.h"

//-----------All Data and Variable for Firebase-------------//
DefaultNetwork network;
FirebaseApp app; 
WiFiClientSecure ssl_client;
RealtimeDatabase Database;
AsyncClientClass aClient(ssl_client, getNetwork(network));
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
unsigned long ms;

WiFiClientSecure espClient;  
PubSubClient client(espClient);

bool isClosed = false;
int lockerLockPin[6][5] = {
  {23,  0, 22, 0, 0},
  {21,  0, 19, 0, 0},
  {18,  0,  4, 0, 0},
  {32,  0, 33, 0, 0},
  {25,  0, 26, 0, 0},
  {27,  0, 14, 0, 0},
  // {13,  0,  0, 0, 0},
  // {13,  0,  0, 0, 0},
  // {13,  0,  0, 0, 0},
};

void setup() {
  Serial.begin(115200);
  turnOnConnection();
  establishFirebaseRequest();
  espClient.setCACert(root_ca);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  for(int pinRel = 0; pinRel < 6; pinRel++){
    pinMode(lockerLockPin[pinRel][0], OUTPUT);
    pinMode(lockerLockPin[pinRel][2], INPUT_PULLUP);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  for(int pinHall = 0; pinHall < 6; pinHall++){
    lockerLockPin[pinHall][3] = digitalRead(lockerLockPin[pinHall][2]);
    if(lockerLockPin[pinHall][3] == 0 && lockerLockPin[pinHall][1] == 1 && lockerLockPin[pinHall][4] == 0){
      Serial.println("test");
      lockerLockPin[pinHall][1] = 0;
      digitalWrite(lockerLockPin[pinHall][0],lockerLockPin[pinHall][1]);
      intToFirebase(aClient, Database, String("/Locker List/Locker ") + String(pinHall + 1) + String("/Lock"), lockerLockPin[pinHall][1]);
      lockerLockPin[pinHall][4] = 1;
      //isClosed = true;
    } 
    else if (lockerLockPin[pinHall][3] == 1){
      lockerLockPin[pinHall][4] = 0;
      //isClosed = false;
    }
  }
}

//----------Connecting to Firebase Database------------//
void establishFirebaseRequest(){
  Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
  Serial.println("Initializing app...");
  ssl_client.setInsecure();
  app.setCallback(asyncCB);
  initializeApp(aClient, app, getAuth(user_auth));
  ms = millis();
  while (app.isInitialized() && !app.ready() && millis() - ms < 120 * 1000);
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);
  Serial.println("Synchronous Set... ");
}
//----------Connecting to Wifi------------//
void turnOnConnection() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//----------response to MQTT--------------//
void callback(char* topic, byte* payload, unsigned int length) {
  String hiveMqPayload;
  String hiveMqTopic = String(topic);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    hiveMqPayload += ((char)payload[i]);
  }
  Serial.println(hiveMqPayload);

  for(int lockerNumber = 0; lockerNumber < 6; lockerNumber++){
    if(hiveMqTopic == String("/Locker ") + String(lockerNumber + 1) + String("/Lock")){
      lockerLockPin[lockerNumber][1] = hiveMqPayload.equals("1");
      Serial.println(hiveMqTopic); // change/del this later
      Serial.println(lockerLockPin[lockerNumber][0]);
      Serial.println(lockerLockPin[lockerNumber][1]); // change/del this later
      digitalWrite(lockerLockPin[lockerNumber][0],lockerLockPin[lockerNumber][1]);
      //Serial.println("Loker Terbuka"); // change/del this later
    }
  }
}
//----------sub and pub to MQTT------------//
void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection… ");
    String clientId = "ESP32Client";
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected!");
      for (int subNumber = 0; subNumber < 6; subNumber++){
        String subPath = String("/Locker ") + String(subNumber + 1) + String("/Lock");
        //const char* subChar = subPath.c_str();
        client.subscribe(subPath.c_str());
      }
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
