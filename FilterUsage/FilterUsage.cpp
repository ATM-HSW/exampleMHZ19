/*
    Filter mode is useful if you are displaying in a graph or stastical analyses.
    When the sensor is reset, reading the CO2 value for around 30 seconds will produce inaccurate results
    while the IR sensor warms. The filter can behave in one of two ways:
    
    Mode 1)  myMHZ19.setFilter(true, true) (default)        <-- you can simply use setFilter() here;
    Values are filtered, and returned value is set to 0. An "errorCode" is set.
    Mode 2)  myMHZ19.setFilter(true, false)
    Values are not filtered but constrained if out of variable range. You must manually use the
    errorCode to complete the "filter". 
    (note, the down side to the filter is that an additional command is sent on each request. 
    For most applications, this is no problem).
    * Uncomment / comment out one of the two examples below*
*/

#include "mbed.h"
#include "MHZ19.h"

#define MODE 0                              // <---------------- Set to 0 change to switch code for each mode 

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
  int CO2Unlimited;

  set_time(0);
  rser.set_baud(MBED_CONF_PLATFORM_STDIO_BAUD_RATE);       // Device to serial monitor feedback

  printf("\n");
#ifdef MBED_MAJOR_VERSION
  int num = printf("Mbed OS version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  puts("");
  PRINT_STR_REPEAT("-", num);
#endif

  rserMHZ19.set_baud(9600);                                // (Uno example) device to MH-Z19 serial start   
  myMHZ19.begin(rserMHZ19);                                // *Serial(Stream) refence must be passed to library begin(). 
  myMHZ19.printCommunication(false, false);
#if MODE
    myMHZ19.setFilter(true, true);                           
#else
    myMHZ19.setFilter(true, false);  
#endif

  oldseconds = time(NULL);
  while(true) {
    seconds = time(NULL);
    if(seconds - oldseconds > 15) {
      // get sensor readings as signed integer        
      CO2Unlimited = myMHZ19.getCO2(true);
        
#if MODE
      // ######### Mode 1 ############# //
      printf("CO2: %d PPM\n", CO2Unlimited);

      if(CO2Unlimited != 0) {
          /* send/store your data code */
      }
      else {
          /* ignore data code */
      }
#else  
      // ######### Mode 2 ############# //         

      // get library error code returned getCO2 function
      uint8_t thisCode = myMHZ19.errorCode;
      
      // handle code based upon error type
      if(thisCode != RESULT_OK) {
        // was it the filter ?
        if(thisCode == RESULT_FILTER) {
          printf("*** Filter was triggered ***\n");
          printf("Offending Value: %d\n", CO2Unlimited);
        }
        // if not, then...
        else {
          printf("Communication Error Found. Error Code: %d\n", thisCode);
        }
      } 
      // error code was result OK. Print as "normal" 
      else {
        printf("CO2: %d PPM\n", CO2Unlimited);
      }
#endif
      oldseconds = seconds;
    }
    thread_sleep_for(100);
  }
}
