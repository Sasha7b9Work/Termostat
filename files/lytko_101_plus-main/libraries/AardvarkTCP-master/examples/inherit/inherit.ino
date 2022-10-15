#include<AardvarkTCP.h>
#include<Ticker.h>

//#define URL "http://robot.local:80/index.html"
//#define URL "http://192.168.1.21:80/index.html"
#define URL "https://robot.local:443/index.html"

class myprotocol: public AardvarkTCP{
    public:
        myprotocol(): AardvarkTCP(){
          onTCPconnect([]{ Serial.printf("User: Connected to %s max payload %d\n",URL,getMaxPayloadSize()); });          
          onTCPdisconnect([](int8_t reason){ Serial.printf("User: Disconnected from %s (reason %d)\n",URL,reason); });
       }

       void connect(const string& url,const uint8_t* fingerprint=nullptr){
          TCPurl(URL,fingerprint);
          TCPconnect();       
       }

      void disconnect(){ TCPdisconnect(); }

      void errorHandler(VARK_cbError f){ onTCPerror(f); }
};


Ticker A1;
myprotocol mp;

void AardvarkErrors(int e,int info){
  switch(e){
    case VARK_TCP_DISCONNECTED:
        Serial.printf("ERROR: NOT CONNECTED info=%d\n",info);
        break;
    case VARK_TCP_UNHANDLED:
        Serial.printf("ERROR: UNHANDLED TCP ERROR info=%d\n",info);
        break;
    case VARK_TLS_BAD_FINGERPRINT:
        Serial.printf("ERROR: TLS_BAD_FINGERPRINT info=%d\n",info);
        break;
    case VARK_TLS_NO_FINGERPRINT:
        Serial.printf("WARNING: NO FINGERPRINT, running insecure\n");
        break;
    case VARK_TLS_NO_SSL:
        Serial.printf("ERROR: secure https:// requested, NO SSL COMPILED-IN: READ DOCS!\n");
        break;
    case VARK_TLS_UNWANTED_FINGERPRINT:
        Serial.printf("WARNING: FINGERPRINT provided, insecure http:// given\n");
        break;
/*
    case VARK_NO_SERVER_DETAILS: //  
        Serial.printf("ERROR:NO_SERVER_DETAILS info=%02x\n",info);
        break;
*/
    case VARK_INPUT_TOO_BIG: //  
        Serial.printf("ERROR: RX msg(%d) that would 'break the bank'\n",info);
        break;
    default:
        Serial.printf("UNKNOWN ERROR: %u extra info %d\n",e,info);
        break;    
  }
}

void setup(){
    Serial.begin(115200);
    Serial.printf("Aardvark Tester %s\n",AARDVARK_VERSION);

    WiFi.begin("XXXXXXXX","XXXXXXXX");
    while(WiFi.status()!=WL_CONNECTED){
      Serial.printf(".");
      delay(1000);
    }
    Serial.printf("WIFI CONNECTED IP=%s\n",WiFi.localIP().toString().c_str());

    mp.errorHandler(AardvarkErrors);
    mp.connect(URL); // no fingerprint
    A1.once_ms(10000,[=](){ mp.disconnect(); });
    
}

void loop() {}