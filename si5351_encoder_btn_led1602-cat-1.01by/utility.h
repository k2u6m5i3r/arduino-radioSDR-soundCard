void showLCDBaner(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("USDX Si5351-ph");
  lcd.setCursor(0, 1);
  lcd.print("1-30MHz");
  delay(1500);
  lcd.setCursor(0, 0);
  lcd.print("              ");
  lcd.setCursor(0, 0);
  lcd.print("UT3MK pcb");
  delay(1500);
}
void freqSetToSi(int32_t freqShow){
  if(freq > 1000000){               // установить частоту только больше 1МГц 
    // put your setup code here, to run once:
    si5351.SendPLLRegisterBulk();
    si5351.freq( freq, 0, rx_ph_q/*90, 0*/ ) ; // RX in USB  
    si5351.reset();
  }  
}
void showLCDFreq(int32_t freqShow){
		byte d1,d2,d3,d4,d5,d6,d7;
	  d7 = int(freqShow / 1000000);
    d6 = ((freqShow / 100000) % 10);
    d5 = ((freqShow / 10000) % 10);
    d4 = ((freqShow / 1000) % 10);
    d3 = ((freqShow / 100) % 10);
    d2 = ((freqShow / 10) % 10);
    d1 = ((freqShow / 1) % 10);
    lcd.setCursor(0, 0);
    lcd.print("fr ");
    lcd.print("     ");
    if (d7 > 9){
      lcd.setCursor(3, 0);
    }
    else {
      lcd.setCursor(4, 0);
    }
    lcd.print(d7);
    lcd.print(".");
    lcd.print(d6);
    lcd.print(d5);
    lcd.print(d4);
    //lcd.print(".");
    lcd.print(d3);
    lcd.print(d2);
    lcd.print(d1);
    lcd.print(" Hz   ");

}
void showLCDStep(int32_t stepShow){
    byte d1,d2,d3,d4,d5,d6,d7;
    d7 = int(stepShow / 1000000);
    d6 = ((stepShow / 100000) % 10);
    d5 = ((stepShow / 10000) % 10);
    d4 = ((stepShow / 1000) % 10);
    d3 = ((stepShow / 100) % 10);
    d2 = ((stepShow / 10) % 10);
    d1 = ((stepShow / 1) % 10);
    lcd.setCursor(0, 1);

    lcd.print("st ");
//    if (d7 > 9) {
//      lcd.setCursor(5, 1);
//    }
//    else {
//      lcd.setCursor(5, 1);
//    }
    lcd.print(d7);
    lcd.print(".");
    lcd.print(d6);
    lcd.print(d5);
    lcd.print(d4);
    lcd.print(".");
    lcd.print(d3);
    lcd.print(d2);
    lcd.print(d1);
    lcd.print(" Hz   ");
}
//void showLCDshiftFreqSi(uint32_t shiftFreq){
void showLCDshiftFreqSi(int shiftFreq){
    lcd.setCursor(0, 0);
    lcd.print("cor.            ");
    lcd.setCursor(5, 0);
    lcd.print(shiftFreq);
    lcd.print(" Hz ");

}
