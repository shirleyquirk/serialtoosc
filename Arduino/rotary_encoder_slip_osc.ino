//Rotary Encoder Code 
// 20/11/2019 Benjamin Shirley-Quirk
// Rotary Encoder inputs

#define OSC
#define N_ENCODERS 1
#define OSC_SLIP
//#define OSC_UDP

//can't logging if slip
//#define LOGGING
//#define TIMETAG



#ifdef LOGGING
#include "logging.h"
#define LOGLEVEL LOG_DEBUG
#endif /*LOGGING*/


#if ( (N_ENCODERS) > 0 )
#include <Encoder.h>
#include "Rotary.h"
#define ROTARY
#endif /*N_ENCODERS>0*/

#ifdef OSC
  #include <OSCBundle.h>
  #include <OSCBoards.h>
  #include <OSCTiming.h>

  #define OSC_MESG_SIZE 128

  #ifdef OSC_SLIP
  #ifdef BOARD_HAS_USB_SERIAL
    #include <SLIPEncodedUSBSerial.h>
    SLIPEncodedUSBSerial SLIPSerial( thisBoardsSerialUSB );
  #else
    #include <SLIPEncodedSerial.h>
     SLIPEncodedSerial SLIPSerial(Serial);
  #endif /*BOARD_HAS_USB_SERIAL*/
  #endif /*OSC_SLIP*/
#endif /*OSC*/




myencoder_t encoders[N_ENCODERS];
//macro call defines interrupt functions
//don't know how to call these in a loop
//BOOST_PP_FOR maybe
//ENC(0)
BUTT(0)



OSCBundle osc_bundle;
void setup() {
  #ifdef ROTARY
  encoders[0].enc=new Encoder(3,4);
  encoders[0].pinSW=2;
  encoders[0].button_interrupt=BUT_F(0);
  
    for (int i=0;i<N_ENCODERS;i++){
      encoders[i].min=0;
      encoders[i].max=127;
      encoders[i].value=64;
      encoders[i].wrapping=ROT_CLAMP;
      encoders[i].send_osc_on_update=true;
      encoders[i].bundle=&osc_bundle;//mix C++ class with C struct and weep
      encoders[i].position_fmt="/encoder/%d/position";//  /encoder/<encoder_number>/position i pos
      encoders[i].button_fmt="/encoder/%d/button";//  /encoder/<encoder_number>/button i 1 or 0
      //switching
      encoders[i].send_button=true;
      encoders[i].button_state=false;
      pinMode(encoders[i].pinSW,INPUT_PULLUP);
      attachInterrupt(digitalPinToInterrupt(encoders[i].pinSW),encoders[i].button_interrupt,CHANGE);
    }
  #endif/*ROTARY*/
  #ifdef OSC
    SLIPSerial.begin(115200);//230400,460800
  #endif /*OSC*/
}

void loop() { 
  #ifdef ROTARY
    for (int i=0;i<N_ENCODERS;i++){
      int diff = encoders[i].enc->read();
      encoders[i].enc->write(0);
      if (diff!=0){
        encoders[i].value += diff;
        if (encoders[i].value >= encoders[i].max) {
            if (ROT_CLAMP == encoders[i].wrapping){
              encoders[i].value=encoders[i].max;
            }else if (ROT_CYCLE == encoders[i].wrapping){
              encoders[i].value=encoders[i].min;
            }
          }
          if (encoders[i].value <= encoders[i].min) {
            if (ROT_CLAMP == encoders[i].wrapping){
              encoders[i].value=encoders[i].min;
            }else if (ROT_CYCLE == encoders[i].wrapping){
              encoders[i].value=encoders[i].max;
            }
          }
        if (encoders[i].send_osc_on_update){
          //if encoders[i].valuetype==FLOAT
          //  bundle.add(mesg.c_str()).add(float(encoders[i].value))
          // format osc address
          // e.g. "
          char mesg[OSC_MESG_SIZE];
          sprintf(mesg,encoders[i].position_fmt,i);//how set per_encoder mesg_fmt var_args
          (*encoders[i].bundle).add(mesg).add((float)encoders[i].value/(float)(encoders[i].max-encoders[i].min));
          #ifdef LOGGING
          (*encoders[i].bundle).add(String(encoders[i].value).c_str());
          #endif/*LOGGING*/
          
        }
      }

      if (encoders[i].button_changed && encoders[i].send_button){
        char mesg[OSC_MESG_SIZE];
        sprintf(mesg,encoders[i].button_fmt,i);
        (*encoders[i].bundle).add(mesg).add(encoders[i].button_state);
        encoders[i].button_changed=false;
      }
    }
  #endif/*ROTARY*/
  #ifdef OSC
    static unsigned long lastsent=millis();
  #define POLLTIME 10
    if (millis()>lastsent+POLLTIME){
      if (osc_bundle.size()>0){
        #ifdef TIMETAG
          osc_bundle.setTimetag(oscTime());
        #endif /*TIMETAG*/

        #ifdef OSC_SLIP
          SLIPSerial.beginPacket();
          osc_bundle.send(SLIPSerial);
          SLIPSerial.endPacket();
        #endif /*OSC_SLIP*/
        
        osc_bundle.empty();
      }
      lastsent=millis();
    }
  #endif /*OSC*/
  delay(10);
}
