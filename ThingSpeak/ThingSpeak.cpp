#include "mbed.h"
#include "MHZ19.h"
#include "network-helper.h"
#include "libThingSpeak\ThingSpeak.h"
#include "secrets.h"

#define PRINT_STR_REPEAT(str, times) \
{ \
  for (int i = 0; i < times; ++i) \
    printf("%s", str); \
  puts(""); \
}

MHZ19 myMHZ19;                                             // Constructor for library
BufferedSerial rser(USBTX, USBRX);                       // (Uno example) create device to MH-Z19 serial
BufferedSerial rserMHZ19(PC_12, PD_2);                       // (Uno example) create device to MH-Z19 serial
NetworkInterface *net;
ThingSpeak thingSpeak;
TCPSocket socket;


unsigned long getDataTimer = 0;

int main() {
  bool bStart = true;
  SocketAddress adr;
  nsapi_error_t result;

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

  net = connect_to_default_network_interface();
  if (!net) {
    printf("Error! No network inteface found.\n");
    thread_sleep_for(30000);
    system_reset();
    return -1;
  }

  result = net->gethostbyname(THINGSPEAK_URL, &adr);
  if (result != NSAPI_ERROR_OK ) {
    printf("Error! net->gethostbyname returned: %d\n", result);
    thread_sleep_for(30000);
    system_reset();
  }
  adr.set_port(THINGSPEAK_PORT_NUMBER);

  // Initialize ThingSpeak
  thingSpeak.begin(&socket);

  while(true) {
    int CO2, ret; 
    int8_t Temp;

    result = socket.open(net);
    if (result != NSAPI_ERROR_OK) {
      printf("Error! socket.open(net) returned: %d\n", result);
      thread_sleep_for(30000);
      system_reset();
    }

    result = socket.connect(adr);
    if (result != NSAPI_ERROR_OK) {
      printf("Error! socket.connect(adr) Failed (%d).\n", result);
      thread_sleep_for(30000);
      system_reset();
    }
    
    /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even 
    if below background CO2 levels or above range (useful to validate sensor). You can use the 
    usual documented command with getCO2(false) */

    CO2 = myMHZ19.getCO2();                               // Request CO2 (as ppm)
    Temp = myMHZ19.getTemperature();                      // Request Temperature (as Celsius)
//    printf("CO2 (ppm): %d, Temperatur (C): %d\n", CO2, Temp);                                
    printf("%d,%d\n", CO2, Temp);

    thingSpeak.setField(1, CO2);
    thingSpeak.setField(2, Temp);
    if(bStart)
      thingSpeak.setField(3, 1);
    bStart = false;
    ret = thingSpeak.writeFields(SECRET_CH_ID, SECRET_WRITE_APIKEY);
    if(ret == 200){
      printf("Channel update successful.\n");
    }
    else{
      printf("Problem updating channel. HTTP or lib error code %d\n", ret);
      thread_sleep_for(30000);
      system_reset();
    }
    socket.close();
    
    thread_sleep_for(15000);
  }
}
