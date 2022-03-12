#define VERSION   "1.01by"
//Частота кварца Si5351
  //не проверял 27МГц
//#define F_XTAL    27005000    // 27MHz SI5351 crystal
#define F_XTAL    25000000      // 25MHz SI5351 crystal частоат кварца для Si5351, проверенно на частоте 25МГЦ
#define SI5351_ADDR   0x60      // адрес модуля Si5351

// дисплей
#include <LiquidCrystal.h>
// шина i2c
#include "IIC.h"
// кнопки и енкодер
#include "GyverButton.h"
#include "GyverEncoder.h"

I2C i2c;
// библиотека для работы с Si5351
#include "si5151usdx.h"
SI5351 si5351;
// начальная частота при включении
volatile int32_t freq = 7100000;
// сдвиг фазы между СLK0 CLK1
uint8_t rx_ph_q = 90;
// были измениния
volatile bool change = false;
//переменная для коректировки частоты для Si5351
/*volatile */int shiftFreq = 0;

// Дисплей
// Установка дисплея
LiquidCrystal lcd(4, 5, 6, 7, 8, 9); // I used an odd pin combination because I need pin 2 and 3 for the interrupts.
// полезные функции
#include "utility.h"

// Кнопка
  //Меню запаралеленно с кнопкой энкодера и инкодер обрабатывает
//выводы кнопок
#define BTN_MENU 12
#define BTN_STEP_DEC 11 
#define BTN_STEP_INC 10
#define BTN_BAND_LOCK 13 
GButton btnStepDec(BTN_STEP_DEC);
GButton btnStepInc(BTN_STEP_INC);
GButton btnBandLock(BTN_BAND_LOCK);

// Енкодер
#define CLK 2
#define DT 3
#define SW 12
Encoder enc1(CLK, DT, SW);            // для работы c кнопкой
//enc1.setType(TYPE2);                // Тип энкодера не работает TYPE2, поменять на TYPE1. поместить в setup{}

// переменные для работы
// int32_t freq = 1800000;            // описанна выше, используется для установки частоты в модуле Si5351
int32_t stepFreqCurrent = 0;          // шаг измениния  
int32_t stepFreq[13] = {1,5,10,50,100,500, 1000,5000,10000,50000, 100000,500000, 1000000}; // шаги измения частот
int32_t indexStepFreq = 7;            // начальный шаг измениния
int32_t indexMaxStepFreq = 13;
int32_t currentBand = 0;              // текущий диапазон для быстрыз прыжков
int32_t band[11]={1800000,3500000, 7000000, 10100000, 14000000, 18068000, 21000000, 21000000, 24890000, 28000000, 50000000}; // прыжки по диапазонам
int32_t indexMaxBand = 10;
boolean lock = true;                  // блокировка дисплея, показывает * в конце второй строки
boolean inMenu = false;
// библиотека CAT-ft-480
#include "cat-usdx.h"

void setup() {
  // скорость обмена по CAT-протаколу
  Serial.begin(9600);
  //Si5351
  si5351.powerDown();
  // LCD 1602 инициализация
  lcd.begin(16, 2);                    // настройка дисплея
  // показать банер на дисплее
  showLCDBaner();
  showLCDFreq(freq);
  showLCDStep(stepFreq[indexStepFreq]);
  // Указать тип энкодера https://alexgyver.ru/encoder/
  enc1.setType(TYPE2);                // Тип энкодера, не работает TYPE2, поменять на TYPE1
  Command_GETFreqA();                 // Послать частоту на HDsdr
  freqSetToSi(freq+shiftFreq);        // Установить частоту на Si5351
  // Настройка кнопок btnStepDec
  btnStepDec.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  btnStepDec.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  btnStepDec.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  // HIGH_PULL - кнопка подключена к GND, пин подтянут к VCC (BTN_PIN --- КНОПКА --- GND) | LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND ||по умолчанию стоит HIGH_PULL
  btnStepDec.setType(HIGH_PULL);
  // NORM_OPEN - нормально-разомкнутая кнопка | NORM_CLOSE - нормально-замкнутая кнопка || по умолчанию стоит NORM_OPEN
  btnStepDec.setDirection(NORM_OPEN);
	// Настройка кнопок btnStepInc
  btnStepInc.setDebounce(50);        // настройка антидребезга (по умолчанию 80 мс)
  btnStepInc.setTimeout(300);        // настройка таймаута на удержание (по умолчанию 500 мс)
  btnStepInc.setClickTimeout(600);   // настройка таймаута между кликами (по умолчанию 300 мс)
  // HIGH_PULL - кнопка подключена к GND, пин подтянут к VCC (BTN_PIN --- КНОПКА --- GND) | LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND ||по умолчанию стоит HIGH_PULL
  btnStepInc.setType(HIGH_PULL);
  // NORM_OPEN - нормально-разомкнутая кнопка | NORM_CLOSE - нормально-замкнутая кнопка ||  по умолчанию стоит NORM_OPEN
  btnStepInc.setDirection(NORM_OPEN);
	// Настройка кнопок btnBandLock
  btnStepInc.setDebounce(50);         // настройка антидребезга (по умолчанию 80 мс)
  btnStepInc.setTimeout(300);         // настройка таймаута на удержание (по умолчанию 500 мс)
  btnStepInc.setClickTimeout(600);    // настройка таймаута между кликами (по умолчанию 300 мс)
  // HIGH_PULL - кнопка подключена к GND, пин подтянут к VCC (BTN_PIN --- КНОПКА --- GND) | LOW_PULL  - кнопка подключена к VCC, пин подтянут к GND || по умолчанию стоит HIGH_PULL
  btnStepInc.setType(HIGH_PULL);
  // NORM_OPEN - нормально-разомкнутая кнопка | NORM_CLOSE - нормально-замкнутая кнопка || по умолчанию стоит NORM_OPEN
  btnStepInc.setDirection(NORM_OPEN);
}
void loop() {
	  // прерывания от кнопок и про чего
	btnStepDec.tick();                  // обязательная функция отработки. Должна постоянно опрашиваться
	btnStepInc.tick();                  // обязательная функция отработки. Должна постоянно опрашиваться
	btnBandLock.tick();                 // обязательная функция отработки. Должна постоянно опрашиваться
	enc1.tick();                        // обязательная функция отработки. Должна постоянно опрашиваться
  
  // энкодер
  if (enc1.isRight() && lock){              // поворот энкодера вправо
    freq = freq - stepFreq[indexStepFreq];
  	showLCDFreq(freq);
    Command_GETFreqA();
    freqSetToSi(freq + shiftFreq);
  }
  if (enc1.isLeft() && lock){               // поворот энкодера влево
    freq = freq + stepFreq[indexStepFreq];
    showLCDFreq(freq);
    Command_GETFreqA();
    freqSetToSi(freq + shiftFreq);
  }
  if (enc1.isClick() && lock){              // отпускание кнопки (+ дебаунс)
    // при клике енкодера и нажании кнопки меню
    // покажет версию прошивки
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(VERSION);
    lcd.print(" EW8MKU");
    lcd.setCursor(0, 1);
    lcd.print("21.02.2022");
    delay(900);
    showLCDFreq(freq);
    showLCDStep(stepFreq[indexStepFreq]);
  } 
  if (enc1.isRightH() && lock){              // нажатие и поворот вправо
    // при нажатии енкодера и повороте в парво
    Serial.println(shiftFreq);
    shiftFreq += stepFreq[indexStepFreq];
    showLCDshiftFreqSi(shiftFreq);
     
  }
  if (enc1.isLeftH() && lock){              // нажатие и поворот влево
    // при нажатии енкодера и повороте влево
     Serial.println(shiftFreq);
     shiftFreq -= stepFreq[indexStepFreq];
     showLCDshiftFreqSi(shiftFreq);
     
  }
  // кнопка Декрименты
  if (btnStepDec.isClick() && lock){        // проверка на один клик
    if(indexStepFreq > 0){
      indexStepFreq--;
    } else {
      indexStepFreq = 12;
    }
    stepFreqCurrent = stepFreq[indexStepFreq];
     showLCDStep(stepFreq[indexStepFreq]); 
  } 
  if (btnStepDec.isDouble() && lock){       // проверка на двойной клик  
  }
  if (btnStepDec.isTriple() && lock){       // проверка на тройной клик
  }
  // кнопка Инкрименты
  if (btnStepInc.isClick()&& lock){         // проверка на один клик
    if(indexStepFreq < indexMaxStepFreq){
		  indexStepFreq++;
    } else {
		  indexStepFreq = 0;
    }
    stepFreqCurrent = stepFreq[indexStepFreq];
    showLCDStep(stepFreq[indexStepFreq]);
  }  
//  if (btnStepInc.isDouble()&& lock){        // проверка на двойной клик 
//  }
//  if (btnStepInc.isTriple()&& lock){        // проверка на тройной клик
//  }
  // кнопка Банд/Лок
  if (btnBandLock.isClick()){         // проверка на один клик
    if(lock){
      lock  = false;
      lcd.setCursor(15, 1);
      lcd.print("*");
    } else {
      lock  = true;
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }
  } 
  if (btnBandLock.isDouble()&& lock){                // проверка на двойной клик 
    // быстрые прыжки по бандам
    if(currentBand <= (indexMaxBand)){
      freq = band[currentBand];
      showLCDFreq(freq);
      Command_GETFreqA();
      freqSetToSi(freq+shiftFreq);
    } else {
      currentBand = 0;
    }
    currentBand++;
  }
//  if (btnBandLock.isTriple()&& lock){           // проверка на тройной клик    
//  } 
  serialEvent();
  if(change == true){
    //lcd.setCursor(0, 1);
    //lcd.print("*");
    freqSetToSi(freq+shiftFreq);
    showLCDFreq(freq);
    change = false;
    lcd.setCursor(0, 1);
    lcd.print(" ");
  }
}
