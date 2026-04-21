#include "qr_scanner.h"

//------------Combined function to handle Qr Scan-----------------//
bool handleQrScan(camera_fb_t *fb, struct quirc *q){
  if(!cameraBufferReady(fb)){
    return false;
  }

  if(!resizeBufferImage(fb, q)){
    return false;
  }

  copyBufferImage(fb ,q);
  return true;
}
//-----------check camera if camera buffer ready----------------//
bool cameraBufferReady(camera_fb_t *fb){
  if(!fb){
    Serial.println("Camera Capture Failed");
    return false;
  } else {
    //Serial.println("Camera Capture Success");
    return true;
  }
}
//------------reszie the buffer-------------//
bool resizeBufferImage(camera_fb_t *fb, struct quirc *q){
  if (quirc_resize(q, fb->width, fb->height) < 0){
    Serial.println("Failed to resize quirc buffer");
    return false;
  } else {
    //Serial.println("Succed to resize quirc buffer");
    return true;
  }
}
//------------coppy the buffer-------------//
void copyBufferImage(camera_fb_t *fb, struct quirc *q){
  uint8_t *image = quirc_begin(q, NULL, NULL);
  memcpy(image, fb->buf, fb->len);
  quirc_end(q);
  //Serial.println("Image Copied");
}
//------------decode the buffer-------------//
String decodeBufferImage(struct quirc *q, quirc_decode_error_t err, struct quirc_code *code, struct quirc_data *data){
  int qrGetCount = quirc_count(q);
  if (qrGetCount > 0) {
    quirc_extract(q, 0, code);
    err = quirc_decode(code, data);
    
    if (err) {
      Serial.println("Decoding failed");
      return "";
    } else {
      Serial.println("Decoding successful:");
      //dumpData(data);
      return dumpData(data);
    }
  }
}
//-------------data to hold payload---------//
String dumpData(const struct quirc_data *data) {
  Serial.printf("Version: %d\n", data->version);
  Serial.printf("ECC level: %c\n", "MLHQ"[data->ecc_level]);
  Serial.printf("Mask: %d\n", data->mask);
  Serial.printf("Length: %d\n", data->payload_len);
  Serial.printf("Payload: %s\n", data->payload);
  String qrCodePayloadString = (const char *)data->payload;
  return  qrCodePayloadString;
}