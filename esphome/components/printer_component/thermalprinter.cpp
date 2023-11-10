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
  // this->_rxPin = rxPin;
  // this->_txPin = txPin;
  this->start();
}

void Epson::dump_config(){
    ESP_LOGCONFIG(TAG, "JPGIndustries Printer component");
    int result = getStatus();
    char string_buffer[(sizeof(result)) + 1];
    memcpy(string_buffer, &result, sizeof(result));
    string_buffer[sizeof(result)] = 0; // Null termination.
    ESP_LOGCONFIG(TAG, string_buffer);
}
void Epson::start(){

  // pinMode(this->_txPin, OUTPUT);
  // pinMode(this->_rxPin, INPUT);  
  // this->_printer = new SoftwareSerial (this->_rxPin, this->_txPin);
  // this->_printer->begin(9600);
}

// query status of printer. when online returns value 22.
int Epson::getStatus(){
  write(0x10);    
  write(0x04);  
  write(1);
  int result;
  result = read();
  return result;
}

int Epson::read(){
    int result;
    result = uart::UARTDevice::read();
    return result;
}

// Print and feed n lines
// prints the data in the print buffer and feeds n lines
void Epson::feed(uint8_t n){
  write(0x1B);  
  write(0x64);
  write(n);    
}

// Print one line
void Epson::feed(){
  this->feed(1);    
}


// Set line spacing
// sets the line spacing to n/180-inch
void Epson::lineSpacing(uint8_t n){
  write(0x1B);  
  write(0x33);
  write(n);  
}

// Select default line spacing
// sets the line spacing to 1/6 inch (n=60). This is equivalent to 30 dots.
void Epson::defaultLineSpacing(){
  write(0x1B);  
  write(0x32);
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
  write(0x1B);  
  write(0x52);
  write(n);  
}


void Epson::doubleHeightOn(){
  write(0x1B);    
  write(0x21);  
  write(16);
}

void Epson::doubleHeightOff(){
  write(0x1B);  
  write(0x21);    
  write(0);
}

void Epson::boldOn(){
  write(0x1B);  
  write(0x21);    
  write(8);
}

void Epson::boldOff(){
  write(0x1B);  
  write(0x21);    
  write(0);
}

void Epson::underlineOff() {
  write(0x1B);  
  write(0x21);    
  write(0);
}
void Epson::underlineOn() {
  write(0x1B);  
  write(0x21);    
  write(128);
}


// Turn white/black reverse printing mode on/off
void Epson::reverseOn() {
  write(0x1D);  
  write(0x42);    
  write(1);
}
  
void Epson::reverseOff() {
  write(0x1D);  
  write(0x42);    
  write(0);
}

void Epson::justifyLeft() {
  write(0x1B);  
  write(0x61);    
  write(0);
}

void Epson::justifyCenter() {
  write(0x1B);  
  write(0x61);    
  write(1);
}

void Epson::justifyRight() {
  write(0x1B);  
  write(0x61);    
  write(2);
}
//n range 1-255
void Epson::barcodeHeight(uint8_t n) {
  write(0x1D);  
  write(0x68);    
  write(n);
}
//n range 2-6
void Epson::barcodeWidth(uint8_t n) {
  write(0x1D);  
  write(0x77);    
  write(n);
}
//n range 0-3
void Epson::barcodeNumberPosition(uint8_t n) {
  write(0x1D);  
  write(0x48);    
  write(n);
}
//m range 65-73 (code type)
//n (digit length)
void Epson::printBarcode(uint8_t m, uint8_t n) {
  write(0x1D);  
  write(0x6B);    
  write(m);
  write(n);
}

void Epson::cut() {
  write(0x1D);
  write('V');
  write(66);
  write(0xA); // print buffer and line feed
}

size_t Epson::write(uint8_t c) {
  uart::UARTDevice::write(c);
  return 1;
}

void Epson::printString(const char* text)
{
  // Traverse the string 
  for (int i = 0;strlen(text); i++) { 
    // Print current character 
      write(text[i]); 
  } 
  write(LF);
}

void Epson::logWrapback(const char* text)
{
  ESP_LOGCONFIG(TAG, "wrapback: %s",text);
}

}
}