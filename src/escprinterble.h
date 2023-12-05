#ifndef escprinterble_h
#define escprinterble_h

#include "Arduino.h"
#include "BLEDevice.h"

static boolean connected = false;

enum BarcodeType
{
  BARCODE_CODE93 = 72,
  BARCODE_CODE128 = 73,
};

enum BarcodeText
{
  BARCODE_TEXT_NONE = 48,
  BARCODE_TEXT_ABOVE = 49,
  BARCODE_TEXT_BELOW = 50,
  BARCODE_TEXT_BOTH = 51
};

enum Align
{
  ALIGN_LEFT = 48,
  ALIGN_CENTER = 49,
  ALIGN_RIGHT = 50
};

enum Justify
{
  JUSTIFY_LEFT = 0,
  JUSTIFY_CENTER = 1,
  JUSTIFY_RIGHT = 2
};

class EscPos : public Print
{
public:
  EscPos();
  EscPos(char *service, char *characteristic);

  void effectBold();
  void effectDoubleHeight();
  void effectDoubleWidth();
  void effectUnderline();
  void effectOff();

  size_t write(uint8_t c);
  size_t cwrite(char c);
  int read();

  void align(uint8_t n);

  // space between 2 text
  void around(String leftText, String rightText);
  void around(String leftText, String rightText, int width);

  // barcode
  void barcode(char *code);
  void barcode(int typeBarcode, int height, char *code, int textPosition);

  // QR
  void codeQR(String code);
  void codeQR(String code, int size);

  void feed(uint8_t rows);
  void feed();

  // image
  void printImage(uint8_t *buffer, int width, int height);
  void writeData(uint8_t *data, int length);

  // BLE
  void start();
  void connectLoop();
  bool connect();
  void disconnect();

  // not tested by me yet
  int getStatus();
  void characterSet(uint8_t n);
  void lineSpacing(uint8_t n);
  void defaultLineSpacing();
  void set80mm();
  void set58mm();
  void reverseOff();
  void reverseOn();
  void partialCut();
  void fullCut();
  void justify(uint8_t n);
};

#endif
