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
#include "images.h"
#include "tennis.h"
#include "Hexi_KW40Z.h"
#include "FXOS8700CQ.h"

#define LED_ON      0
#define LED_OFF     1
   
void StartHaptic(void);
void StopHaptic(void const *n);

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

int flag=0;

uint32_t cntVib=0, maxVib=0, minVib=0xffffffff;

void ButtonLeft(void)
{
    StartHaptic();
    
    redLed      = LED_ON;
    greenLed    = LED_ON;
    blueLed     = LED_OFF;
    
    flag = 1;
}

void ButtonRight(void)
{
    StartHaptic();
    
    redLed      = LED_ON;
    greenLed    = LED_OFF;
    blueLed     = LED_OFF;    
    
    flag = 2;
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

void DisplayResult() {
    char text[15];

    oled_text_properties_t textProps={0};
    oled.GetTextProperties(&textProps);
    textProps.fontColor = COLOR_GREEN;
    oled.SetTextProperties(&textProps);

    sprintf(text, "TOT=%08x", cntVib); 
    oled.Label((uint8_t*) text, 5, 48);

    sprintf(text, "MAX=%08x", maxVib); 
    oled.Label((uint8_t*) text, 5, 60);

    sprintf(text, "MIN=%08x", minVib);
    oled.Label((uint8_t*) text, 5, 72);
}

int main() {
    
    pc.baud(9600);

    pc.printf("VibrAnalyzer Start \n");

    redLed      = LED_OFF;
    greenLed    = LED_OFF;
    blueLed     = LED_OFF;
    
    /* Pointer for the image to be displayed  */  
    const uint8_t *image1;
    const uint8_t *image2;
    const uint8_t *image3;

    /* Setting pointer location of the 96 by 96 pixel bitmap */
    image1  = /*Relay_OFF*/ tennis_app_image2;
    image2  = Button_OFF;
    image3  = Button_ON;

    /* Accel Int setup */
    accelIntPin.mode(PullUp);
    
    /* Accel Int callback setup */
    accelIntPin.fall(&AccelIntCallback);

    /* Turn on the backlight of the OLED Display */
    oled.DimScreenON();
    
    /* Register callbacks to application functions */
    kw40z_device.attach_buttonLeft(&ButtonLeft);
    kw40z_device.attach_buttonRight(&ButtonRight);

    /* Fill screen with black */
    oled.FillScreen(COLOR_BLACK);
        
    oled.DrawImage(image1,0,0);

    /* Accel data enable */
    fxos.enable_trans_accel();
    
    while (true) 
    {
        if (flag == 1) 
        {
            if (accelReady == 1)
            {
                accelReady = 0;

                ++cntVib;

                if (I2C_SUCCESS == fxos.read_accel(&accelData))
                {
                  /* Get Vector magnitude of each axis */
                  uint16_t xAxis = (accelData.x & 0x7fff);
                  uint16_t yAxis = (accelData.y & 0x7fff);
                  uint16_t zAxis = (accelData.z & 0x7fff);
                  /* Magnitude rounded-up */
                  uint32_t accMag = (uint32_t)(0.5 + ((xAxis*xAxis) + 
                                      (yAxis*yAxis) + (zAxis*zAxis)));
                  /* Update Min and Max Vibration */
                  if (accMag > maxVib) {
                    maxVib = accMag;
                  }
                  if (accMag < minVib) {
                    minVib = accMag;
                  }
                }
            }

        }
        else if (flag == 2)
        {
            pc.printf("Count=%X Min=%X Max=%X\n",
                      cntVib, minVib, maxVib);
        }
        Thread::wait(50);
    }
}
