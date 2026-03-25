/*******************************************
 * see header-file for further informations
 ********************************************/

#include "Arduino.h"
#include "thermalprinter.h"
static const char LF = 0xA; // print buffer and line feed  
static const char ESC = 0x1B;
static const char GS = 0x1D;
#include <cstdint>
#include <unordered_map>

// Fallback for unmapped chars
constexpr uint8_t CP437_FALLBACK = ' ';

struct Cp437Map {
  uint16_t unicode;
  uint8_t cp437;
};

static const Cp437Map CP437_TABLE[] = {

  {0x00A0, 0xFF}, // NBSP
  {0x00A1, 0xAD}, // ¡
  {0x00A2, 0x9B}, // ¢
  {0x00A3, 0x9C}, // £
  {0x00A5, 0x9D}, // ¥
  {0x00AA, 0xA6}, // ª
  {0x00AB, 0xAE}, // «
  {0x00AC, 0xAA}, // ¬
  {0x00B0, 0xF8}, // °
  {0x00B1, 0xF1}, // ±
  {0x00B2, 0xFD}, // ²
  {0x00B5, 0xE6}, // µ
  {0x00B7, 0xFA}, // ·
  {0x00BA, 0xA7}, // º
  {0x00BB, 0xAF}, // »
  {0x00BC, 0xAC}, // ¼
  {0x00BD, 0xAB}, // ½
  {0x00BF, 0xA8}, // ¿

  // Accents
  {0x00C4, 0x8E}, {0x00C5, 0x8F}, {0x00C6, 0x92},
  {0x00C7, 0x80}, {0x00C9, 0x90},
  {0x00D1, 0xA5}, {0x00D6, 0x99}, {0x00DC, 0x9A},
  {0x00DF, 0xE1},
  {0x00E0, 0x85}, {0x00E1, 0xA0}, {0x00E2, 0x83},
  {0x00E4, 0x84}, {0x00E5, 0x86},
  {0x00E6, 0x91},
  {0x00E7, 0x87},
  {0x00E8, 0x8A}, {0x00E9, 0x82}, {0x00EA, 0x88}, {0x00EB, 0x89},
  {0x00EC, 0x8D}, {0x00ED, 0xA1}, {0x00EE, 0x8C}, {0x00EF, 0x8B},
  {0x00F1, 0xA4},
  {0x00F2, 0x95}, {0x00F3, 0xA2}, {0x00F4, 0x93},
  {0x00F6, 0x94},
  {0x00F7, 0xF6},
  {0x00F9, 0x97}, {0x00FA, 0xA3}, {0x00FB, 0x96},
  {0x00FC, 0x81},
  {0x00FF, 0x98},

  // Currency / symbols
  {0x0192, 0x9F},
  {0x20A7, 0x9E},

  // Greek subset
  {0x0393, 0xE2}, {0x0398, 0xE9}, {0x03A3, 0xE4},
  {0x03A6, 0xE8}, {0x03A9, 0xEA},
  {0x03B1, 0xE0}, {0x03B4, 0xEB},
  {0x03C0, 0xE3}, {0x03C3, 0xE5},
  {0x03C4, 0xE7}, {0x03C6, 0xED},

  // Math
  {0x2219, 0xF9},
  {0x221A, 0xFB},
  {0x2248, 0xF7},
  {0x2261, 0xF0},
  {0x2264, 0xF3},
  {0x2265, 0xF2},

  // Box drawing
  {0x2500, 0xC4}, {0x2502, 0xB3},
  {0x250C, 0xDA}, {0x2510, 0xBF},
  {0x2514, 0xC0}, {0x2518, 0xD9},
  {0x251C, 0xC3}, {0x2524, 0xB4},
  {0x252C, 0xC2}, {0x2534, 0xC1},
  {0x253C, 0xC5},

  // Double box
  {0x2550, 0xCD}, {0x2551, 0xBA},
  {0x2554, 0xC9}, {0x2557, 0xBB},
  {0x255A, 0xC8}, {0x255D, 0xBC},

  // Blocks
  {0x2580, 0xDF},
  {0x2584, 0xDC},
  {0x2588, 0xDB},
  {0x258C, 0xDD},
  {0x2590, 0xDE},
  {0x2591, 0xB0},
  {0x2592, 0xB1},
  {0x2593, 0xB2},

  // Misc
  {0x25A0, 0xFE}
};

uint8_t unicode_to_cp437(uint16_t code)
{
    if (code <= 0x7F)
        return (uint8_t)code;

    int low = 0;
    int high = sizeof(CP437_TABLE)/sizeof(CP437_TABLE[0]) - 1;

    while (low <= high)
    {
        int mid = (low + high) / 2;

        if (CP437_TABLE[mid].unicode == code)
            return CP437_TABLE[mid].cp437;

        if (CP437_TABLE[mid].unicode < code)
            low = mid + 1;
        else
            high = mid - 1;
    }

    return '?';
}
uint32_t decode_utf8(const char *&s)
{
    uint8_t c = *s++;

    if (c < 0x80)
        return c;

    if ((c >> 5) == 0x6) {
        uint32_t cp = ((c & 0x1F) << 6);
        cp |= (*s++ & 0x3F);
        return cp;
    }

    if ((c >> 4) == 0xE) {
        uint32_t cp = ((c & 0x0F) << 12);
        cp |= ((*s++ & 0x3F) << 6);
        cp |= (*s++ & 0x3F);
        return cp;
    }

    return '?';
}


namespace esphome {
namespace thermalprinter {

  
static const char *TAG = "printer_component.component";
Epson::Epson()
{
  // Serial1 = HardwareSerial::HardwareSerial(0);
  Serial1.begin( 115200, SERIAL_8N1, 20, 21, true );
  // this->_rxPin = rxPin;
  // this->_txPin = txPin;
  this->start();
}

void Epson::dump_config(){
    // ESP_LOG_INFO(TAG, "JPGIndustries Printer component");
    // int result = getStatus();
    // char string_buffer[(sizeof(result)) + 1];
    // memcpy(string_buffer, &result, sizeof(result));
    // string_buffer[sizeof(result)] = 0; // Null termination.
    // ESP_LOGCONFIG(TAG, string_buffer);
}
void Epson::start(){

  // pinMode(this->_txPin, OUTPUT);
  // pinMode(this->_rxPin, INPUT);  
  // this->_printer = new SoftwareSerial (this->_rxPin, this->_txPin);
  // this->_printer->begin(9600);
}

// query status of printer. when online returns value 22.
int Epson::getStatus(){
  Epson::write(0x10);    
  Epson::write(0x04);  
  Epson::write(1);
  int result;
  return result;
}
int Epson::getPaperStatus(){
  Epson::write(0x10);
  Epson::write(0x04);
  Epson::write(4);

  unsigned long start = millis();
  while (!Serial1.available()) {
    if (millis() - start > 100) return -1; // timeout
  }

  return Serial1.read();
}
void Epson::beep(){
  Epson::write(0x10); // DLE
  Epson::write(0x14); // DC4
  Epson::write(3);    // fn=3
}

// int Epson::readSerial(){
//     int result;
//     result = Serial1.read();
//     return result;
// }

void Epson::letterSpacing(int spacing){
  Epson::write(ESC);  
  Epson::write(0x20);
  Epson::write(spacing);    
}
void Epson::printLogo(int logoNum){
  Epson::justifyCenter();
  int logoIndex = 48+logoNum;
  //29  40  76  6  0  48  85  kc1  kc2  x  y
  Epson::write(29);  
  Epson::write(40);
  Epson::write(76);  
  Epson::write(6);  
  Epson::write(0);  
  Epson::write(48);  
  Epson::write(69);  
  Epson::write(48);  
  Epson::write(logoIndex);
  Epson::write(1);
  Epson::write(1);    
  Epson::justifyLeft();
}
// Print and feed n lines
// prints the data in the print buffer and feeds n lines
void Epson::feed(uint8_t n){
  Epson::write(ESC);  
  Epson::write(0x64);
  Epson::write(n);    
}

// Print one line
void Epson::feed(){
  this->feed(1);    
}

// Set line spacing
// sets the line spacing to n/180-inch
void Epson::lineSpacing(uint8_t n){
  Epson::write(ESC);  
  Epson::write(0x33);
  Epson::write(n);  
}

// Select default line spacing
// sets the line spacing to 1/6 inch (n=60). This is equivalent to 30 dots.
void Epson::defaultLineSpacing(){
  Epson::write(ESC);  
  Epson::write(0x32);
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
// see reference for Details! 
void Epson::characterSet(uint8_t n){
  Epson::write(ESC);  
  Epson::write(0x52);
  Epson::write(n);  
}

// Helper: update a single bit in currentMode and send ESC ! n
void Epson::updateTextMode(GlyphType mask, bool enable) {
    if (enable) {
        currentTextMode |= mask;
    } else {
        currentTextMode &= ~mask;
    }
    Epson::write(ESC);
    Epson::write('!');
    Epson::write((uint8_t) currentTextMode);
}

// --- ESC/POS text mode functions using updateTextMode ---
// Bold / Emphasized
void Epson::boldOn()        { updateTextMode(GlyphType::Bold, true); }
void Epson::boldOff()       { updateTextMode(GlyphType::Bold, false); }
void Epson::emphasizedOn()  { updateTextMode(GlyphType::Bold, true); } // same as bold if printer merges
void Epson::emphasizedOff() { updateTextMode(GlyphType::Bold, false); }

// Double height / width
void Epson::doubleHeightOn()   { updateTextMode(GlyphType::DoubleHeight, true); } // bit 4
void Epson::doubleHeightOff()  { updateTextMode(GlyphType::DoubleHeight, false); }
void Epson::doubleWidthOn()    { updateTextMode(GlyphType::DoubleWidth, true); } // bit 5
void Epson::doubleWidthOff()   { updateTextMode(GlyphType::DoubleWidth, false); }
void Epson::doubleSizeOn()     { updateTextMode(GlyphType::DoubleHeight | GlyphType::DoubleWidth, true); } // bits 4+5
void Epson::doubleSizeOff()    { updateTextMode(GlyphType::DoubleHeight | GlyphType::DoubleWidth, false); }

// Italic (if supported)
void Epson::italicOn()        { updateTextMode(GlyphType::Italic, true); }  // bit 6
void Epson::italicOff()       { updateTextMode(GlyphType::Italic, false); }

// Small text (some printers use Font B bit)
void Epson::smallTextOn()     { updateTextMode(GlyphType::Small, true); }  // bit 0 = Font B
void Epson::smallTextOff()    { updateTextMode(GlyphType::Small, false); }
// Underline
// void Epson::underlineOn()   { updateTextMode(GlyphType::Underline, true); } // bit 7
// void Epson::underlineOff()  { updateTextMode(GlyphType::Underline, false); }

void Epson::underlineOn(){
  Epson::write(ESC);
  Epson::write(0x2D); // '-'
  Epson::write(2);    // 1 = thin, 2 = thick
}

void Epson::underlineOff(){
  Epson::write(ESC);
  Epson::write(0x2D);
  Epson::write(0);
}

void Epson::doubleStrikeOn(){
  Epson::write(ESC);
  Epson::write(0x47); // 'G'
  Epson::write(1);
}

void Epson::doubleStrikeOff(){
  Epson::write(ESC);
  Epson::write(0x47);
  Epson::write(0);
}

void Epson::upsideDownOn(){
  Epson::write(ESC);
  Epson::write(0x7B); // '{'
  Epson::write(1);
}

void Epson::upsideDownOff(){
  Epson::write(ESC);
  Epson::write(0x7B); // '{'
  Epson::write(0);
}

void Epson::codePage(uint8_t n) {
  Epson::write(ESC);   // ESC
  Epson::write('t');   // 't' command
  Epson::write(n);     // code page number
}

void Epson::fontA() {
  Epson::write(ESC);
  Epson::write('M');
  Epson::write(0);  // Font A
}

void Epson::fontB() {
  Epson::write(ESC);
  Epson::write('M');
  Epson::write(1);  // Font B
}

// Turn white/black reverse printing mode on/off
void Epson::reverseOn() {
  Epson::write(GS);  
  Epson::write(0x42);    
  Epson::write(1);
}
  
void Epson::reverseOff() {
  Epson::write(GS);  
  Epson::write(0x42);    
  Epson::write(0);
}

void Epson::justifyLeft() {
  Epson::write(ESC);  
  Epson::write(0x61);    
  Epson::write(0);
}

void Epson::justifyCenter() {
  Epson::write(ESC);  
  Epson::write(0x61);    
  Epson::write(1);
}

void Epson::justifyRight() {
  Epson::write(ESC);  
  Epson::write(0x61);    
  Epson::write(2);
}
// bitmapLines: array of strings, each string = 8 chars, 'X' = dot on, ' ' = dot off
void Epson::define8x8Glyph(uint8_t code, const char* bitmapLines[8]) {
    uint8_t bitmap[8]; // 8 vertical bytes

    for (int col = 0; col < 8; col++) {
        bitmap[col] = 0;
        for (int row = 0; row < 8; row++) {
            if (bitmapLines[row][col] == 'X') {
                bitmap[col] |= (1 << row); // set vertical bit
            }
        }
    }

    // Send ESC & command
    Epson::write(0x1B);  // ESC
    Epson::write('&');   // '&'
    Epson::write(code);  // character code
    Epson::write(0);     // m = vertical bytes per column minus 1 (8 pixels = 1 byte → 1-1=0)

    // send bitmap column-major
    for (int i = 0; i < 8; i++) {
        Epson::write(bitmap[i]);
    }
}
void Epson::defineGlyph(uint8_t code, const uint8_t* bitmap, uint8_t width, uint8_t height){
  uint8_t vertical_bytes = (height + 7) / 8;

  Epson::write(0x1B);  // ESC
  Epson::write('&');   // '&'
  Epson::write(code);  // character code
  Epson::write(vertical_bytes - 1); // m = vertical bytes per column - 1

  // Send column-major bitmap
  for (uint8_t x = 0; x < width; x++){
      for (uint8_t y = 0; y < vertical_bytes; y++){
          // Each byte = 8 vertical dots in this column
          Epson::write(bitmap[y*width + x]);
      }
  }
}
void Epson::printCustomGlyph(uint8_t code){
  // enable
  Epson::write(0x1B);
  Epson::write('%');
  Epson::write(1);

  Epson::write(code);

  // disable
  Epson::write(0x1B);
  Epson::write('%');
  Epson::write(0);
}

//n range 1-255
void Epson::barcodeHeight(uint8_t n) {
  Epson::write(GS);  
  Epson::write(0x68);    
  Epson::write(n);
}
//n range 2-6
void Epson::barcodeWidth(uint8_t n) {
  Epson::write(GS);  
  Epson::write(0x77);    
  Epson::write(n);
}
//n range 0-3
void Epson::barcodeNumberPosition(uint8_t n) {
  Epson::write(GS);  
  Epson::write(0x48);    
  Epson::write(n);
}
//m range 65-73 (code type)
//n (digit length)
void Epson::printBarcode(uint8_t m, uint8_t n) {
  Epson::write(GS);  
  Epson::write(0x6B);    
  Epson::write(m);
  Epson::write(n);
}

void Epson::printQRCode(const std::string &text, uint8_t size) {
  // Clamp module size 1–16
  if (size < 1) size = 1;
  if (size > 16) size = 16;

  uint16_t len = text.length();

  // ---- Select QR Model 2 ----
  Epson::writeBytes("\x1D\x28\x6B\x04\x00\x31\x41\x32\x00", 9);

  // ---- Set QR Module Size ----
  Epson::write(0x1D);
  Epson::write(0x28);
  Epson::write(0x6B);
  Epson::write(0x03);     // pL
  Epson::write(0x00);     // pH
  Epson::write(0x31);     // cn
  Epson::write(0x43);     // fn = module size
  Epson::write(size);

  // ---- Set Error Correction (M = 15%) ----
  //L: 0x30
  //M: 0x31
  //Q: 0x32
  Epson::writeBytes("\x1D\x28\x6B\x03\x00\x31\x45\x30", 8);

  // ---- Store Data in QR Buffer ----
  uint16_t store_len = len + 3;
  uint8_t pL = store_len & 0xFF;
  uint8_t pH = (store_len >> 8) & 0xFF;

  Epson::write(0x1D);
  Epson::write(0x28);
  Epson::write(0x6B);
  Epson::write(pL);
  Epson::write(pH);
  Epson::write(0x31);     // cn
  Epson::write(0x50);     // fn = store data
  Epson::write(0x30);     // m
  Epson::writeBytes(text.data(), len);

  // ---- Print QR Code ----
  Epson::writeBytes("\x1D\x28\x6B\x03\x00\x31\x51\x30", 8);
}

void Epson::cut() {
  Epson::write(GS);
  Epson::write('V');
  Epson::write(66);
  Epson::write(0xA); // print buffer and line feed
}
/// <summary>
/// Prints the image. The image must be 384px wide.
/// </summary>
/// <param name='image'>
/// Image to print.
/// </param>
int Epson::configureImagePage(const bool highDensity,const uint32_t width,const uint32_t height)
{
  imagePageMode = true;
  currentImageHeight = height;
  currentImageWidth = width;
  int configurationWidth = 0;
  int configurationHeight = 0;
  if (highDensity)
  {
    currentImageDensity = 33;
    configurationWidth = width*3;
    configurationHeight = height*2;
  }else{
    currentImageDensity = 0;
    configurationWidth = width;
    configurationHeight = height;
  }
  uint8_t dxL = configurationWidth & 255;
  uint8_t dxH = configurationWidth >> 8;
  uint8_t dyL = configurationHeight & 255;
  uint8_t dyH = configurationHeight >> 8;

  //trigger page mode.
  Epson::write(27);
  Epson::write(76);
  
  //begin set page size
  Epson::write(27);
  Epson::write(87);
  //start at 0,0
  Epson::write(0);
  Epson::write(0);
  Epson::write(0);
  Epson::write(0);

  //end at W, H (this is the split word paradigm again. Not sure about height configuration?)
  Epson::write(dxL);
  Epson::write(dxH);
  Epson::write(dyL);
  Epson::write(dyH);
  
  
  return configurationWidth;
  
}
int Epson::configureImage(const bool highDensity,const uint32_t width,const uint32_t height)
{
  imagePageMode = false;
  currentImageHeight = height;
  currentImageWidth = width;
  int configurationWidth = 0;
  int configurationHeight = 0;
  if (highDensity)
  {
    currentImageDensity = 33;
    configurationWidth = width*3;
    configurationHeight = height*3;
  }else{
    currentImageDensity = 0;
    configurationWidth = width;
    configurationHeight = height;
  }
  
  return configurationWidth;
  
}
void Epson::finishImage()
{
  Epson::write(12);
}
void Epson::printImageRaster(const char* line_buffer, const int width_bytes, const int num_rows, const bool highDensity) 
{
    uint8_t xL = width_bytes & 0xFF;
    uint8_t xH = width_bytes >> 8;

    uint16_t height = num_rows;
    uint8_t yL = height & 0xFF;
    uint8_t yH = height >> 8;

    // GS v 0
    Epson::write(29); // GS
    Epson::write(118); // v
    Epson::write(48); // 0

    Epson::write(highDensity ? 1 : 0); // mode
    Epson::write(xL);
    Epson::write(xH);
    Epson::write(yL);
    Epson::write(yH);

    // write all rows in one go
    Epson::writeBytes(line_buffer, width_bytes * num_rows);
}

void Epson::printImageLine(const char* line_buffer, const int line_length, const bool highDensity)
{
  //parameters in the ESCPOS lib, these are a uint16 split in 2 denoting the 
  //  width of the line. 
  //align the line with our printer size.
  int current_width = (line_length);
  currentImageWidth = current_width;
  if(highDensity)
  {
    current_width = (line_length - (line_length % 3));
    currentImageWidth = current_width / 3;
  }
  uint8_t nL = currentImageWidth & 255;
  uint8_t nH = currentImageWidth >> 8;

  //enable unidirectional printing
  Epson::write(27);
  Epson::write(85);
  Epson::write(255);

  //set line spacing ?
  Epson::write(27);
  Epson::write(51);
  // Epson::write(imagePageMode ? 24:0);
  Epson::write(24);
  
  //prepare for image
  Epson::write(27);//ESC
  Epson::write(42);//*
  Epson::write(highDensity ? 33 : 0);//changes the DPI (see manual)
  Epson::write(nL);//lower byte of the width
  Epson::write(nH);//upper byte of the width
  
  //write the data
  Epson::writeBytes(line_buffer, current_width);

  //newline to kick out the buffer
  // Epson::write(13);
  // Epson::write(10);
  Epson::write(27); // ESC
  Epson::write(74); // J
  Epson::write(20); // exact dot feed (doesn't work.)

  //reset the unidirectional printing
  Epson::write(27);
  Epson::write(85);
  Epson::write(0);

  //reset the line spacing?
  Epson::write(27);
  Epson::write(51);
  Epson::write(24);

  //newline
  if(imagePageMode)
  {
    Epson::write(13);
    Epson::write(10);
  }
}

void Epson::speed(int inSpeed)
{
    Epson::write(GS);
    Epson::write(40);
    Epson::write(75);
    Epson::write(2);
    Epson::write(0);
    Epson::write(50);
    // char stringbuff[3];
    // itoa(inSpeed, stringbuff,10)
    if(inSpeed < 1)
    {
      inSpeed = 1;
    }
    if (inSpeed > 10)
    {
      inSpeed = 10;
    }
    Epson::write((uint8_t) inSpeed);
}

// =========================
// Apply formatting (ESPHome switches)
// =========================
void Epson::apply_format(const std::string &token, bool enable)
{
  if (token == "bold")
  {
    if(enable){
        boldOn();
    }else{
        boldOff();
    }
  }
  else if (token == "italic")
  {
    if(enable){
        italicOn();
    }else{
        italicOff();
    }
  }
  else if (token == "underline")
  {
    if(enable){
        underlineOn();
    }else{
        underlineOff();
    }
  }
  else if (token == "doubleheight")
  {
    if(enable){
        doubleHeightOn();
    }else{
        doubleHeightOff();
    }
  }
  else if (token == "doublesize")
  {
    if(enable){
        doubleSizeOn();
    }else{
        doubleSizeOff();
    }
  }
  else if (token == "small")
  {
    if(enable){
        fontB();
    }else{
        fontA();
    }
  }
  else if (token == "emphasized")
  {
    if(enable){
        emphasizedOn();
    }else{
        emphasizedOff();
    }
  }
  else if (token == "whiteonblack")
  {
    if(enable){
        reverseOn();
    }else{
        reverseOff();
    }
  }
}

// =========================
// Execute a token command
// Supports {bold}, {/bold}, {center}, {feed:3}, {hr}, {reset}
// =========================
void Epson::execute_token(std::string token)
{
  if (token.empty()) return;

  bool disable = false;

  // check for closing token like {/bold}
  if (token[0] == '/')
  {
    disable = true;
    token = token.substr(1);
  }

  // Formatting switches
  if (token == "bold" || token == "italic" || token == "underline" ||
      token == "doubleheight" || token == "doublesize" ||
      token == "small" || token == "emphasized" ||
      token == "whiteonblack")
  {
    apply_format(token, !disable);
    return;
  }

  // Alignment
  if (token == "center") { justifyCenter(); return; }
  if (token == "right")  { justifyRight(); return; }
  if (token == "left")   { justifyLeft(); return; }

  // Feed lines, e.g., {feed:3}
  if (token.rfind("feed:", 0) == 0)
  {
    int n = atoi(token.substr(5).c_str());
      feed(n);
  }

  // Reset all formatting
  if (token == "reset")
  {
    apply_format("bold", false);
    apply_format("italic", false);
    apply_format("underline", false);
    apply_format("doubleheight", false);
    apply_format("doublesize", false);
    apply_format("small", false);
    apply_format("emphasized", false);
    apply_format("whiteonblack", false);
    justifyCenter();
  }
}

uint8_t Epson::getCurrentGlyphWidth() {
  return glyphWidth(currentTextMode);
}



void Epson::printText(const char *str)
{
    while (*str)
    {
        uint32_t cp = decode_utf8(str);
        uint8_t out = unicode_to_cp437(cp);
        write(out);
    }
}
size_t Epson::write(uint8_t c) {
  Serial1.write(c);
  return 1;
}

size_t Epson::writeBytes(const char* inData,int length)
{
  size_t writtenBytes = 0;
  for (int i=0;i<length;i++){
    writtenBytes += Epson::write((uint8_t) inData[i]);
  }
  return writtenBytes;
}
// void Epson::printString(const char* text)
// {
//     // Traverse the string 
//   for (int i = 0;i<10; i++) { 
//     // Print current character 
//       Epson::write(text[i]); 
//   } 
//   Epson::write(LF);
// }

void Epson::logWrapback(const char* text)
{
  // ESP_LOGCONFIG(TAG, "wrapback: %s",text);
}


//TCP SERVER CODE.
//@TODO MOVE THIS TO ANOTHER CUSTOM COMPONENT
//This code integrates direct control over TCP commands.
//
//
//
void Epson::startTCPServer()
{
  if(serverStarted)
  {
    stopTCPServer();
  }
  if(DEBUG_ENABLE)
    Epson::print("Start TCP Server\n");
  tcpServer = WiFiServer(8888);
  tcpServer.begin();
  if(DEBUG_ENABLE)
    Epson::print("TCP Server started\n");
  serverStarted = true;
}

bool Epson::isAvailable()
{
  return serverStarted;
}
bool Epson::connected()
{
  if(serverStarted)
  {
    if(!clientConnected)
    {
      tcpClient = tcpServer.accept();
    }
    if(tcpClient)
    {
      // tcpClient = &active_client;
      if (tcpClient.connected())
      {
        //this check effectively oneshots the debug statements
        if(!clientConnected)
        {
          if(DEBUG_ENABLE)
            Epson::print("TCP Client Connected\n");
          clientConnected = true;
        }
      }else{
        if(clientConnected)
        {
          if(DEBUG_ENABLE)
            Epson::print("TCP Client Disconnected\n");
          tcpClient.stop(); 
        }
        clientConnected = false;
      }
    }
  }
  return clientConnected;
}
bool Epson::hasData()
{
  if(tcpClient)
  {
    return (tcpClient.available());
  }else{
    return false;
  }
}
char Epson::read()
{
  return tcpClient.read();
}
int Epson::read(char * const line_buffer, int buf_size)
{
  int i=0;
  for(; i<buf_size;i++)
  {
    // if(Epson::hasData())
    // {
      line_buffer[i] = tcpClient.read();
    // }else{
    //   break;
    // }
  }
  return i;
}
void Epson::listenOnTCPServer()
{
  if (!serverStarted)
  {
    startTCPServer();
  }
  int i = 0;
  
  if(DEBUG_ENABLE)
    Epson::print("begin listen\n");

  while(i < 100)
  {
    // WiFiClient tcpClient = tcpServer.available();
    // Epson::print(tcpServer.available());
    // Epson::print("\n");
    // if (!tcpClient)
    // {
      // Epson::print(tcpClient.connected());
      // Epson::print("\nTCP Client didn't connect!\n");
    // }
    if (Epson::connected())
    {
      
      if(DEBUG_ENABLE)
        Epson::print("TCP Client Connected\n\n");
      this->tcpClient.connected();
      this->tcpClient.available();
      if(DEBUG_ENABLE)
        Epson::print("TCP Client is alive\n\n");

    }
    if (Epson::connected())
    {
      if(Epson::hasData())
      {
          // char c = this->tcpClient.read();
          Epson::print(Epson::read());
       }
      delay(100);
    }
    delay(100);
    i++;
  }
  
  if(DEBUG_ENABLE)
    Epson::print("End listen\n");
}
void Epson::disconnectTCPClient()
{
  if(tcpClient)
  {
    tcpClient.stop();
    // delete &tcpClient;
  }
  clientConnected = false;
}
void Epson::stopTCPServer()
{
  if(DEBUG_ENABLE)
    Epson::print("Stop TCP Server\n");
  tcpServer.stop();
  serverStarted = false;
}

// void Epson::initTCP(Epson printer)
// {
//   AsyncServer* server = new AsyncServer(8888); // start listening on tcp port 7050
// 	server->onClient((void*)&Epson::handleNewClient, server);
// 	server->begin();
// }
// /* server events */
// static void Epson::handleNewClient(void* arg, AsyncClient* client) {
// 	// Epson::printf("\n new client has been connected to server, ip: %s", client->remoteIP().toString().c_str());

// 	// add to list
// 	clients.push_back(client);
	
// 	// register events
// 	client->onData((void*)&Epson::handleData, NULL);
// 	client->onError((void*)&Epson::handleError, NULL);
// 	client->onDisconnect((void*)&Epson::handleDisconnect, NULL);
// 	client->onTimeout((void*)&Epson::handleTimeOut, NULL);
// }
// static void Epson::handleError(void* arg, AsyncClient* client, int8_t error) {
// 	// Epson::printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
// }

// static void Epson::handleData(void* arg, AsyncClient* client, void *data, size_t len) {
// 	// Epson::printf("\n data received from client %s \n", client->remoteIP().toString().c_str());
// 	// Epson::printf((uint8_t*)data);

// 	// reply to client
// 	if (client->space() > 32 && client->canSend()) {
// 		char reply[32];
// 		// sprintf(reply, "this is from %s", SERVER_HOST_NAME);
// 		client->add(reply, strlen(reply));
// 		client->send();
// 	}
// }

// static void Epson::handleDisconnect(void* arg, AsyncClient* client) {
// 	// Epson::printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
// }

// static void Epson::handleTimeOut(void* arg, AsyncClient* client, uint32_t time) {
// 	// Epson::printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
// }


}
}