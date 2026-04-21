#include "firebase_send_data.h"

void asyncCB(AsyncResult &aResult){
    if (aResult.appEvent().code() > 0)
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }
}

void printError(int code, const String &msg){
    Firebase.printf("Error, msg: %s, code: %d\n", msg.c_str(), code);
}

bool sendToFirebase(AsyncClientClass &aClient, RealtimeDatabase &Database, String nodePath, String WaNumber){
  bool sendStatusToFirebase = Database.set<String>(aClient, nodePath, WaNumber);
  if (sendStatusToFirebase){
    Serial.println("Set string is ok");
    return sendStatusToFirebase;
  }else{
    printError(aClient.lastError().code(), aClient.lastError().message());
  }
}

bool intToFirebase(AsyncClientClass &aClient, RealtimeDatabase &Database, String nodePath, int lockerStatus){
  bool intStatusToFirebase = Database.set<int>(aClient, nodePath, lockerStatus);
  if (intStatusToFirebase){
    Serial.println("Set int is ok");
    return intStatusToFirebase;
  }else{
    printError(aClient.lastError().code(), aClient.lastError().message());
  }
}

int getFromFirebase(AsyncClientClass &aClient, RealtimeDatabase &Database, String nodePath){
  int dataGetFromFirebase = Database.get<int>(aClient, nodePath);
    if (aClient.lastError().code() == 0){
      Serial.println(dataGetFromFirebase);
      return dataGetFromFirebase;
    } else {
      printError(aClient.lastError().code(), aClient.lastError().message());
    }  
}