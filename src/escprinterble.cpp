#include "Arduino.h"
#include "escprinterble.h"
#include "BLEDevice.h"

static const char LF = 0x0A;
static const char GS = 0x1D;
static const char DLE = 0x10;
static const char EOT = 0x04;
static const char ESC = 0x1B;
static bool withResponse = false;

// The remote service we wish to connect to.
// static BLEUUID serviceUUID("000018F0-0000-1000-8000-00805F9B34FB");
// The characteristic of the remote service we are interested in.
// static BLEUUID charUUID("00002AF1-0000-1000-8000-00805F9B34FB");
static BLEUUID serviceUUID;
static BLEUUID charUUID;
static boolean doConnect = false;
static boolean doScan = false;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEAdvertisedDevice *myDevice;
static BLEClient *pClient;

static void notifyCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
  Serial.print("data: ");
  Serial.write(pData, length);
  Serial.println();
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
    Serial.println("BLEClient onConnect");
  }

  void onDisconnect(BLEClient *pclient)
  {
    connected = false;
    Serial.println("onDisconnect");
    pclient->disconnect();
  }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  }   // onResult
};

EscPos::EscPos()
{
  serviceUUID = BLEUUID("e7810a71-73ae-499d-8c15-faa9aef0c3f2");
  charUUID = BLEUUID("bef8d6c9-9c21-4c9e-b632-bd58c1009f9f");
}

EscPos::EscPos(char *service, char *characterictic)
{
  serviceUUID = BLEUUID(service);
  charUUID = BLEUUID(characterictic);
}

void EscPos::start()
{

  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("ESP32");

  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  delay(500);
}

void EscPos::around(String leftText, String rightText)
{
  int width = 32;
  around(leftText, rightText, width);
}

void EscPos::around(String leftText, String rightText, int width)
{
  int leftSpace = leftText.length();
  int rightSpace = rightText.length();
  int space = (width - leftSpace) - rightSpace;

  print(leftText);
  for (int y = 0; y < space; y++)
  {
    print(" ");
  }
  println(rightText);
}

int EscPos::read()
{
  /* int result;
  result = pRemoteCharacteristic->readUInt8();
  return result; */
}
size_t EscPos::write(uint8_t c)
{
  if (!connected)
  {
    return 0;
  }

  pRemoteCharacteristic->writeValue(c);
  delay(10);
  return 1;
}
size_t EscPos::cwrite(char c)
{
  if (!connected)
  {
    return 0;
  }

  pRemoteCharacteristic->writeValue(c);
  delay(10);
  return 1;
}

void EscPos::writeData(uint8_t *data, int len)
{
  if (!connected)
  {
    return;
  }

  while (len > 20)
  {
    pRemoteCharacteristic->writeValue(data, 20, withResponse);
    if (!withResponse)
      delay(4);
    data += 20;
    len -= 20;
  }
  if (len)
  {
    pRemoteCharacteristic->writeValue(data, len, withResponse);
  }
}

// query error of printer. return 0 if ok, 4 for cover open, 32 for paper end, 64 for other error.
int EscPos::getStatus()
{
  this->write(DLE);
  this->write(EOT);
  this->write(0x02);
  int result;
  // result = pRemoteCharacteristic->readUInt8() - 18;
  result = 32;

  return result;
}

// Print and feed n lines
void EscPos::feed(uint8_t n)
{
  this->write(ESC);
  this->write('d');
  this->write(n);
}
// Print one line
void EscPos::feed()
{
  this->feed(1);
}

// Print 80mm paper
void EscPos::set80mm()
{
  this->write(GS);
  this->write('W');
  this->write(64);
  this->write(2);
}
// Print 58mm paper
void EscPos::set58mm()
{
  this->write(GS);
  this->write('W');
  this->write(164);
  this->write(1);
}

// Set line spacing to n/180-inch
void EscPos::lineSpacing(uint8_t n = 60)
{
  this->write(ESC);
  this->write('3');
  this->write(n);
}
// Select default line spacing to 1/6 inch (n=60)
void EscPos::defaultLineSpacing()
{
  this->write(ESC);
  this->write('2');
}

// Select an international character set
//  0 = U.S.A.
//  1 = France
//  2 = Germany
//  3 = U.K.
//  4 = Denmark I
//  5 = Sweden
//  6 = Italy
//  7 = Spain
//  8 = Japan
//  9 = Norway
// 10 = Denmark II
// 11 = Spain II
// 12 = Latin America
// 13 = Korea
// 14 = Slovenia / Croatia
// 15 = China
void EscPos::characterSet(uint8_t n = 0)
{
  this->write(ESC);
  this->write('R');
  this->write(n);
}

void EscPos::effectDoubleHeight()
{
  this->write(ESC);
  this->write('!');
  this->write(16);
}
void EscPos::effectBold()
{
  this->write(ESC);
  this->write('!');
  this->write(8);
}
void EscPos::effectDoubleWidth()
{
  this->write(ESC);
  this->write('!');
  this->write(32);
}
void EscPos::effectUnderline()
{
  this->write(ESC);
  this->write('!');
  this->write(128);
}
void EscPos::effectOff()
{
  this->write(ESC);
  this->write('!');
  this->write(0);
}

void EscPos::reverseOn()
{
  this->write(GS);
  this->write('B');
  this->write(1);
}
void EscPos::reverseOff()
{
  this->write(GS);
  this->write('B');
  this->write(0);
}

void EscPos::justify(uint8_t n)
{
  this->write(ESC);
  this->write('a');
  this->write(n);
}

void EscPos::align(uint8_t n)
{
  this->write(ESC);
  this->write('a');
  this->write(n);
}

void EscPos::partialCut()
{
  this->write(GS);
  this->write('V');
  this->write(66);
  this->write(0xA); // print buffer and line feed
}
void EscPos::fullCut()
{
  this->write(GS);
  this->write('V');
  this->write(65);
  this->write(0xA); // print buffer and line feed
}

void EscPos::printImage(uint8_t *buffer, int width, int height)
{
  int pitch = (width + 7) >> 3;
  uint8_t *x;

  this->write(GS);
  this->write('v');
  this->write('0');
  this->write('0');
  this->write((width + 7) >> 3);
  this->write(0);
  this->write((uint8_t)height);
  this->write((uint8_t)(height >> 8));

  // Print the graphics
  x = buffer;
  for (int y = 0; y < height; y++)
  {
    writeData(x, pitch);
    delay(1);
    x += pitch;
  } // for y
}

void EscPos::barcode(char *code)
{
  barcode(BARCODE_CODE128, 64, code, BARCODE_TEXT_BELOW);
}

void EscPos::barcode(int iType, int iHeight, char *code, int iTextPos)
{
  uint8_t len;
  len = (uint8_t)strlen(code);

  // barcodeTextPosition
  // n range 0-3 - 0=no 1=above 2=below 3=both
  this->write(GS);
  this->write('H');
  this->write(iTextPos);

  // barcodeHeight
  this->write(GS);
  this->write('h');
  this->write(iHeight);

  // barcodeWidth
  this->write(GS);
  this->write('w');
  this->write(2);

  // barcodeType
  this->write(GS);
  this->write('k');
  this->write(iType);

  // barcodeEndPrint
  this->write(len);
  print(code);
  this->write(0x00);
}

void EscPos::codeQR(String code)
{
  codeQR(code, 3);
}

void EscPos::codeQR(String code, int size)
{
  int length = code.length();
  // start code print
  this->write(GS);
  this->write('(');
  this->write('k');
  this->write(4);
  this->write(0);
  this->write(49);
  this->write(65);
  this->write(50);
  this->write(0);
  this->write(GS);
  this->write('(');
  this->write('k');
  this->write(3);
  this->write(0);
  this->write(49);
  this->write(67);
  this->write(size);
  this->write(GS);
  this->write('(');
  this->write('k');
  this->write(3);
  this->write(0);
  this->write(49);
  this->write(69);
  this->write(48);
  this->write(GS);
  this->write('(');
  this->write('k');
  length = length + 3;
  uint8_t ll = length % 256;
  length = (uint8_t)(length / 256);
  uint8_t lh = length % 256;
  this->write(ll);
  this->write(lh);
  this->write(49);
  this->write(80);
  this->write(48);

  // print code
  print(code);

  // end code
  this->write(GS);
  this->write('(');
  this->write('k');
  this->write(3);
  this->write(0);
  this->write(49);
  this->write(81);
  this->write(48);
}

void EscPos::disconnect(void)
{
  Serial.println("disconnect ");
  Serial.println(connected);

  if (!connected)

    if (pClient != NULL)
    {
      pClient->disconnect();
      connected = false;
    }
}

bool EscPos::connect(void)
{

  if (myDevice == nullptr)
  {
    Serial.println("Failed to find our service UUID: ");
    return false;
  }

  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());

  pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
  Serial.println(" - Connected to server");
  pClient->setMTU(517); // set client to request maximum MTU from server (default is 23 otherwise)

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    pClient->disconnect();

    return false;
  }
  Serial.println(" - Found our characteristic");

  // Read the value of the characteristic.
  if (pRemoteCharacteristic->canRead())
  {
    std::string value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    Serial.println(value.c_str());
  }

  if (pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  connected = true;

  delay(100);

  return true;
}

void EscPos::connectLoop(void)
{
  if (doConnect == true)
  {
    if (connect())
    {
      Serial.println("We are now connected to the BLE Server.");
    }
    else
    {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected)
  {
  }
  else if (doScan)
  {
    BLEDevice::getScan()->start(0); // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    delay(100);
  }

  delay(500);
}
