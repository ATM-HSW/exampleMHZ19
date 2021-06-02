#include "mbed.h"
#include "MHZ19.h"

#define PRINT_STR_REPEAT(str, times) \
{ \
  for (int i = 0; i < times; ++i) \
    printf("%s", str); \
  puts(""); \
}

MHZ19 myMHZ19;                                             // Constructor for library
BufferedSerial rser(USBTX, USBRX);                       // (Uno example) create device to MH-Z19 serial
BufferedSerial rserMHZ19(PC_12, PD_2);                       // (Uno example) create device to MH-Z19 serial


unsigned long getDataTimer = 0;

int main() {
  bool bStart = true;

  rser.set_baud(MBED_CONF_PLATFORM_STDIO_BAUD_RATE);           // Device to serial monitor feedback

  printf("\n");
#ifdef MBED_MAJOR_VERSION
  int num = printf("Mbed OS version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  puts("");
  PRINT_STR_REPEAT("-", num);
#endif

  rserMHZ19.set_baud(9600);                                  // (Uno example) device to MH-Z19 serial start   
  myMHZ19.begin(rserMHZ19);                                // *Serial(Stream) refence must be passed to library begin(). 
  myMHZ19.printCommunication(false, false);

  myMHZ19.autoCalibration();                               // Turn auto calibration ON (OFF autoCalibration(false))

  char myVersion[4];          
  myMHZ19.getVersion(myVersion);
  printf("Firmware Version: %c.%c.%c.%c\n", myVersion[0], myVersion[1], myVersion[2], myVersion[3]);

  printf("Range: %d\n", myMHZ19.getRange());   
  printf("Background CO2: %d\n", myMHZ19.getBackgroundCO2());
  printf("Temperature Cal: %d\n", myMHZ19.getTempAdjustment());
  printf("ABC Status: %s\n", myMHZ19.getABC() ? "ON" : "OFF");

  while(true) {
    int CO2, ret; 
    int8_t Temp;

    /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even 
    if below background CO2 levels or above range (useful to validate sensor). You can use the 
    usual documented command with getCO2(false) */

    CO2 = myMHZ19.getCO2();                               // Request CO2 (as ppm)
    Temp = myMHZ19.getTemperature();                      // Request Temperature (as Celsius)
//    printf("CO2 (ppm): %d, Temperatur (C): %d\n", CO2, Temp);                                
    printf("%d,%d\n", CO2, Temp);

    thread_sleep_for(15000);
  }
}
