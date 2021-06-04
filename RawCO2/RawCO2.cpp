/* 
  Raw CO2:
  Using the raw value competantly requires insight into the technology.
  
  However, itcan still be useful to have a rough value as a 'sanity check'.
  This is because the raw is not affect by span/range/zero/temperature.
  By plotting the Raw value vs CO2 ppm the full range (2000 usually),
  a trend can be produced (an exponetial rend is ideal for a 2000 range).
 */

#include "mbed.h"
#include "MHZ19.h"

#define PRINT_STR_REPEAT(str, times) \
{ \
  for (int i = 0; i < times; ++i) \
    printf("%s", str); \
  puts(""); \
}

MHZ19 myMHZ19;                             // Constructor for library
BufferedSerial rser(USBTX, USBRX);         // Debug output serial
BufferedSerial rserMHZ19(PC_12, PD_2);     // MH-Z19 serial

int main() {
  time_t seconds, oldseconds;
  double adjustedCO2;

  set_time(0);
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

  oldseconds = time(NULL);
  while(true) {
    seconds = time(NULL);
    if(seconds - oldseconds > 15) {
      adjustedCO2 = myMHZ19.getCO2Raw();

      printf("---------------------\n");
      printf("Raw CO2:      %f\n", adjustedCO2);
      adjustedCO2 = 6.60435861e+15 * exp(-8.78661228e-04 * adjustedCO2);      // Exponential equation for Raw & CO2 relationship
      printf("Adjusted CO2: %fppm\n", adjustedCO2);

      oldseconds = seconds;
    }
    thread_sleep_for(100);
  }
}