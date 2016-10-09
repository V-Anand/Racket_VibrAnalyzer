/*
* Bitmaps need to be formatted as 16bppRgb565, flipped vertically
* and a 6 byte header needs to be appended at the start of the array.
* Use the following tool to create arrays for images. 
* https://github.com/MikroElektronika/HEXIWEAR/tree/master/SW/ResourceCollectionTool
* It takes an image and outputs the array. It handles the flipping and the 6 byte header.  
* Image needs to be converted outside the tool to fit the screen 
* (Max Dimensions:96px by 96px).
*/

#include "mbed.h"
#include "Hexi_OLED_SSD1351.h"
#include "tennis.h"
#include "Hexi_KW40Z.h"
#include "FXOS8700CQ.h"

#define LED_ON      0
#define LED_OFF     1
#define MAX_WIDTH   96
#define MAX_HEIGHT  96
#define MAX_VIBS    96
   
void StartHaptic(void);
void StopHaptic(void const *n);

typedef enum {
    T_APP_UNKNOWN,
    T_APP_READY,
    T_APP_STARTED,
    T_APP_STOPPED,
    T_APP_RESET_WAIT,
    T_APP_RESET
} RacketAppState_t;

/* Instantiate the RGB LED and Haptic motor pinout */
DigitalOut redLed(LED1);
DigitalOut greenLed(LED2);
DigitalOut blueLed(LED3);
DigitalOut haptic(PTB9);

/* Instantiate the Hexi KW40Z Driver (UART TX, UART RX) */ 
KW40Z kw40z_device(PTE24, PTE25);

/* Instantiate the SSD1351 OLED Driver */
SSD1351 oled(PTB22,PTB21,PTC13,PTB20,PTE6, PTD15); // (MOSI,SCLK,POWER,CS,RST,DC)

/* Define timer for haptic feedback */
RtosTimer hapticTimer(StopHaptic, osTimerOnce);

/* Accelerator Functions */
FXOS8700CQ fxos(PTC11 /* sda */, PTC10 /* scl */);

/* Accelerator interrupt INT1 */
InterruptIn accelIntPin(PTC1);

/* Serial interface */
Serial pc(USBTX, USBRX);

SRAWDATA accelData;

int accelReady=0;

RacketAppState_t flag=T_APP_READY;

uint32_t cntVib=0;

float vibs[MAX_VIBS] = {0.0};

void ButtonLeft(void)
{
    flag = (T_APP_READY == flag) ? T_APP_STARTED : flag;

    StartHaptic();

    redLed      = LED_ON;
    greenLed    = LED_ON;
    blueLed     = LED_OFF;
}

void ButtonRight(void)
{
    flag = (T_APP_STARTED == flag) 
           ? T_APP_STOPPED 
           : (T_APP_RESET_WAIT == flag) ? T_APP_RESET : flag;

    StartHaptic();
    
    redLed      = LED_ON;
    greenLed    = LED_OFF;
    blueLed     = LED_OFF;    
}

void StartHaptic(void)
{
    hapticTimer.start(75);
    haptic = 1;
}

void StopHaptic(void const *n) {
    haptic = 0;
    hapticTimer.stop();
    redLed      = LED_OFF;
    greenLed    = LED_OFF;
    blueLed     = LED_OFF;
}

void AccelIntCallback() {
    accelReady = 1;
}

void DisplayStarted() {
    oled.DrawBox(84,6,8,8,COLOR_RED);
}

void DisplayResult() {
    char text[15];

    /* Fill screen with black */
    oled.FillScreen(COLOR_BLACK);

    uint8_t size = (cntVib > MAX_VIBS) ? MAX_VIBS : cntVib;
    uint8_t idx = (cntVib > MAX_VIBS) ? (cntVib % MAX_VIBS) : 0;
    float total = 0.0;
    for(uint8_t x=0; x < size; ++x, ++idx) {
      total += vibs[ idx % MAX_VIBS];
      
      float vib = log2( vibs[ idx % MAX_VIBS ] );
      uint8_t y = (vib >= MAX_VIBS) ? (MAX_VIBS - 1) : (uint8_t)(vib+0.5);
      oled.DrawPixel(x, y, COLOR_YELLOW);
    }

    oled_text_properties_t textProps={0};
    oled.GetTextProperties(&textProps);
    textProps.fontColor = COLOR_GREEN;
    sprintf(text, "Avg:%.2f", (total/size)); 
    oled.SetTextProperties(&textProps);
    oled.Label((uint8_t*) text, 20, 50);
}
void DisplayImage() {
    /* Pointer for the image to be displayed  */  
    const uint8_t *image1;

    /* Setting pointer location of the 96 by 96 pixel bitmap */
    image1  = tennis_app_image2;

    /* Fill screen with black */
    oled.FillScreen(COLOR_BLACK);
        
    oled.DrawImage(image1,0,0);
}
uint16_t GetMagn(uint16_t value) {
    bool const isNeg = ((value & (1<<15)) != 0);
    if (isNeg) {
      return (~value + 1);
    }
    return value;
}
void PrintResult() {
    pc.printf("Count=%X \n", cntVib );
    uint8_t size = (cntVib > MAX_VIBS) ? MAX_VIBS : cntVib;
    uint16_t idx = (cntVib > MAX_VIBS) ? (cntVib % MAX_VIBS) : 0;
    for(uint16_t i=0;i < size; ++i, ++idx) {
      pc.printf("%.2f \n", log2(vibs[ idx % MAX_VIBS]));
    }
}

int main() {
    
    pc.baud(9600);

    pc.printf("VibrAnalyzer Start \n");

    redLed      = LED_OFF;
    greenLed    = LED_OFF;
    blueLed     = LED_OFF;
    
    /* Accel Int setup */
    accelIntPin.mode(PullUp);
    
    /* Accel Int callback setup */
    accelIntPin.fall(&AccelIntCallback);

    /* Turn on the backlight of the OLED Display */
    oled.DimScreenON();
    
    /* Register callbacks to application functions */
    kw40z_device.attach_buttonLeft(&ButtonLeft);
    kw40z_device.attach_buttonRight(&ButtonRight);

    DisplayImage();

    /* Accel data enable */
    fxos.enable_trans_accel();
    
    while (true) 
    {
        if (T_APP_STARTED == flag) 
        {
            if (accelReady == 1)
            {
                accelReady = 0;
                if (I2C_SUCCESS == fxos.read_accel(&accelData))
                {
                  /* Get Vector magnitude of each axis */
                  uint16_t xAxis = GetMagn(accelData.x);
                  uint16_t yAxis = GetMagn(accelData.y);
                  uint16_t zAxis = GetMagn(accelData.z);
                  /* RMS acceleration */
                  /* Plot Vibration for Analysis */
                  vibs[cntVib % MAX_VIBS] = sqrt(
                    ((xAxis*xAxis) + (yAxis*yAxis) + (zAxis*zAxis))/3);
                  ++cntVib;
                }
            }
            DisplayStarted();
        }
        else if (T_APP_STOPPED == flag)
        {
            DisplayResult();

            flag = T_APP_RESET_WAIT;
        }
        else if (T_APP_RESET == flag)
        {
            PrintResult();

            cntVib=0;

            flag = T_APP_READY;

            DisplayImage();
        }
        Thread::wait(50);
    }
}
