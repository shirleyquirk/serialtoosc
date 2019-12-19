  #include <OSCBundle.h>
  
  #define CTRL_PIN1 2 
  #define CTRL_PIN2 3
  int ctrl=64 ;
  
  // Rotary encoder is wired with the common to ground and the two
  // outputs to pins 5 and 6.
  //Rotary rotary = Rotary(2, 3);
  // Values returned by 'process'
  // No complete step yet.
  #define DIR_NONE 0x0
  // Clockwise step.
  #define DIR_CW 0x10
  // Anti-clockwise step.
  #define DIR_CCW 0x20
  #define R_START 0x0
#define HALF_STEP

  #ifdef HALF_STEP
// Use the half-step state table (emits a code at 00 and 11)
#define R_CCW_BEGIN 0x1
#define R_CW_BEGIN 0x2
#define R_START_M 0x3
#define R_CW_BEGIN_M 0x4
#define R_CCW_BEGIN_M 0x5
const unsigned char ttable[6][4] = {
  // R_START (00)
  {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
  // R_CCW_BEGIN
  {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
  // R_CW_BEGIN
  {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
  // R_START_M (11)
  {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
  // R_CW_BEGIN_M
  {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
  // R_CCW_BEGIN_M
  {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
};
#else
  #define R_CW_FINAL 0x1
  #define R_CW_BEGIN 0x2
  #define R_CW_NEXT 0x3
  #define R_CCW_BEGIN 0x4
  #define R_CCW_FINAL 0x5
  #define R_CCW_NEXT 0x6
  
  const unsigned char ttable[7][4] = {
  
    // R_START
    {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
    // R_CW_FINAL
    {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
    // R_CW_BEGIN
    {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
    // R_CW_NEXT
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
    // R_CCW_BEGIN
    {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
    // R_CCW_FINAL
    {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
    // R_CCW_NEXT
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
  };
#endif/*HALF_STEP*/


#define DEBOUNCE_TIME 100

#define ENC_F(x) e##x
#define BUT_F(x) b##x


#define BUTT(x) void BUT_F(x)(){\
static unsigned long last_interrupt_time=0;\
unsigned long interrupt_time = millis();\
if(interrupt_time - last_interrupt_time > DEBOUNCE_TIME){\
/*instruments[instrument].velocities[x]=80-instruments[instrument].velocities[x];}*/\
encoders[x].button_changed=true;\
encoders[x].button_state=!encoders[x].button_state;\
}\
last_interrupt_time=interrupt_time;}\

#define ENC(x) void ENC_F(x)(){\
  unsigned char s=(digitalRead(encoders[x].pinA)<<1)|(digitalRead(encoders[x].pinB));\
  encoders[x].raw_state=s;\
  encoders[x].state=ttable[encoders[x].state&0xf][s];\
  encoders[x].state_changed=true;\
}\


typedef enum wrapping_t{
  ROT_CLAMP,
  ROT_CYCLE
}wrapping_t;


typedef struct myencoder_t{
  int pinA;
  int pinB;
  int pinSW;
  int encoder_speed;
  int min;
  int max;
  int value;
  wrapping_t wrapping;
  unsigned char state;
  unsigned char raw_state;
  bool state_changed;
  void  (*rotary_interrupt)(void);
  void  (*button_interrupt)(void);
  char* position_fmt;
  char* button_fmt;
  bool send_osc_on_update;
  bool send_button;
  bool button_changed;
  bool button_state;
  OSCBundle *bundle;
  Encoder *enc;
}myencoder_t;




//TODO: multiple encoder_state() here.
//nim template
//
