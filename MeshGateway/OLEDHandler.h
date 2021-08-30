  
///////////////////////////////////////////
// Libraries 
///////////////////////////////////////////
#include <Wire.h>
#include "SSD1306Wire.h"


///////////////////////////////////////////
// OLED display
///////////////////////////////////////////

SSD1306Wire  display(0x3c, SDA, SCL);


///////////////////////////////////
// Prototype Functions
///////////////////////////////////

void INIT_DISPLAY();
void READYDISPALY();
void PRINT_ANY_DISPALY(String msg);
void LcdPrint(String display1,String display2, String tm);
void drawRect(void);
void fillRect(void);
void setupOLED(void);


///////////////////////////////////
// INIT_DISPLAY
///////////////////////////////////
int counter=0;
void INIT_DISPLAY()
{
  display.init();
  display.setFont(ArialMT_Plain_10);
  display.setContrast(255);
}

void READYDISPALY()
{

//   drawRect();
//   vTaskDelay(100);
//   display.clear();

//   fillRect();
//   vTaskDelay(100);
  display.clear();

  LcdPrint("Connecting","MeshTracker","000");
  
}
void setupOLED(void){
  INIT_DISPLAY();
  READYDISPALY();
}
void setCounter(int c){
    counter=c;
}
void PRINT_ANY_DISPALY(String msg)
{

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(5, 10,msg);
  display.display();
  
}
void drawProgressBarDemo() {
  int progress = (counter / 5) % 100;
  // draw the progress bar
  display.drawProgressBar(0, 32, 120, 10, progress);

  // draw the percentage as String
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 15, String(progress) + "%");
}


void LcdPrint(String display1,String display2, String tm){
  
 display.clear();
 display.setTextAlignment(TEXT_ALIGN_LEFT);
 display.setFont(ArialMT_Plain_10);
 display.drawString(0, 0,display1);
 display.drawString(0, 10,display2);
 display.drawString(0, 20,tm);
 display.display();

 }

 void drawRect(void) {
  for (int16_t i=0; i<display.getHeight()/2; i+=2) {
    display.drawRect(i, i, display.getWidth()-2*i, display.getHeight()-2*i);
    display.display();
    vTaskDelay(10);
  }
}

void fillRect(void) {
  uint8_t color = 1;
  for (int16_t i=0; i<display.getHeight()/2; i+=3) {
    display.setColor((color % 2 == 0) ? BLACK : WHITE); // alternate colors
    display.fillRect(i, i, display.getWidth() - i*2, display.getHeight() - i*2);
    display.display();
    vTaskDelay(10);
    color++;
  }
  // Reset back to WHITE
  display.setColor(WHITE);
}
