#ifndef FIREBASE_SEND_DATA_H  // Unique identifier for this header file
#define FIREBASE_SEND_DATA_H

#include <WiFi.h>
#include <FirebaseClient.h>
#include <WiFiClientSecure.h>

#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASSWORD "your-wifi-pass"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "your-api-key"

// User Email and password that already registerd or added in your project.
#define USER_EMAIL "your-email"
#define USER_PASSWORD "your-password"
#define DATABASE_URL "your-database-url"

void asyncCB(AsyncResult &aResult);
void printError(int code, const String &msg);
bool sendToFirebase(AsyncClientClass &aClient, RealtimeDatabase &Database, String nodePath, String WaNumber);
bool intToFirebase(AsyncClientClass &aClient, RealtimeDatabase &Database, String nodePath, int lockerStatus);
int getFromFirebase(AsyncClientClass &aClient, RealtimeDatabase &Database, String nodePath);

#endif // TFT_FUNCTIONS_H