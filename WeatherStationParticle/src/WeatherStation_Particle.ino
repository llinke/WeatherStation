#include "blynk.h"
#include "PietteTech_DHT.h"
#include "ThingSpeak.h"

// system defines
#define DHTTYPE  DHT22              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   4         	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   60000  // Sample every minute

unsigned long myChannelNumber = 396211;
const char * myWriteAPIKey = "9KLINGZQRT3IPO5O";
TCPClient client;

//declaration
void dht_wrapper(); // must be declared before the lib initialization

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);

// globals
unsigned int DHTnextSampleTime;	    // Next time we want to start sample
bool bDHTstarted;		    // flag to indicate we started acquisition
int n;                              // counter

//DANGER - DO NOT SHARE!!!!
char auth[] = "95101e5a59b747bf8c5e5410024ed20b"; // Put your blynk token here
//DANGER - DO NOT SHARE!!!!

char VERSION[64] = "0.04";

#define READ_INTERVAL 60000

void setup()
{
  Blynk.begin(auth);
  ThingSpeak.begin(client);
  DHTnextSampleTime = 0;  // Start the first sample immediately
  Particle.publish("DHT22 - firmware version", VERSION, 60, PRIVATE);
}


// This wrapper is in charge of calling
// must be defined like this for the lib work
void dht_wrapper()
{
    DHT.isrCallback();
}

void loop()
{
  Blynk.run(); // all the Blynk magic happens here
  // Check if we need to start the next sample
  if (millis() > DHTnextSampleTime)
  {
    if (!bDHTstarted)
    {		// start the sample
      DHT.acquire();
      bDHTstarted = true;
    }
    if (!DHT.acquiring())
    {
      // Read data values
      float temp = (float)DHT.getCelsius();
      int temp1 = (temp - (int)temp) * 100;
      float humid = (float)DHT.getHumidity();
      int humid1 = (humid - (int)humid) * 100;

      // Publish to Particle
      char tempInChar[32];
      sprintf(tempInChar,"%0d.%d", (int)temp, temp1);
      Particle.publish("Temperature from the dht22 is:", tempInChar, 60, PRIVATE);
      // Blynk virtual pin 1 will be the temperature
      Blynk.virtualWrite(V1, tempInChar);
      sprintf(tempInChar,"%0d.%d", (int)humid, humid1);
      Particle.publish("Humidity from the dht22 is:", tempInChar, 60, PRIVATE);
      // Blynk virtual pin 2 will be the humidity
      Blynk.virtualWrite(V2, tempInChar);

      // Publish to ThingSpeak
      ThingSpeak.setField(1, temp);
      ThingSpeak.setField(2, humid);
      ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

      n++;  // increment counter
      bDHTstarted = false;  // reset the sample flag so we can take another
      DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  // set the time for next sample
    }
  }
}
