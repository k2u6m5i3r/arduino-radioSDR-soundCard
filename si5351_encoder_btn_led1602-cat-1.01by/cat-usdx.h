// взял с проекта USDX, протокол CAT для ts480, используется в HDSDR 
// не все функции используются в этом приёмнике
#define CATCMD_SIZE   32
#define CAT              1   // CAT-interface
//#define CAT_EXT        1   // Extended CAT support: remote button and screen control commands over CAT
//#define CAT_STREAMING  1   // Extended CAT support: audio streaming over CAT, once enabled and triggered with CAT cmd, samplerate 7812Hz, 8-bit unsigned audio is sent over UART. The ";" is omited in the data-stream, and only sent to indicate the beginning and end of a CAT cmd.

// CAT support inspired by Charlie Morris, ZL2CTM, contribution by Alex, PE1EVX, source: http://zl2ctm.blogspot.com/2020/06/digital-modes-transceiver.html?m=1
// https://www.kenwood.com/i/products/info/amateur/ts_480/pdf/ts_480_pc.pd


//volatile uint8_t cat_key = 0;
//volatile uint8_t cat_active = 0;
//volatile uint8_t vox = 0;
//volatile uint8_t cat_streaming = 0;
//volatile uint8_t _cat_streaming = 0;

volatile uint32_t rxend_event = 0;

void serialEvent();
void Command_UK(char k1, char k2);
void Command_UD();
void Command_UA(char en);
void Command_GETFreqA();
void Command_SETFreqA();
void Command_IF();
void Command_ID();
void Command_PS();
void Command_PS1();
void Command_AI();
void Command_AI0();
void Command_GetMD();
void Command_SetMD();
void Command_RX();
void Command_AG0();
void Command_XT1();
void Command_RT1();
void Command_TX0();
void Command_TX1();
void Command_TX2();
void Command_RC();
void Command_FL0();
void Command_RS();
void Command_VX(char mode);

char CATcmd[CATCMD_SIZE];

void analyseCATcmd()
{
  if((CATcmd[0] == 'F') && (CATcmd[1] == 'A') && (CATcmd[2] == ';'))
    Command_GETFreqA();
  else if((CATcmd[0] == 'F') && (CATcmd[1] == 'A') && (CATcmd[13] == ';'))
    Command_SETFreqA();
  else if((CATcmd[0] == 'I') && (CATcmd[1] == 'F') && (CATcmd[2] == ';'))
    Command_IF();
  else if((CATcmd[0] == 'I') && (CATcmd[1] == 'D') && (CATcmd[2] == ';'))
    Command_ID();
  else if((CATcmd[0] == 'P') && (CATcmd[1] == 'S') && (CATcmd[2] == ';'))
    Command_PS();
  else if((CATcmd[0] == 'P') && (CATcmd[1] == 'S') && (CATcmd[2] == '1'))
    Command_PS1();
  else if((CATcmd[0] == 'A') && (CATcmd[1] == 'I') && (CATcmd[2] == ';'))
    Command_AI();
  else if((CATcmd[0] == 'A') && (CATcmd[1] == 'I') && (CATcmd[2] == '0'))
    Command_AI0();
  else if((CATcmd[0] == 'M') && (CATcmd[1] == 'D') && (CATcmd[2] == ';'))
    Command_GetMD();
  else if((CATcmd[0] == 'M') && (CATcmd[1] == 'D') && (CATcmd[3] == ';'))
    Command_SetMD();
  else if((CATcmd[0] == 'R') && (CATcmd[1] == 'X') && (CATcmd[2] == ';'))
    Command_RX();
  else if((CATcmd[0] == 'T') && (CATcmd[1] == 'X') && (CATcmd[2] == ';'))
    Command_TX0();
  else if((CATcmd[0] == 'T') && (CATcmd[1] == 'X') && (CATcmd[2] == '0'))
    Command_TX0();
  else if((CATcmd[0] == 'T') && (CATcmd[1] == 'X') && (CATcmd[2] == '1'))
    Command_TX1();
  else if((CATcmd[0] == 'T') && (CATcmd[1] == 'X') && (CATcmd[2] == '2'))
    Command_TX2();
  else if((CATcmd[0] == 'A') && (CATcmd[1] == 'G') && (CATcmd[2] == '0'))  // add
    Command_AG0();
  else if((CATcmd[0] == 'X') && (CATcmd[1] == 'T') && (CATcmd[2] == '1'))  // add
    Command_XT1();
  else if((CATcmd[0] == 'R') && (CATcmd[1] == 'T') && (CATcmd[2] == '1'))  // add
    Command_RT1();
  else if((CATcmd[0] == 'R') && (CATcmd[1] == 'C') && (CATcmd[2] == ';'))  // add
    Command_RC();
  else if((CATcmd[0] == 'F') && (CATcmd[1] == 'L') && (CATcmd[2] == '0'))  // need?
    Command_FL0();
  else if((CATcmd[0] == 'R') && (CATcmd[1] == 'S') && (CATcmd[2] == ';'))
    Command_RS();
  else if((CATcmd[0] == 'V') && (CATcmd[1] == 'X') && (CATcmd[2] != ';'))
    Command_VX(CATcmd[2]);
  else if((CATcmd[0] == 'U') && (CATcmd[1] == 'K') && (CATcmd[4] == ';'))  // remote key press
    Command_UK(CATcmd[2], CATcmd[3]);
  else if((CATcmd[0] == 'U') && (CATcmd[1] == 'D') && (CATcmd[2] == ';'))  // display contents
    Command_UD();
  else if((CATcmd[0] == 'U') && (CATcmd[1] == 'A') && (CATcmd[3] == ';'))  // audio streaming enable/disable
    Command_UA(CATcmd[2]);
  else {
    Serial.print("?;");
  }
}


volatile uint8_t cat_ptr = 0;
void serialEvent(){
  if(Serial.available()){
    rxend_event = millis() + 10;  // block display until this moment, to prevent CAT cmds that initiate display changes to interfere with the next CAT cmd e.g. Hamlib: FA00007071000;ID;
    char data = Serial.read();
    CATcmd[cat_ptr++] = data;
    if(data == ';'){
      CATcmd[cat_ptr] = '\0'; // terminate the array
      cat_ptr = 0;            // reset for next CAT command
      analyseCATcmd();
      delay(10);
    } else if(cat_ptr > (CATCMD_SIZE - 1)){ Serial.print("E;"); cat_ptr = 0; } // overrun
  }
}


void Command_UK(char k1, char k2){
//  cat_key = ((k1 - '0') << 4) | (k2 - '0');
//  if(cat_key & 0x40){ encoder_val--; cat_key &= 0x3f; }
//  if(cat_key & 0x80){ encoder_val++; cat_key &= 0x3f; }
//  char Catbuffer[16];
//  sprintf(Catbuffer, "UK%c%c;", k1, k2);
//  Serial.print(Catbuffer);
}

void Command_UD(){
//  char Catbuffer[40];
//  sprintf(Catbuffer, "UD%02u%s;", (lcd.curs) ? lcd.y*16+lcd.x : 16*2+1, lcd.text);
//  Serial.print(Catbuffer);
}

void Command_UA(char en){
//  char Catbuffer[16];
//  sprintf(Catbuffer, "UA%01u;", (_cat_streaming = (en == '1')));
//  Serial.print(Catbuffer);
//  if(_cat_streaming){ Serial.print("US"); cat_streaming = true; }
}

void Command_GETFreqA(){
  char Catbuffer[32];
  unsigned int g,m,k,h;
  uint32_t tf;

  tf=freq;
  
  g=(unsigned int)(tf/1000000000lu);
  tf-=g*1000000000lu;
  m=(unsigned int)(tf/1000000lu);
  tf-=m*1000000lu;
  k=(unsigned int)(tf/1000lu);
  tf-=k*1000lu;
  h=(unsigned int)tf;

  sprintf(Catbuffer,"FA%02u%03u",g,m);
  Serial.print(Catbuffer);
  sprintf(Catbuffer,"%03u%03u;",k,h);
  Serial.print(Catbuffer);
}

void Command_SETFreqA(){
  char Catbuffer[16];
  strncpy(Catbuffer,CATcmd+2,11);
  Catbuffer[11]='\0';

  freq=(uint32_t)atol(Catbuffer);
  change=true;

}

void Command_IF(){
//  char Catbuffer[32];
//  unsigned int g,m,k,h;
//  uint32_t tf=0;
//	uint8_t mode =0;
//  tf=freq;
//  g=(unsigned int)(tf/1000000000lu);
//  tf-=g*1000000000lu;
//  m=(unsigned int)(tf/1000000lu);
//  tf-=m*1000000lu;
//  k=(unsigned int)(tf/1000lu);
//  tf-=k*1000lu;
//  h=(unsigned int)tf;
//
//  sprintf(Catbuffer,"IF%02u%03u%03u%03u",g,m,k,h);
//  Serial.print(Catbuffer);
//  sprintf(Catbuffer,"00000+000000");
//  Serial.print(Catbuffer);
//  sprintf(Catbuffer,"0000");
//  Serial.print(Catbuffer);
//  Serial.print(mode + 1);
//  sprintf(Catbuffer,"0000000;");
//  Serial.print(Catbuffer);
}
void Command_AI(){
//  Serial.print("AI0;");
}

void Command_AG0(){
//  Serial.print("AG0;");
}

void Command_XT1(){
//  Serial.print("XT1;");
}

void Command_RT1(){
//  Serial.print("RT1;");
}
void Command_RC(){
//  rit = 0;
//  Serial.print("RC;");
}
void Command_FL0(){
//  Serial.print("FL0;");
}
void Command_GetMD(){
//	uint8_t mode =0;
//  Serial.print("MD");
//  Serial.print(mode + 1);
//  Serial.print(';');
}
void Command_SetMD(){
//  mode = CATcmd[2] - '1';
//
//  vfomode[vfosel%2] = mode;
//  change = true;
//  si5351.iqmsa = 0;  // enforce PLL reset
}
void Command_AI0(){
//  Serial.print("AI0;");
}
void Command_RX(){
// #ifdef TX_ENABLE
  // switch_rxtx(0);
  // semi_qsk_timeout = 0;  // hack: fix for multiple RX cmds
// #endif
  // Serial.print("RX0;");
}
void Command_TX0(){
// #ifdef TX_ENABLE
  // switch_rxtx(1);
// #endif
}
void Command_TX1(){
// #ifdef TX_ENABLE
  // switch_rxtx(1);
// #endif
}
void Command_TX2(){
// #ifdef TX_ENABLE
  // switch_rxtx(1);
// #endif
}
void Command_RS(){
  Serial.print("RS0;");
}
void Command_VX(char mode){
//  char Catbuffer[16];
//  sprintf(Catbuffer, "VX%c;",mode);
//  Serial.print(Catbuffer);
}
void Command_ID(){
  Serial.print("ID020;");
}
void Command_PS(){
  Serial.print("PS1;");
}
void Command_PS1(){
}
