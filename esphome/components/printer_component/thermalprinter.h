/*******************************************
 *
 * Name.......:  Epson TM-T88II Library
 * Version....:  0.6.0
 * Description:  A Library to control an Epson TM-T88II thermal printer (microprinter) 
 *               by an arduino via serial connection
 * Project....:  https://github.com/signalwerk/thermalprinter
 * License....:  You may use this work under the terms of either the MIT License or 
                 the GNU General Public License (GPL) Version 3
 * Keywords...:  thermal, micro, receipt, printer, serial, tm-t88, tm88, tmt88, epson
 * ********************************************/
// reference: 
//https://www.pos.com.au/Help-SP/PrinterCommandsforEscPOSNonGraph.html
// https://download4.epson.biz/sec_pubs/pos/reference_en/escpos/ref_escpos_en/receipt_with_barcode.html
#ifndef thermalprinter_h
#define thermalprinter_h

#include "Arduino.h"
#include <WiFi.h>
#include <vector>

#include "esphome/core/component.h"
#include <AsyncTCP.h>
#include <HardwareSerial.h>
#define DEBUG_ENABLE false
namespace esphome {
namespace thermalprinter {

  enum class GlyphType : uint8_t {
    Normal       = 0x00, // no formatting
    Small        = 0x01, // bit 0 = Font B
    Bold         = 0x08, // bit 3
    Emphasized   = 0x08, // same as Bold if printer merges
    DoubleHeight = 0x10, // bit 4
    DoubleWidth  = 0x20, // bit 5
    Italic       = 0x40, // bit 6
    Underline    = 0x80  // bit 7
  };
  struct CharGlyph {
      char c;
      GlyphType mode;
  };

// Bitwise helpers
inline GlyphType operator|(GlyphType a, GlyphType b) {
    return static_cast<GlyphType>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline GlyphType operator&(GlyphType a, GlyphType b) {
    return static_cast<GlyphType>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline GlyphType& operator|=(GlyphType& a, GlyphType b) {
    a = a | b;
    return a;
}

inline GlyphType& operator&=(GlyphType& a, GlyphType b) {
    a = a & b;
    return a;
}
inline GlyphType operator~(GlyphType& g) {
    return static_cast<GlyphType>(~static_cast<uint8_t>(g));
}
inline bool hasFlag(GlyphType value, GlyphType flag) {
  return (static_cast<uint8_t>(value) & static_cast<uint8_t>(flag)) != 0;
}
static std::vector<AsyncClient*> clients; // a list to hold all clients

// public uart::UARTDevice, 
class Epson : public Print, public Component {
public:
  
  Epson();
  
void printTextWrap(const std::string &utf8_text);
void printText(const char *utf8_text);
size_t write(uint8_t c);
size_t writeBytes(const char* inData,int length);

void dump_config() override;

void start();

int getStatus();
void characterSet(uint8_t n);
void defaultLineSpacing();
//Text Mode Functions
void updateTextMode(GlyphType mask, bool enable);
void boldOn();
void boldOff();
void emphasizedOn();
void emphasizedOff();
void underlineOn();
void underlineOff();
void doubleHeightOn();
void doubleHeightOff();
void doubleWidthOn();
void doubleWidthOff();
void doubleSizeOn();
void doubleSizeOff();
void italicOn();
void italicOff();
void smallTextOn();
void smallTextOff();

void apply_format(const std::string &token, bool enable);
void execute_token(std::string token);
//
void codePage(uint8_t n);
void fontA();
void fontB();
void feed(uint8_t n);
void feed();
void speed(int speed);
void letterSpacing(int spacing);
void lineSpacing(uint8_t n);
void reverseOff();
void reverseOn();
void justifyLeft();
void justifyCenter();
void justifyRight();
void barcodeHeight(uint8_t n);
void barcodeWidth(uint8_t n);
void barcodeNumberPosition(uint8_t n);
void printBarcode(uint8_t m,uint8_t n);
void printQRCode(const std::string &text, uint8_t size);
void cut();
void printString(const char* text);
void logWrapback(const char* text);
int configureImagePage(const bool highDensity,const uint32_t width,const uint32_t height);
int configureImage(const bool highDensity,const uint32_t width,const uint32_t height);
void finishImage();
void printImageLine(const char* line_buffer, const int line_length,const bool highDensity);
void printLogo(int logoNum);

// void initTCP(Epson printer);
// void handleNewClient(void* arg, AsyncClient* client);
// void handleError(void* arg, AsyncClient* client, int8_t error);
// void handleData(void* arg, AsyncClient* client, void *data, size_t len);
// void handleDisconnect(void* arg, AsyncClient* client);
// void handleTimeOut(void* arg, AsyncClient* client, uint32_t time);

void startTCPServer();
bool isAvailable();
void listenOnTCPServer();
void disconnectTCPClient();
void stopTCPServer();
bool connected();
bool hasData();
char read();
int read(char * const line_buffer, int buf_size);
private:  
  WiFiServer tcpServer;
  WiFiClient tcpClient;
  bool serverStarted = false;
  bool clientConnected = false;
  bool imagePageMode = false;
  uint32_t currentImageWidth = 0;
  uint32_t currentImageHeight = 0;
  int currentImageDensity = 1;
  GlyphType currentTextMode = GlyphType::Normal;  // keeps track of ESC ! mode bits


  
    std::vector<CharGlyph> lineBuffer_;
    std::vector<CharGlyph> wordBuffer_;
    uint8_t currLineWidth_ = 0;
    size_t lastSpaceIndex_ = std::string::npos;

    // Escape parsing state
    bool parsing_escape_ = false;
    bool maybe_escape_ = false;
    std::string escape_buffer_;

    uint8_t glyphWidth(GlyphType g) {
        uint8_t w = 1;
        if (static_cast<uint8_t>(g & GlyphType::DoubleWidth)) w *= 2;
        if (static_cast<uint8_t>(g & GlyphType::DoubleHeight)) w *= 2;
        if (static_cast<uint8_t>(g & GlyphType::Small)) w = 1;
        return w;
    }

    void flushLine();
    void printChar(const CharGlyph &cg);
};

}
}
#endif