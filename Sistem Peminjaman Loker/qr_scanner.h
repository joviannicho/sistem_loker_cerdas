#ifndef QR_SCANNER_H
#define QR_SCANNER_H

#include <Arduino.h>
#include "camera_set_up.h"
#include "quirc.h"

bool handleQrScan(camera_fb_t *fb, struct quirc *q);
bool cameraBufferReady(camera_fb_t *fb);
bool resizeBufferImage(camera_fb_t *fb, struct quirc *q);
void copyBufferImage(camera_fb_t *fb, struct quirc *q);
String decodeBufferImage(struct quirc *q, quirc_decode_error_t err, struct quirc_code *code, struct quirc_data *data);
String dumpData(const struct quirc_data *data);

#endif