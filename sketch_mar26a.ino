#include <b64.h>
#include <HttpClient.h>

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,0,177);

// Initialize the Ethernet client library
// with the IP address and port of the server 
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;

// Name of the server we want to connect to
//const char* kHostname = "192.168.171.140";
const char* kHostname = "apps.gerard.co";
// Path to download (this is the bit after the hostname in the URL
// that you want to download
//const char* kPath = "/status.gmg?sn=22222";
const char* kPath = "/jenkins/";
// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 10000;

int cycle = 1;
String respStr;

int red = 7;
int green = 8;
int blue = 9;
  
int redValue;
int greenValue;
int blueValue;
int delayValue;
int delayBetweenRequests = 10000; //10 seconds

unsigned long lastRequestTime = 0;

void setup() {
// Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  Serial.println("About to Start!");
  
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  
  //Connect Ethernet
  Serial.println("About to Set All Off!");
  setAllOff();
  
  Serial.println("About to Connect!");

  networkConnect();
  Serial.println("Connect Completed!");
}

void networkConnect() {
 // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  
 
  // give the Ethernet shield a second to initialize:
  delay(1000);
}

void loop()
{
   
  if (lastRequestTime == 0 || (millis() - lastRequestTime) > delayBetweenRequests) {
    resetValues();
    int err = 0;
    
    EthernetClient c;
    HttpClient http(c);
    
    err = http.get(kHostname, kPath);
    lastRequestTime = millis();
    
    if (err == 0)
    {
      
      int statusCode = http.responseStatusCode();
      if (statusCode >= 0)
      {
  
        // Usually you'd check that the response code is 200 or a
        // similar "success" code (200-299) before carrying on,
        // but we'll print out whatever response we get
  
        int skip_response_headers = http.skipResponseHeaders();
        if (skip_response_headers >= 0)
        {
          int bodyLen = http.contentLength();
  
        
          // Now we've got to the body, so we can print it out
          unsigned long timeoutStart = millis();
          char c;
          // Whilst we haven't timed out & haven't reached the end of the body
          while ( (http.connected() || http.available()) &&
                 ((millis() - timeoutStart) < kNetworkTimeout) )
          {
              if (http.available())
              {
                  c = http.read();
                  // Print out this character
                  respStr.concat(String(c));
                 
                  bodyLen--;
                  // We read something, reset the timeout counter
                  timeoutStart = millis();
              }
              else
              {
                
            Serial.println("http not available");
                  // We haven't got any data, so let's pause to allow some to
                  // arrive
                  delay(kNetworkDelay);
              }
          }
        }
        else
        {
          Serial.print("Failed to skip response headers: ");
          Serial.println(err);
        }
      }
      else
      {    
        Serial.print("Getting response failed: ");
        Serial.println(err);
      }
    }
    else
    {
      Serial.print("Connect failed: ");
      Serial.println(err);
      networkConnect();
    }
    http.stop();
    
    //Post Request Processing
    processRequest(respStr);
  }
  controlColors();
}

//Reset Every Value
void resetValues() {
  redValue = 0;
  greenValue = 0;
  blueValue = 0;
  delayValue = 0;
  respStr = "";
  cycle = 1;
}

//Handle the LED
void controlColors() {
  
   //Write to PWM Led Strip
   writePins();

   //Are we blinking?
   if (delayValue > 0 ) {
     delay(delayValue);
     setAllOff();
     delay(delayValue);
     
     //Write Pins after delay
     writePins();
   }
   
   
    
}

void writePins() {
 //Write to PWM Led Strip
   analogWrite(red, redValue);
   analogWrite(green, greenValue);
   analogWrite(blue, blueValue);  
}

//Turn of All the LEDs
void setAllOff() {
   analogWrite(red, LOW);
   analogWrite(green, LOW);
   analogWrite(blue, LOW); 
}

//Process The HTTP Response
void processRequest(String str) {
    Serial.println(str);
    int stopping_point = str.indexOf("|"); 
    
    String part = str.substring(0, stopping_point);  
    int i_part = part.toInt();
    
    switch(cycle) {
      case 1: redValue = i_part; break;
      case 2: greenValue = i_part; break;
      case 3: blueValue = i_part; break;
      case 4: delayValue = i_part; break;
    }
    
   cycle++;
   
   if(cycle > 4) {
     return;
   }
   
   String newString = str.substring(stopping_point + 1, str.indexOf('\0'));
   
   processRequest(newString);
}


