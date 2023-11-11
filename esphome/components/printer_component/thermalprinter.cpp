/*******************************************
 * see header-file for further informations
 ********************************************/

#include "Arduino.h"
#include "thermalprinter.h"

static const char LF = 0xA; // print buffer and line feed  
    
    
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
  result = this->read();
  return result;
}

int Epson::read(){
    int result;
    result = Serial1.read();
    return result;
}

// Print and feed n lines
// prints the data in the print buffer and feeds n lines
void Epson::feed(uint8_t n){
  Epson::write(0x1B);  
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
  Epson::write(0x1B);  
  Epson::write(0x33);
  Epson::write(n);  
}

// Select default line spacing
// sets the line spacing to 1/6 inch (n=60). This is equivalent to 30 dots.
void Epson::defaultLineSpacing(){
  Epson::write(0x1B);  
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
  Epson::write(0x1B);  
  Epson::write(0x52);
  Epson::write(n);  
}


void Epson::doubleHeightOn(){
  Epson::write(0x1B);    
  Epson::write(0x21);  
  Epson::write(16);
}

void Epson::doubleHeightOff(){
  Epson::write(0x1B);  
  Epson::write(0x21);    
  Epson::write(0);
}

void Epson::boldOn(){
  Epson::write(0x1B);  
  Epson::write(0x21);    
  Epson::write(8);
}

void Epson::boldOff(){
  Epson::write(0x1B);  
  Epson::write(0x21);    
  Epson::write(0);
}

void Epson::underlineOff() {
  Epson::write(0x1B);  
  Epson::write(0x21);    
  Epson::write(0);
}
void Epson::underlineOn() {
  Epson::write(0x1B);  
  Epson::write(0x21);    
  Epson::write(128);
}


// Turn white/black reverse printing mode on/off
void Epson::reverseOn() {
  Epson::write(0x1D);  
  Epson::write(0x42);    
  Epson::write(1);
}
  
void Epson::reverseOff() {
  Epson::write(0x1D);  
  Epson::write(0x42);    
  Epson::write(0);
}

void Epson::justifyLeft() {
  Epson::write(0x1B);  
  Epson::write(0x61);    
  Epson::write(0);
}

void Epson::justifyCenter() {
  Epson::write(0x1B);  
  Epson::write(0x61);    
  Epson::write(1);
}

void Epson::justifyRight() {
  Epson::write(0x1B);  
  Epson::write(0x61);    
  Epson::write(2);
}
//n range 1-255
void Epson::barcodeHeight(uint8_t n) {
  Epson::write(0x1D);  
  Epson::write(0x68);    
  Epson::write(n);
}
//n range 2-6
void Epson::barcodeWidth(uint8_t n) {
  Epson::write(0x1D);  
  Epson::write(0x77);    
  Epson::write(n);
}
//n range 0-3
void Epson::barcodeNumberPosition(uint8_t n) {
  Epson::write(0x1D);  
  Epson::write(0x48);    
  Epson::write(n);
}
//m range 65-73 (code type)
//n (digit length)
void Epson::printBarcode(uint8_t m, uint8_t n) {
  Epson::write(0x1D);  
  Epson::write(0x6B);    
  Epson::write(m);
  Epson::write(n);
}

void Epson::cut() {
  Epson::write(0x1D);
  Epson::write('V');
  Epson::write(66);
  Epson::write(0xA); // print buffer and line feed
}

size_t Epson::write(uint8_t c) {
  Serial1.write(c);
  return 1;
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

}
}