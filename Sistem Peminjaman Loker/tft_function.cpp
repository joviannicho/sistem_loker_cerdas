#include "tft_function.h"

displayFunction::displayFunction(){
  inputIndex = 0;
  inputGap = 0;
}

//--------------------------inputing number function display------------------------------//
void displayFunction::inputInterfacePrint(TFT_eSPI &tft){
  tft.fillScreen(TFT_GRAY);
  tft.setTextColor(TFT_WHITE, TFT_GRAY);
  tft.drawString("Input Whatsapp Numbers", 100, 20);

  // print number button 0-9
  for(int inputPrintIndex = 0; inputPrintIndex < 6; inputPrintIndex++){
    int inputPrintGap = 10 + (80 * inputPrintIndex);
    int inputPrintNumGap = 25 + (80 * inputPrintIndex);
    char inputNumPrint = ('0' + inputPrintIndex);

    tft.fillRoundRect(inputPrintGap, 165, 60, 70, 10, TFT_WHITE);
    tft.drawChar(inputPrintNumGap, 215, inputNumPrint, TFT_BLACK, TFT_BLACK, 2);

    tft.fillRoundRect(inputPrintGap, 245, 60, 70, 10, TFT_WHITE);
    tft.drawChar(inputPrintNumGap, 295, (inputNumPrint + 6), TFT_BLACK, TFT_BLACK, 2);
  }

  // print del button with X
  tft.fillRoundRect(330, 245, 140, 70, 10, TFT_RED);
  tft.drawChar(385, 295, 'X', TFT_BLACK, TFT_BLACK, 2);
  
  // print input bar with country code 62
  tft.fillRoundRect(10, 85, 400, 70, 10, TFT_WHITE);
  //tft.fillRoundRect(10, 85, 65, 70, 10, TFT_LIGHTGRAY);
  //tft.drawChar(15, 135, '6', TFT_BLACK, TFT_BLACK, 2);
  //tft.drawChar(45, 135, '2', TFT_BLACK, TFT_BLACK, 2);

  // print send number button
  tft.fillRoundRect(410, 85, 60, 70, 10, TFT_GREEN);
  tft.fillRect(420, 116, 20, 8, TFT_WHITE);
  tft.fillTriangle(460, 120, 440, 130, 440, 110, TFT_WHITE);

  // print picked locker place
  // tft.fillRoundRect(410, 5, 60, 70, 10, TFT_WHITE);
  // tft.drawChar(425, 55, ('0' + pickedLockerNumber), TFT_BLACK, TFT_BLACK, 2);
}
String displayFunction::handleInputButtonPress(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY){
  inputGap = 15 + (30 * inputIndex);
  tft.fillRect(150, 50, 200, 35, TFT_GRAY);
  for(int pressIndex = 0; pressIndex < 6; pressIndex++){
    int buttonLeftX = 10 + (80 * pressIndex);
    int buttonRightX = buttonLeftX + 60;

    // touch function print number in display
    if(touchX >= buttonLeftX && touchX <= buttonRightX){
      if (inputIndex < 13){
        if(touchY >= 165 && touchY <= 235){  
          tft.drawChar(inputGap, 135, ('0' + pressIndex), TFT_BLACK, TFT_BLACK, 2);
          holdInputedNumber[inputIndex] = pressIndex;
          Serial.println(holdInputedNumber[inputIndex]); ///remove this later
          inputIndex++;
          Serial.println(inputIndex);///remove this later
        }
        if(touchY >= 245 && touchY <= 315 && touchX < 330){
          tft.drawChar(inputGap, 135, ('0' + pressIndex + 6), TFT_BLACK, TFT_BLACK, 2);
          holdInputedNumber[inputIndex] = pressIndex + 6;
          Serial.println(holdInputedNumber[inputIndex]); ///remove this later
          inputIndex++;
          Serial.println(inputIndex);///remove this later
        }
      }
    }
  }

  // touch function del number in display
  if(touchX > 330 && touchY >= 245 && touchY <= 315 && inputIndex > 0){
    Serial.println("del");///remove this later
    tft.fillRect(inputGap - 30, 85, 27, 70, TFT_WHITE);
    inputIndex--;
    holdInputedNumber[inputIndex] = 0;
  }

  // touch function to send number (return the string)
  if(touchY >= 85 && touchY <= 155 && touchX >= 410){
    String resultToSend = "";
    if(inputIndex >= 10){
      for(int sendOrder = 1; sendOrder < inputIndex; sendOrder++){
        resultToSend += String(holdInputedNumber[sendOrder]);
      }
      Serial.println(resultToSend);
      Serial.println("sent"); ///remove this later
      return resultToSend;
    }else{
      tft.setTextColor(TFT_BLACK, TFT_RED);
      tft.drawString("Invalid Numbers", 150, 50);
    }
  }
  return "";
}
void displayFunction::resetInputNumber(){
  inputIndex = 0;
  inputGap = 0;
  Serial.print("inputIndex = ");
  Serial.println(inputIndex);
  Serial.print("inputGap = ");
  Serial.println(inputGap);
}

//------------------------menu function and display-------------------------------------//
void displayFunction::menuInterfacePrint(TFT_eSPI &tft){
  tft.fillScreen(TFT_GRAY);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  
  tft.setTextDatum(4);
  tft.setTextSize(2);  
  
  tft.fillRoundRect(60, 10, 360, 140, 10, TFT_BLACK);
  tft.fillRoundRect(70, 20, 340, 120, 10, TFT_WHITE);
  tft.drawString("PINJAM", 240, 80);

  tft.fillRoundRect(60, 170, 360, 140, 10, TFT_BLACK);
  tft.fillRoundRect(70, 180, 340, 120, 10, TFT_WHITE);
  tft.drawString("BUKA", 240, 240);
}
menuState displayFunction::handleMenuButtonPress(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY){
  if(touchX >= 60 && touchX <= 420){
    if(touchY < 160){
      Serial.println("Pinjam");
      return menuState::GET_LOCKER_AVAILABILITY;
    }
    if(touchY > 160){
      Serial.println("Buka");
      return menuState::GET_BORROWED_LOCKER;
    }
  }
  return menuState::MAIN_MENU;
}

//-----------------------locker pick function and display-------------------------------//
void displayFunction::lockerInterfacePrint(TFT_eSPI &tft, int statusArr[]){
  tft.fillScreen(TFT_GRAY);
  tft.setTextColor(TFT_WHITE, TFT_GRAY);
  tft.setTextDatum(0);
  tft.setTextSize(1);
  tft.drawString("Pilih Loker", 180, 20);

  for(int lockerPrintIndex = 0; lockerPrintIndex < 3; lockerPrintIndex++){
    int lockerPrintGap = 10 + (160 * lockerPrintIndex);
    int lockerNumPrintGap = 60 + lockerPrintGap;
    char lockerNumPrint = '0' + lockerPrintIndex + 1;

    if(statusArr[lockerPrintIndex]){
      tft.fillRoundRect(lockerPrintGap, 85, 140, 70, 10, TFT_WHITE);
      tft.drawChar(lockerNumPrintGap, 135, lockerNumPrint, TFT_BLACK, TFT_BLACK, 2);
    }
    
    if(statusArr[lockerPrintIndex + 3]){
      tft.fillRoundRect(lockerPrintGap, 165, 140, 70, 10, TFT_WHITE);
      tft.drawChar(lockerNumPrintGap, 215, (lockerNumPrint + 3), TFT_BLACK, TFT_BLACK, 2);
    }
    
    if(statusArr[lockerPrintIndex + 6]){
      tft.fillRoundRect(lockerPrintGap, 245, 140, 70, 10, TFT_WHITE);
      tft.drawChar(lockerNumPrintGap, 295, (lockerNumPrint + 6), TFT_BLACK, TFT_BLACK, 2);
    }
  }
}
int displayFunction::handleLockerButtonPress(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY, int statusArr[]){
  // touch button to select locker number
  for(int lockerPressIndex = 0; lockerPressIndex < 3; lockerPressIndex++){
    int lockerButtonLeftX = 10 + (160 * lockerPressIndex);
    int lockerButtonRightX = lockerButtonLeftX + 140;
    int pickedLockerNumber = lockerPressIndex + 1;

    if(touchX >= lockerButtonLeftX && touchX <= lockerButtonRightX){
      if(touchY >= 85 && touchY <= 155 && statusArr[lockerPressIndex] == 1){
        Serial.println(pickedLockerNumber);
        return pickedLockerNumber;
      }

      if(touchY >= 165 && touchY <= 235 && statusArr[lockerPressIndex + 3] == 1){
        Serial.println(pickedLockerNumber + 3);
        return pickedLockerNumber + 3;
      }

      if(touchY >= 245 && touchY <= 315 && statusArr[lockerPressIndex + 6] == 1){
        Serial.println(pickedLockerNumber + 6);
        return pickedLockerNumber + 6;
      }
    }
  }
  return 0;
}

//---------------------back button function and display-------------------------//
void displayFunction::backInterfacePrint(TFT_eSPI &tft, int pickedLockerNumber){
  tft.fillRoundRect(9, 4, 62, 72, 10, TFT_WHITE );
  tft.fillRoundRect(10, 5, 60, 70, 10, TFT_BLACK );
  tft.fillRect(40, 36, 20, 8, TFT_WHITE);
  tft.fillTriangle(20, 40, 40, 30, 40, 50, TFT_WHITE);

  tft.fillRoundRect(410, 5, 60, 70, 10, TFT_WHITE);
  tft.drawChar(425, 55, ('0' + pickedLockerNumber), TFT_BLACK, TFT_BLACK, 2);
}
menuState displayFunction::handleBackButtonPressLocker(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY){
  if(touchY >= 5 && touchY <= 75 && touchX <= 90){
    Serial.println("back"); //change this
    return menuState::MAIN_MENU; //change this later
  }
  return menuState::PICK_LOCKER_MENU;
}
menuState displayFunction::handleBackButtonPressLockerScan(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY){
  if(touchY >= 5 && touchY <= 75 && touchX <= 90){
    Serial.println("back"); //change this
    return menuState::MAIN_MENU; //change this later
  }
  return menuState::LOCKER_TO_SCAN_MENU;
}
menuState displayFunction::handleBackButtonPressInput(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY){
  if(touchY >= 5 && touchY <= 75 && touchX <= 90){
    Serial.println("back"); //change this
    return menuState::PICK_LOCKER_MENU; //change this later
  }
  return menuState::INPUT_NUMBER_MENU;
}
menuState displayFunction::handleBackButtonPressScan(TFT_eSPI &tft, uint16_t touchX, uint16_t touchY){
  if(touchY >= 5 && touchY <= 75 && touchX <= 90){
    Serial.println("back"); //change this
    return menuState::LOCKER_TO_SCAN_MENU; //change this later
  }
  return menuState::SCAN_QR_MENU;
}

//--------------------Loading Screen------------------------------------------//
void displayFunction::startingUpScreen(TFT_eSPI &tft){
  tft.fillScreen(TFT_GRAY);
  tft.setTextColor(TFT_BLACK, TFT_BLACK);  
  tft.setTextDatum(4);
  tft.setTextSize(2);
  tft.drawString("STARTING UP", 240, 160); 
}
void displayFunction::getStatusValue(TFT_eSPI &tft){
  tft.fillScreen(TFT_GRAY);
  tft.setTextColor(TFT_BLACK, TFT_BLACK);  
  tft.setTextDatum(4);
  tft.setTextSize(2);
  tft.drawString("FETCHING DATA", 240, 160); 
}
void displayFunction::sendBorrowRequest(TFT_eSPI &tft){
  tft.fillScreen(TFT_GRAY);
  tft.setTextColor(TFT_BLACK, TFT_BLACK);  
  tft.setTextDatum(4);
  tft.setTextSize(2);
  tft.drawString("SENDING DATA", 240, 160); 
}
void displayFunction::sendQrTranslation(TFT_eSPI &tft){
  tft.fillScreen(TFT_GRAY);
  tft.setTextColor(TFT_BLACK, TFT_BLACK);  
  tft.setTextDatum(4);
  tft.setTextSize(2);
  tft.drawString("SENDING QR KEY", 240, 160); 
}
//----------------------Print Camera Image to display-------------------------//
void displayFunction::batchPrintImage(camera_fb_t *fb, TFT_eSPI& tft) {
  uint16_t *rgbBuffer = new uint16_t[fb->width * fb->height]; 
  int centX = (tft.width() - fb->width) / 2;
  int centY = (tft.height() - fb->height) / 2;  
  for (int col = 0; col < fb->height; col++) 
  {
    for (int row = 0; row < fb->width; row++) 
    {
      int Orig = col * fb->width + row;
      int Mirror = col * fb->width + (fb->width - 1 - row); 

      uint8_t grayscale = fb->buf[Orig];
      // Convert grayscale to RGB565
      uint16_t rgb565 = ((grayscale >> 3) << 11) | ((grayscale >> 2) << 5) | (grayscale >> 3);
      rgbBuffer[Mirror] = rgb565;
    }
  }
  tft.pushImage(centX, centY, fb->width, fb->height, rgbBuffer);
  delete[] rgbBuffer;
}
