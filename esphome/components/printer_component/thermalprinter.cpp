/*******************************************
 * see header-file for further informations
 ********************************************/

#include "Arduino.h"
#include "thermalprinter.h"
static const char LF = 0xA; // print buffer and line feed  
static const char ESC = 0x1B;
static const char GS = 0x1D;
    
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

// int Epson::read(){
//     int result;
//     result = Serial1.read();
//     return result;
// }

void Epson::letterSpacing(int spacing){
  Epson::write(ESC);  
  Epson::write(0x20);
  Epson::write(spacing);    
}
void Epson::printLogo(){
  //29  40  76  6  0  48  85  kc1  kc2  x  y
  Epson::write(GS);  
  Epson::write(0x28);
  Epson::write(0x4C);  
  Epson::write(6);  
  Epson::write(0);  
  Epson::write(48);  
  Epson::write(85);  
  Epson::write(48);  
  Epson::write(48);
  Epson::write(0);
  Epson::write(0);    
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


void Epson::doubleHeightOn(){
  Epson::write(ESC);    
  Epson::write(0x21);  
  Epson::write(16);
}

void Epson::doubleHeightOff(){
  Epson::write(ESC);  
  Epson::write(0x21);    
  Epson::write(0);
}

void Epson::boldOn(){
  Epson::write(ESC);  
  Epson::write(0x21);    
  Epson::write(8);
}

void Epson::boldOff(){
  Epson::write(ESC);  
  Epson::write(0x21);    
  Epson::write(0);
}

void Epson::underlineOff() {
  Epson::write(ESC);  
  Epson::write(0x21);    
  Epson::write(0);
}
void Epson::underlineOn() {
  Epson::write(ESC);  
  Epson::write(0x21);    
  Epson::write(128);
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
void Epson::printImage(const uint8_t* image,int width,int height)
{
// '// Set graphics data: [Function 67] Define the NV graphics data (raster format)
// '// 128(=8*16) dots wide and 120 dots tall with respect to key code "G1"
// '// GS ( L   pL  pH   m  fn   a kc1/Kc2  b  xL  xH  yL  yH   c
//https://download4.epson.biz/sec_pubs/pos/reference_en/escpos/ref_escpos_en/graphics.html

  for (int i=0;i<(width*height);i++)
  {
    ESP_LOGD("INFO","%d",image[i]);
  }
  if (width != 384 || height > 65635) {
    ESP_LOGD("INFO","Image size error width: %d, Height: %d",width,height);
  }

  //Print LSB first bitmap
  Epson::write(18);
  Epson::write(118);
  
  Epson::write((byte)(height & 255)); 	//height LSB
  Epson::write((byte)(height >> 8)); 	//height MSB

  
  for (int y = 0; y < height; y++) {
    sleep(20);
    for (int x = 0; x < (width/8); x++) {
      Epson::write(image[x + (y*x)]);
    }	
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

void Epson::startTCPServer()
{
  if(serverStarted)
  {
    stopTCPServer();
  }
  Epson::print("Start TCP Server\n");
  tcpServer = WiFiServer(8888);
  tcpServer.begin();
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
      tcpClient = tcpServer.available();
    }
    if(tcpClient)
    {
      // tcpClient = &active_client;
      if (tcpClient.connected())
      {
        //this check effectively oneshots the debug statements
        if(!clientConnected)
        {
          Epson::print("TCP Client Connected\n");
          clientConnected = true;
        }
      }else{
        if(clientConnected)
        {
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
  Epson::print("READ\n");
  return tcpClient.read();
}
void Epson::listenOnTCPServer()
{
  if (!serverStarted)
  {
    startTCPServer();
  }
  int i = 0;
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
      Epson::print("TCP Client Connected\n\n");
    }
    while (Epson::connected())
    {
      if(this->tcpClient.available()>0)
      {
        Epson::print( "Message Received!\n");
        
        while (this->tcpClient.available()>0) {
          char c = this->tcpClient.read();
          Epson::print(c);
        }
        return;
      }
      delay(10);
    }
    delay(100);
    i++;
  }
  Epson::print("End listen\n");
}
void Epson::stopTCPServer()
{
  Epson::print("Stop TCP Server\n");
  if(tcpClient)
  {
    tcpClient.stop();
  }
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