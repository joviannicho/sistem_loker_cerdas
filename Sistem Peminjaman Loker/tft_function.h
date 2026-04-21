// typedef struct rectangleButtonDraw {
//     int x, y; 
//     int w, h;
//     char buttonName;
//     uint32_t rectColor;
// }rectangleButtonDraw;

// typedef struct triangleDraw{
//   int x, y, x1, y1, x2, y2;
//   uint32_t triColor;
// }triangleDraw;

#ifndef TFT_FUNCTION_H  // Unique identifier for this header file
#define TFT_FUNCTION_H

// Include headers
#include "camera_set_up.h"
#include <TFT_eSPI.h>
#define TFT_GRAY 0x5AEB
#define TFT_LIGHTGRAY 0xD69A

enum class menuState {
  WAIT_MENU,
  MAIN_MENU,

  GET_LOCKER_AVAILABILITY,
  PICK_LOCKER_MENU,
  INPUT_NUMBER_MENU,
  FIREBASE_SEND_MENU,

  GET_BORROWED_LOCKER,
  LOCKER_TO_SCAN_MENU,
  SCAN_QR_MENU,
  FIREBASE_QR_MENU
};

class displayFunction{
  public:
    displayFunction();
    //-------------------Inputing number menu UI and Touchscreen--------------------------//
    void inputInterfacePrint(TFT_eSPI& tft);
    String handleInputButtonPress(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY);
    void resetInputNumber();

    //-------------------Main menu UI and Touchscreen--------------------------//
    void menuInterfacePrint(TFT_eSPI& tft);
    menuState handleMenuButtonPress(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY);

    //-------------------IPicking Locker menu UI and Touchscreen--------------------------//
    void lockerInterfacePrint(TFT_eSPI &tft,int statusArr[]);
    int handleLockerButtonPress(TFT_eSPI &tft, uint16_t touchX, uint16_t touch, int statusArr[]);

    //-------------------Back Button UI and Touchscreen--------------------------//
    void backInterfacePrint(TFT_eSPI &tft, int pickedLockerNumber);
    menuState handleBackButtonPressLocker(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY);
    menuState handleBackButtonPressLockerScan(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY);
    menuState handleBackButtonPressInput(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY);
    menuState handleBackButtonPressScan(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY);

    //-------------------Loading Screen--------------------------//
    void startingUpScreen(TFT_eSPI &tft);
    void getStatusValue(TFT_eSPI &tft);
    void sendBorrowRequest(TFT_eSPI &tft);
    void sendQrTranslation(TFT_eSPI &tft);
    
    //--------------------Print camera Image to Screen----------------------//
    void batchPrintImage(camera_fb_t *fb, TFT_eSPI& tft);
  private:
    int inputIndex;
    int inputGap;
    int holdInputedNumber[13];
};
#endif // TFT_FUNCTIONS_H
