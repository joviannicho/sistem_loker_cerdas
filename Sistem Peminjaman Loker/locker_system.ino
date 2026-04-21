#include "firebase_send_data.h"
#include "camera_set_up.h"
#include "qr_scanner.h"
#include "tft_function.h"

//------------All Data and Variable for TFT Screen------------//
TFT_eSPI tft = TFT_eSPI();
uint16_t touchX, touchY;
bool isTouch, isUntouch = true;
menuState menuSelect,menuSelectAfter;
displayFunction disp;
//-----------All Data and Variable for Qr Scan---------------//
struct quirc *q = NULL;
struct quirc_code code;
struct quirc_data data;
quirc_decode_error_t err;
camera_fb_t* fb = NULL;
//-----------All Data and Variable for Firebase-------------//
DefaultNetwork network;
FirebaseApp app; 
WiFiClientSecure ssl_client;
RealtimeDatabase Database;
AsyncClientClass aClient(ssl_client, getNetwork(network));
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
unsigned long ms;
//-----------All Data and Variable to Send---------------//
int pickedLockerNumber;
String whatsappNumberSend;
String qrCodeKey;
//-----------All Data to get for Locker Availability-------------//
int statusArr[9];

void setup() {
  Serial.begin(115200);
  displayBegin();
  disp.startingUpScreen(tft);
  turnOnConnection();
  establishFirebaseRequest();
  WiFi.disconnect();
  cameraBegin();
  quircBegin();
}

void loop() {
  menuSwitchCase();
}

void menuSwitchCase(){
  switch(menuSelect){
    case menuState::MAIN_MENU:
      if(printOnce()){
        disp.menuInterfacePrint(tft);
      }
      if(debounceButton()){
        menuSelect = disp.handleMenuButtonPress(tft, touchX, touchY);
      }
      break;
    //------------------Pinjam Menu-------------------//
    case menuState::GET_LOCKER_AVAILABILITY:
      disp.getStatusValue(tft);
      turnOnConnection();
      for(int getIndex = 0; getIndex < 9; getIndex++){
        statusArr[getIndex] = getFromFirebase(aClient, Database, String("/Locker List/Locker ") + String(getIndex + 1) + String("/Status"));
      }
      WiFi.disconnect();
      menuSelect = menuState::PICK_LOCKER_MENU;// change this later
      break;
    case menuState::PICK_LOCKER_MENU:
      if(printOnce()){
        pickedLockerNumber = 0;
        disp.lockerInterfacePrint(tft, statusArr);
        disp.backInterfacePrint(tft, pickedLockerNumber);
      }
      if(debounceButton()){
        pickedLockerNumber = disp.handleLockerButtonPress(tft, touchX, touchY, statusArr);
        menuSelect = disp.handleBackButtonPressLocker(tft, touchX, touchY);
      } 
      if (pickedLockerNumber != 0){
        menuSelect = menuState::INPUT_NUMBER_MENU;
      }
      break;
    case menuState::INPUT_NUMBER_MENU:
      if(printOnce()){
        whatsappNumberSend = "";
        disp.resetInputNumber();
        disp.inputInterfacePrint(tft);
        disp.backInterfacePrint(tft, pickedLockerNumber);
      }
      if(debounceButton()){
        whatsappNumberSend = disp.handleInputButtonPress(tft, touchX, touchY);
        menuSelect = disp.handleBackButtonPressInput(tft, touchX, touchY);
      }
      if(whatsappNumberSend != ""){
        menuSelect = menuState::FIREBASE_SEND_MENU;// change this later
      }
      break;
    case menuState::FIREBASE_SEND_MENU:
      disp.sendBorrowRequest(tft);
      turnOnConnection();
      sendToFirebase(aClient, Database, String("/Locker List/Locker ") + String(pickedLockerNumber) + String("/WA Number"), whatsappNumberSend);
      intToFirebase(aClient, Database, String("/Locker List/Locker ") + String(pickedLockerNumber) + String("/Status"), 0);
      WiFi.disconnect();
      menuSelect = menuState::WAIT_MENU;// change this later
      break;
    //---------------------Ambil Menu--------------------------//
    case menuState::GET_BORROWED_LOCKER:
      disp.getStatusValue(tft);
      turnOnConnection();
      for(int getIndex = 0; getIndex < 9; getIndex++){
        statusArr[getIndex] = getFromFirebase(aClient, Database, String("/Locker List/Locker ") + String(getIndex + 1) + String("/Status"));
        statusArr[getIndex] = statusArr[getIndex] ^ 1;
      }
      WiFi.disconnect();
      menuSelect = menuState::LOCKER_TO_SCAN_MENU;// change this later      
      break;
    case menuState::LOCKER_TO_SCAN_MENU:
      if(printOnce()){
        pickedLockerNumber = 0;
        disp.lockerInterfacePrint(tft, statusArr);
        disp.backInterfacePrint(tft, pickedLockerNumber);
      }
      if(debounceButton()){
        pickedLockerNumber = disp.handleLockerButtonPress(tft, touchX, touchY, statusArr);
        menuSelect = disp.handleBackButtonPressLockerScan(tft, touchX, touchY); // cahnge this
      } 
      if (pickedLockerNumber != 0){
        menuSelect = menuState::SCAN_QR_MENU;
      }
      break;
    case menuState::SCAN_QR_MENU:
      if(printOnce()){
        qrCodeKey = "";
        tft.fillScreen(TFT_GRAY);
        disp.backInterfacePrint(tft, pickedLockerNumber);
      }
      fb = esp_camera_fb_get();
      handleQrScan(fb, q);
      qrCodeKey = decodeBufferImage(q, err, &code, &data);
      disp.batchPrintImage(fb, tft);
      esp_camera_fb_return(fb);
      if(debounceButton()){
        menuSelect = disp.handleBackButtonPressScan(tft, touchX, touchY);
      }
      if(qrCodeKey != "")
      {
        menuSelect = menuState::FIREBASE_QR_MENU;// change this maybe
      }
      break;
    case menuState::FIREBASE_QR_MENU:
      disp.sendQrTranslation(tft);
      turnOnConnection();
      sendToFirebase(aClient, Database, String("/Locker List/Locker ") + String(pickedLockerNumber) + String("/Qr Key"), qrCodeKey);
      WiFi.disconnect();
      menuSelect = menuState::WAIT_MENU;// change this later    
      break;
    default:
      menuSelect = menuState::MAIN_MENU;
      menuSelectAfter = menuState::WAIT_MENU;
  }
}
//------------Debounce button so button only pressed once----------------------//
bool debounceButton(){
  isTouch = tft.getTouch(&touchX, &touchY);
  //touchX = tft.width() - touchX;
  touchY = tft.height() - touchY;
  if(isTouch && isUntouch){
    isUntouch = false;
    return true;
  } else if(!isTouch && !isUntouch){
    isUntouch = true;
  }
  return false;
}
//-----------print anything only once when chaning menu----------------------//
bool printOnce(){
  if(menuSelect != menuSelectAfter){
    menuSelectAfter = menuSelect;
    return true;
  }
  return false;
}
//-----------Display begin------------------//
void displayBegin(){  
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_GRAY);
  tft.setSwapBytes(true);
  tft.setFreeFont(&FreeSansBold12pt7b);
}
//-----------Camera Setup config------------------//
void cameraBegin(){
  camera_config_t config;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE; // for streaming
  config.frame_size = FRAMESIZE_QVGA; 
  config.jpeg_quality = 10;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) 
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}
//-----------Begin Qr Code Scan Pointer------------//
void quircBegin(){
  q = quirc_new();
  if (q == NULL) 
  {
    Serial.println("Failed to initialize quirc");
    ESP.restart();
  }
}
//----------Turn on Wifi------------//
void turnOnConnection(){
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
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