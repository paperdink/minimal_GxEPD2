
#include "config.h"

#define ENABLE_GxEPD2_GFX 1

#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>

/* Uncomment only one of the below display
 * definitions based on the device you have
 */
// paperd.ink classic
//GxEPD2_BW<GxEPD2_420, GxEPD2_420::HEIGHT> display(GxEPD2_420(/*CS=*/ EPD_CS, /*DC=*/ EPD_DC, /*RST=*/ EPD_RST, /*BUSY=*/ EPD_BUSY));
// paperd.ink merlot
GxEPD2_3C<GxEPD2_420c_Z21, GxEPD2_420c_Z21::HEIGHT> display(GxEPD2_420c_Z21(/*CS=5*/ EPD_CS, /*DC=*/ EPD_DC, /*RST=*/ EPD_RST, /*BUSY=*/ EPD_BUSY));

#include "albert.h"

void helloWorld(GxEPD2_GFX& display);
void showPartialUpdate(GxEPD2_GFX& display);
void helloFullScreenPartialMode(GxEPD2_GFX& display);
void showBox(GxEPD2_GFX& display, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool partial);
void showFont(GxEPD2_GFX& display, const char name[], const GFXfont* f);
void drawFont(GxEPD2_GFX& display, const char name[], const GFXfont* f);
void showPartialUpdate(GxEPD2_GFX& display);
void drawBitmaps(GxEPD2_GFX& display);

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("setup");
    delay(100);
    pinMode(EPD_EN, OUTPUT);
    pinMode(EPD_RST, OUTPUT);
    pinMode(SD_EN, OUTPUT);

    // Enable power to EPD
    digitalWrite(EPD_EN, LOW);
    digitalWrite(EPD_RST, HIGH);

    display.init(115200);
    // first update should be full refresh
    helloWorld(display);
    delay(5000);
    helloFullScreenPartialMode(display);
    delay(3000);
    showPartialUpdate(display);
    delay(3000);
    showFont(display, "FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
    delay(3000);
    drawBitmaps(display);
    
    // Disable power to EPD
    digitalWrite(EPD_EN, HIGH);
}


void loop()
{
}

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()

void helloWorld(GxEPD2_GFX& display)
{
    //Serial.println("helloWorld");
    char text[] = "Hello World!";
    // most e-papers have width < height (portrait) as native orientation, especially the small ones
    // in GxEPD2 rotation 0 is used for native orientation (most TFT libraries use 0 fix for portrait orientation)
    // set rotation to 1 (rotate right 90 degrees) to have enough space on small displays (landscape)
    display.setRotation(0);
    // select a suitable font in Adafruit_GFX
    display.setFont(&FreeMonoBold9pt7b);
    // on e-papers black on white is more pleasant to read
    // or you could do it other way round
    display.setTextColor(GxEPD_WHITE);
    // Adafruit_GFX has a handy method getTextBounds() to determine the boundary box for a text for the actual font
    int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
                                                               // center bounding box by transposition of origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;
    // full window mode is the initial mode, set it anyway
    display.setFullWindow();
    // here we use paged drawing, even if the processor has enough RAM for full buffer
    // so this can be used with any supported processor board.
    // the cost in code overhead and execution time penalty is marginal
    // tell the graphics class to use paged drawing mode
    display.firstPage();
    do
    {
        // this part of code is executed multiple times, as many as needed,
        // in case of full buffer it is executed once
        // IMPORTANT: each iteration needs to draw the same, to avoid strange effects
        // use a copy of values that might change, don't read e.g. from analog or pins in the loop!
        
        //display.fillScreen(GxEPD_WHITE); 
        // set the background to white/black/red (fill the buffer with appropriate value)
        display.fillScreen(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
        display.setCursor(x, y); // set the postition to start printing text
        display.print(text); // print some text
                             // end of part executed multiple times
    }
    // tell the graphics class to transfer the buffer content (page) to the controller buffer
    // the graphics class will command the controller to refresh to the screen when the last page has been transferred
    // returns true if more pages need be drawn and transferred
    // returns false if the last page has been transferred and the screen refreshed for panels without fast partial update
    // returns false for panels with fast partial update when the controller buffer has been written once more, to make the differential buffers equal
    // (for full buffered with fast partial update the (full) buffer is just transferred again, and false returned)
    while(display.nextPage());
    //Serial.println("helloWorld done");
}

char HelloWorld[] = "Hello World!";

void helloFullScreenPartialMode(GxEPD2_GFX& display)
{
    //Serial.println("helloFullScreenPartialMode");
    char fullscreen[] = "Full screen update";
    char fpm[] = "Fast partial mode supported";
    char spm[] = "Slow partial mode supported";
    char npm[] = "No partial mode supported";
    display.setPartialWindow(0, 0, display.width(), display.height());
    display.setRotation(0);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);

    char* updatemode;
    if(display.epd2.hasFastPartialUpdate){
        updatemode = fpm;
    }else if(display.epd2.hasPartialUpdate){
        updatemode = spm;
    }else {
        updatemode = npm;
    }
    // do this outside of the loop
    int16_t tbx, tby; uint16_t tbw, tbh;
    // center update text
    display.getTextBounds(fullscreen, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t utx = ((display.width() - tbw) / 2) - tbx;
    uint16_t uty = ((display.height() / 4) - tbh / 2) - tby;
    // center update mode
    display.getTextBounds(updatemode, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t umx = ((display.width() - tbw) / 2) - tbx;
    uint16_t umy = ((display.height() * 3 / 4) - tbh / 2) - tby;
    // center HelloWorld
    display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t hwx = ((display.width() - tbw) / 2) - tbx;
    uint16_t hwy = ((display.height() - tbh) / 2) - tby;

    display.fillScreen(GxEPD_WHITE);
    display.setCursor(hwx, hwy);
    display.print(HelloWorld);
    display.setCursor(utx, uty);
    display.print(fullscreen);
    display.setCursor(umx, umy);
    display.print(updatemode);

    display.display();
    //Serial.println("helloFullScreenPartialMode done");
}

void showBox(GxEPD2_GFX& display, uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool partial)
{
    //Serial.println("showBox");
    display.setRotation(1);
    if(partial){
        display.setPartialWindow(x, y, w, h);
    }else {
        display.setFullWindow();
    }
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
        display.fillRect(x, y, w, h, GxEPD_BLACK);
    }
    while(display.nextPage());
    //Serial.println("showBox done");
}

void showFont(GxEPD2_GFX& display, const char name[], const GFXfont* f)
{
    display.setFullWindow();
    display.setRotation(0);
    display.setTextColor(GxEPD_BLACK);
    display.firstPage();
    do
    {
        drawFont(display, name, f);
    }
    while(display.nextPage());
}

void drawFont(GxEPD2_GFX& display, const char name[], const GFXfont* f)
{
    //display.setRotation(0);
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setFont(f);
    display.setCursor(0, 0);
    display.println();
    display.println(name);
    display.println(" !\"#$%&'()*+,-./");
    display.println("0123456789:;<=>?");
    display.println("@ABCDEFGHIJKLMNO");
    display.println("PQRSTUVWXYZ[\\]^_");
    if(display.epd2.hasColor){
        display.setTextColor(GxEPD_RED);
    }
    display.println("`abcdefghijklmno");
    display.println("pqrstuvwxyz{|}~ ");
}

// note for partial update window and setPartialWindow() method:
// partial update window size and position is on byte boundary in physical x direction
// the size is increased in setPartialWindow() if x or w are not multiple of 8 for even rotation, y or h for odd rotation
// see also comment in GxEPD2_BW.h, GxEPD2_3C.h or GxEPD2_GFX.h for method setPartialWindow()
// showPartialUpdate() purposely uses values that are not multiples of 8 to test this

void showPartialUpdate(GxEPD2_GFX& display)
{
    // some useful background
    char text[] = "Partial update demo";
    int16_t tbx, tby; uint16_t tbw, tbh; // boundary box window
    display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh); // it works for origin 0, 0, fortunately (negative tby!)
                                                               // center bounding box by transposition of origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;

    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x,y);
    display.print(text);
    display.display(false);

    // use asymmetric values for test
    uint16_t box_x = 10;
    uint16_t box_y = 15;
    uint16_t box_w = 70;
    uint16_t box_h = 20;
    uint16_t cursor_y = box_y + box_h - 6;
    float value = 13.95;
    uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 10;
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);
    // show where the update box is
    for (uint16_t r = 0; r < 4; r++)
    {
        Serial.println("Starting black background");
        display.setRotation(r);
        //display.setPartialWindow(box_x, box_y, box_w, box_h);
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
        //display.fillScreen(GxEPD_BLACK);
        display.displayWindow(box_x, box_y, box_w, box_h);
        Serial.println("Waiting for 10 secs after black.");
        delay(10000);
        Serial.println("Starting white background");
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.displayWindow(box_x, box_y, box_w, box_h);
        Serial.println("Waiting for 10 secs after white.");
        delay(10000);
    }
    //return;
    // show updates in the update box
    for (uint16_t r = 0; r < 4; r++)
    {
        display.setRotation(r);
        display.setPartialWindow(box_x, box_y, box_w, box_h);
        for (uint16_t i = 1; i <= 10; i += incr)
        {
            display.firstPage();
            do
            {
                display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
                display.setCursor(box_x, cursor_y);
                display.print(value * i, 2);
            }
            while (display.nextPage());
            delay(500);
        }
        delay(1000);
        display.firstPage();
        do
        {
            display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        }
        while (display.nextPage());
        delay(1000);
    }
}

void drawBitmaps(GxEPD2_GFX& display)
{
    display.setFullWindow();
    display.setRotation(0);
    
    display.fillScreen(GxEPD_WHITE);
    display.drawBitmap(0, 0, albert, 400, 300, GxEPD_BLACK);
    
    if(display.epd2.hasColor){
        display.drawBitmap(0, 0, albert_red, 400, 300, GxEPD_RED);
    }

    display.display();
}
