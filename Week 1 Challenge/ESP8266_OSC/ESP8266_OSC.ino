#include <Adafruit_ESP8266.h>
#include <SoftwareSerial.h>

int m = 0; //command pool counter
int cmd[100];
int fadeValue = 0; //represent the brightness of the LED

//boolean print_num = false; //for debugging

#define LED_PIN  5
#define ESP_RX   3
#define ESP_TX   2
#define ESP_RST  4

//---------- Used for connect to AP----------
//#define ESP_SSID "name"  // Your network name here
//#define ESP_PASS "password" // Your network password here


SoftwareSerial mySerial(ESP_RX, ESP_TX);

// Must declare output stream before Adafruit_ESP8266 constructor; can be
// a SoftwareSerial stream, or Serial/Serial1/etc. for UART.
Adafruit_ESP8266 wifi(&mySerial, &Serial, ESP_RST);
// Must call begin() on the stream(s) before using Adafruit_ESP8266 object.


// Converting 32-bit binary to float, reference below
// https://en.wikipedia.org/wiki/Single-precision_floating-point_format
int computeBIN(byte a,byte b,byte c,byte d)
{
  float sum = 1;
  a = (a<<1) | bitRead(b,7);
  
  for(int i = 0;i<8;i++)
  {
    sum += bitRead(d,i) * pow(2,(-23+i));
  }

  for(int i = 8;i<16;i++)
  {
    sum += bitRead(c,i-8) *pow(2,(-23+i));
  }

  for(int i = 16;i<23;i++)
  {
    sum += bitRead(b,i-16) * pow(2,(-23+i));
  }
  
  sum *= pow(2,(a-127));
  
  return int(sum);
}

// Command List
// 1--Check Firmware, 2--Check IP, 3--Open Server, 4--Set AP Mode
// 5--Set AP Parameter, 6--Set Multiple Connection 
void sendCMD(int n)
{
    char buffer[50];

    switch (n)
    {
        case 1:
            wifi.println(F("AT+GMR"));
            break;
        case 2:
            wifi.println(F("AT+CIFSR"));
            break;
        case 3:
            wifi.println(F("AT+CIPSERVER=1,9822"));
            break;
        case 4:
            wifi.println(F("AT+CWMODE=2"));
            break;
        case 5:
            wifi.println(F("AT+CWSAP=\"qiaozhiw\",\"23959822\",5,3"));
            break;
        case 6:
            wifi.println(F("AT+CIPMUX=1"));
            break;
        default:
        break;    
    }

    if(wifi.readLine(buffer, sizeof(buffer))) 
    {
        Serial.println(buffer);
        wifi.find(); // Discard the 'OK' that follows
    } 
    else 
    {
        Serial.println(F("error"));
    }
}

/*
void connectAP()
{
    Serial.print(F("Connecting to WiFi..."));
    if(wifi.connectToAP(F(ESP_SSID), F(ESP_PASS))) 
    {
        // IP addr check isn't part of library yet, but
        // we can manually request and place in a string.
        Serial.print(F("OK\nChecking IP addr..."));
        sendCMD(2);
        //wifi.closeAP();
    } 
    else 
    { 
        // WiFi connection failed
        Serial.println(F("FAIL"));
    }
}
*/

void setup() 
{
    char buffer[50];
    //pinMode(LED_PIN, OUTPUT);

    // This might work with other firmware versions (no guarantees)
    // by providing a string to ID the tail end of the boot message:

    // comment/replace this if you are using something other than v 0.9.2.4!
    wifi.setBootMarker(F("Version:0.9.2.4]\r\n\r\nready"));

    mySerial.begin(9600); // Soft serial connection to ESP8266
    Serial.begin(9600); while(!Serial); // UART serial debug

    // Test if module is ready
    Serial.print(F("Hard reset..."));
    if(!wifi.hardReset()) 
    {
        Serial.println(F("no response from module."));
        for(;;);
    }
    Serial.println(F("OK."));

    Serial.print(F("Soft reset..."));
    if(!wifi.softReset()) 
    {
        Serial.println(F("no response from module."));
        for(;;);
    }
    Serial.println(F("OK."));

    Serial.print(F("Checking firmware version..."));
    sendCMD(1); 

    //connectAP();
    Serial.print(F("Setting up AP"));
    sendCMD(4); //Set AP Mode    
    sendCMD(5); //Set AP Parameter
    sendCMD(6); //Set Multiple Connection
    
    Serial.print(F("Setting up server"));
    sendCMD(3); //Open Server Mode

  
}

void manualControl()
{
    while (mySerial.available())
  {
    Serial.write(mySerial.read());
    
  }  
 while (Serial.available())
  {
    mySerial.write(Serial.read());
    
  }

}

void listenOSC()
{

 while (mySerial.available())
    {      
        cmd[m] = mySerial.read();
        m++;       
        print_num = true;       
    }

  /*
   //------For debugging command------
   if (print_num)
    {
        for(int i=0;i<m;i++)
        {   
            Serial.print(cmd[i]); 
            Serial.print(" ");   
            Serial.println(char(cmd[i])); 
        }

        Serial.print(F("Total "));
        Serial.println(m);
        Serial.println();
        print_num = false;
    }
   */
    
  /*
    //--------For Toggle Button------  
    for(int i=0;i<m;i++)
    {
    if ((cmd[i] == 79) and (cmd[i+1] == 75)) //OK
    {
    m = 0;
    
    if (cmd[i-6] == 63)
    {
        digitalWrite(LED_PIN, HIGH); // Turn_on
        m = 0;
    }
    if (cmd[i-6] == 0)
    {
        digitalWrite(LED_PIN, LOW);
        m = 0;
    }
    }
    }
  */
   
    //---------For Scroll Bar---------  
    for(int i=0;i<m;i++)
    {
    if ((cmd[i] == 44) and (cmd[i+1] == 102)) //equals "," and "f"
    {
       int temp = computeBIN(cmd[i+4],cmd[i+5],cmd[i+6],cmd[i+7]); //still kinda buggy
       if (temp != 0) //hack,avoid sudden change
       {
         fadeValue = temp;
       }
        m = 0; //Clear the command pool
        analogWrite(LED_PIN, fadeValue);
    }
    }
 
   
}
void loop() 
{

  listenOSC();
  //manualControl();
  
  //analogWrite(LED_PIN, fadeValue);  
  //delay(10);
  
}
