#include <LiquidCrystal.h>    //Biblioteca para controlar o display LCD
#include <math.h>             //Biblioteca para funções matematicas
 
// LCD conectado nos pinos digitais 11, 9, 8, 5, 4, 3, 2
LiquidCrystal lcd(10, 9, 8, 5, 4, 3, 2);
const int pinLDR     =A0;
const int pinTMP36   =A1;
const int pinUmidade =A2;
 
const float VCC = 5.0;
const int R_FIXED = 10000;
 
const int ledRed     = 13;
const int ledYellow  = 12;
const int ledGreen   = 11;
const int buzzer     = 6;
 
const bool LedActiveHIGH = true;
const float A =600000.0;
const float B = -1.25;
 
const float minLuminosidade         = 100.0;
const float maxLuminosidade         = 200.0;
const float marginAlertLuminosidade = 20.0;
 
const int minUmidade          = 60;
const int maxUmidade          = 80;
const int marginAlertUmidade = 10;
 
unsigned long tempoAnterior = 0;
const unsigned long intervaloDisplay = 5000;
int estadoDisplay = 0;
 
//Animação da nosso logo  (de baixo para cima)
byte wineCheio[8] = {
  B00011000,
  B00111100,
  B01111110,
  B01111110,
  B00111100,
  B01111100,
  B01111100,
  B11111110
};
 
void printCentralizado(int linha, String texto){
  int col = (16 - texto.length()) / 2;
  if (col < 0) col = 0;
  lcd.setCursor(col, linha);
  lcd.print(texto);
}
float calcularLux(int adc){
  if (adc <= 1) return 0.0;
  if (adc >= 1022) return 1e6;
  float Vout = adc * (VCC / 1023.0);
  float R_LDR = R_FIXED * (VCC / Vout - 1.0);
  if (R_LDR <= 0.0 || !isfinite(R_LDR)) return 1e6;
  float lux = A * pow(R_LDR, B);
  if (!isfinite(lux) || lux < 0.0) lux = 0.0;
  return lux;
}
 
void acenderLed(bool green, bool yellow, bool red) {
  digitalWrite (ledGreen,  LedActiveHIGH ? green  : !green);
  digitalWrite (ledYellow, LedActiveHIGH ? yellow : !yellow);
  digitalWrite (ledRed,    LedActiveHIGH ? red    : !red);
}
 
float lerTemperaturaCelsius(){
  int leituraADC = analogRead(pinTMP36);
  float tensao = leituraADC * (VCC / 1023.0);
  float temperatura = (tensao - 0.5) * 100.00;
  return temperatura;
}
 
int lerUmidadePct(){
  int leituraADC = analogRead(pinUmidade);
  int umidadePct = map(leituraADC, 0, 1023, 0, 100);
  return umidadePct;
}
 
void setup () {
  lcd.begin(16, 2);
  lcd.createChar (0,wineCheio);
  lcd.clear();
  lcd.setCursor(7, 0); lcd.write(byte(0)); delay(500);
  lcd.clear();
   lcd.setCursor(0, 0); lcd.write(byte(2));
  printCentralizado (0, "Bem-vindo(a) a ");
   lcd.setCursor(0, 0); lcd.write(byte(2));
  printCentralizado (0, "Vinheria Agnello!!");
  tempoAnterior = millis();
  estadoDisplay = 0;
  Serial.begin(9600);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  acenderLed(false, false, false);
  digitalWrite(buzzer, LOW);
}
 
void loop (){
  int leituraADC = analogRead(pinLDR);
  float lux = calcularLux(leituraADC);
  int luminosidadePct = map((int)lux, 0, 1000, 0, 100);
  float temperatura = lerTemperaturaCelsius();
  int temperaturaInt = (int) temperatura;
  int umidadePct = lerUmidadePct();
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervaloDisplay){
    tempoAnterior = tempoAtual;
    estadoDisplay++;
    if (estadoDisplay > 2) estadoDisplay = 0;
    lcd.clear();
  }
  //Tela de Temperatura
  if (estadoDisplay == 0) {
    printCentralizado (0, "TEMPERATURA");
    String linha2 = String(temperaturaInt) + (char)223+ "C";
    if (temperaturaInt < 12) linha2 += "BAIXA";
    else if (temperaturaInt > 14) linha2 += "ALTA";
    else linha2 += "IDEAL";
 
    printCentralizado(1, linha2);
  }
  //Tela de Luminosidade
   else if (estadoDisplay == 1) {
    printCentralizado(0, "LUMINOSIDADE");
    String linha2 = String(luminosidadePct) + "% ";
    if (lux < minLuminosidade - marginAlertLuminosidade || lux > maxLuminosidade + marginAlertLuminosidade) {
      linha2 += "ALM";
    }
     else if ((lux >= minLuminosidade - marginAlertLuminosidade && lux < minLuminosidade) ||
             (lux > maxLuminosidade && lux <= maxLuminosidade + marginAlertLuminosidade)) {
      linha2 += "ALT"; 
     }
      else {
      linha2 += "OK";
      }
     printCentralizado(1, linha2);
   }
  //Tela de Umidade
  else if (estadoDisplay == 2) {
    printCentralizado(0, "UMIDADE");
    String linha2 = String(umidadePct) + "% ";
    if (umidadePct < minUmidade - marginAlertUmidade ||
        umidadePct > maxUmidade + marginAlertUmidade) {
      linha2 += "ALM";
    }
    else if ((umidadePct >= minUmidade- marginAlertUmidade && umidadePct < minUmidade) ||
             (umidadePct > maxUmidade && umidadePct <= maxUmidade + marginAlertUmidade)) {
      linha2 += "ALT";
    }
     else {
      linha2 += "OK"; 
     }
    printCentralizado(1, linha2);
  }
  //Alarme (apenas de luminosidade)
  if (lux < minLuminosidade - marginAlertLuminosidade || lux > maxLuminosidade +marginAlertLuminosidade) {
    acenderLed(false, false, true); // vermelho
    digitalWrite(buzzer, LOW);      // buzzer desligado aqui
    delay(500);
    }
  else if ((lux >= minLuminosidade - marginAlertLuminosidade && lux < minLuminosidade) ||
           (lux > maxLuminosidade && lux <= maxLuminosidade + marginAlertLuminosidade)) {
    acenderLed(false, true, false); // amarelo
    digitalWrite(buzzer, HIGH);     // buzzer ligado
    delay(3000);
    digitalWrite(buzzer, LOW);
    delay(3000);
  }
  else {
    acenderLed(true, false, false); // verde
    digitalWrite(buzzer, LOW);
    delay(500);
  }
}