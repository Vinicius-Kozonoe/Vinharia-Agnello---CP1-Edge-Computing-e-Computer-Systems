// Biblioteca para usar o display LCD
#include <LiquidCrystal.h>

// Pinos de conexão do display LCD
LiquidCrystal lcd(8, 7, 6, 5, 4, 3, 2);

// Pinos dos sensores
const int pinTemp = A1; // Sensor de temperatura (TMP36 analógico)
const int pinUmid = A2; // Sensor de umidade analógico
const int pinLDR  = A0; // Sensor de luminosidade (LDR)

// LEDs indicadores
const int ledGreen   = 11; // LED verde → condições normais
const int ledYellow  = 12; // LED amarelo → situação de alerta
const int ledRed     = 13; // LED vermelho → situação crítica

// Buzzer de alerta
const int buzzer = 10; // Saída do buzzer sonoro

// Limites de luminosidade (%)
const int lumMin    = 40;
const int lumMax    = 80;
const int margemLum = 10; // Margem para zona de alerta

// Limites de umidade (%)
const int umidMin    = 60;
const int umidMax    = 80;
const int margemUmid = 10; // Margem para zona de alerta

// Limites de temperatura (°C)
const float tempMin = 12.0;
const float tempMax = 15.0;

// Controle do buzzer
unsigned long ultimaAtivacaoBuzzer = 0;
const unsigned long intervaloBuzzer = 3000; // 3 segundos

// Caracteres personalizados para a garrafa e taça
byte garrafa1[8]    = {B00000, B01111, B01000, B01000, B01000, B01111, B00000, B00000}; 
byte garrafa2[8]    = {B00000, B11111, B11100, B11100, B11100, B11111, B00000, B00000}; 
byte garrafa3[8]    = {B00000, B11110, B00011, B00001, B00011, B11110, B00000, B00000}; 
byte garrafa4[8]    = {B00000, B00000, B00000, B11000, B00000, B00000, B00000, B00000}; 
byte gota[8]        = {B00000, B00000, B00000, B11000, B00100, B00100, B01110, B00100}; 
byte tacaVazia[8]   = {B10001, B10001, B01010, B00100, B00100, B00100, B11111, B00000};  
byte tacaMetade[8]  = {B10001, B11111, B01110, B00100, B00100, B00100, B11111, B00000}; 
byte tacaCheia[8]   = {B11111, B11111, B01110, B00100, B00100, B00100, B11111, B00000}; 

// Caracteres personalizados para o termômetro
byte termometroBaixo[8] = {B01110, B01010, B01010, B01010, B01010, B01010, B11111, B01110}; 
byte termometroMedio[8] = {B01110, B01010, B01010, B01010, B01010, B11111, B11111, B01110}; 
byte termometroAlto[8]  = {B01110, B01010, B01010, B11111, B11111, B11111, B11111, B01110}; 

void setup() {
  lcd.begin(16, 2); // Inicia o display LCD (16 colunas e 2 linhas)

  // Registra os caracteres personalizados no LCD
  lcd.createChar(0, garrafa1);
  lcd.createChar(1, garrafa2);
  lcd.createChar(2, garrafa3);
  lcd.createChar(3, garrafa4);
  lcd.createChar(4, gota);
  lcd.createChar(5, tacaVazia);
  lcd.createChar(6, tacaMetade);
  lcd.createChar(7, tacaCheia);

  // Usa os slots extras do LCD para o termômetro
  lcd.createChar(8, termometroBaixo);
  lcd.createChar(9, termometroMedio);
  lcd.createChar(10, termometroAlto);

  // Animação inicial da garrafa enchendo a taça junto com o termômetro
  lcd.clear(); 
  lcd.setCursor(5,0); lcd.write(byte(0));
  lcd.setCursor(6,0); lcd.write(byte(1));
  lcd.setCursor(7,0); lcd.write(byte(2));
  lcd.setCursor(8,0); lcd.write(byte(3));
  lcd.setCursor(8,1); lcd.write(byte(5)); // Taça vazia
  lcd.setCursor(10,0); lcd.write(byte(8)); // Termômetro baixo
  delay(500);

  lcd.clear();
  lcd.setCursor(5,0); lcd.write(byte(0));
  lcd.setCursor(6,0); lcd.write(byte(1));
  lcd.setCursor(7,0); lcd.write(byte(2));
  lcd.setCursor(8,0); lcd.write(byte(4));
  lcd.setCursor(8,1); lcd.write(byte(6)); // Taça meio
  lcd.setCursor(10,0); lcd.write(byte(9)); // Termômetro médio
  delay(500);

  lcd.clear();
  lcd.setCursor(5,0); lcd.write(byte(0));
  lcd.setCursor(6,0); lcd.write(byte(1));
  lcd.setCursor(7,0); lcd.write(byte(2));
  lcd.setCursor(8,0); lcd.write(byte(4));
  lcd.setCursor(8,1); lcd.write(byte(7)); // Taça cheia
  lcd.setCursor(10,0); lcd.write(byte(10)); // Termômetro alto
  delay(500);

  // Mensagem de boas-vindas
  lcd.setCursor(0, 0); 
  lcd.print(" BEM-VINDO(a)!  "); 
  lcd.setCursor(0, 1); 
  lcd.print("Vinheria Agnello"); 
  delay(5000);

  // Configura LEDs e buzzer como saída
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Garante que tudo inicie desligado
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledYellow, LOW);
  digitalWrite(ledRed, LOW);
  digitalWrite(buzzer, LOW);
}

void loop() {
  // Faz a média de 10 leituras para suavizar valores
  long somaTemp = 0, somaUmid = 0, somaLum = 0;
  for (int i = 0; i < 10; i++) {
    somaTemp += analogRead(pinTemp);
    somaUmid += analogRead(pinUmid);
    somaLum  += analogRead(pinLDR);
    delay(10);
  }

  int leituraTemp = somaTemp / 10;
  int leituraUmid = somaUmid / 10;
  int leituraLDR  = somaLum / 10;

  // Converte temperatura
  float tensao = leituraTemp * 5 / 1023.0;
  float temperatura = (tensao - 0.5) * 100.0;

  // Converte umidade e luminosidade
  float umidade = map(leituraUmid, 0, 1023, 0, 100);
  float luminosidade = map(leituraLDR, 0, 1023, 0, 100);

  // Status da temperatura
  String statusTemp;
  if (temperatura < tempMin) statusTemp = "Baixa"; 
  else if (temperatura > tempMax) statusTemp = "Alta"; 
  else statusTemp = "OK"; 

  // Status da umidade
  String statusUmid;
  if (umidade < (umidMin - margemUmid) || umidade > (umidMax + margemUmid))
    statusUmid = "Critica"; 
  else if ((umidade >= (umidMin - margemUmid) && umidade < umidMin) ||
           (umidade > umidMax && umidade <= (umidMax + margemUmid)))
    statusUmid = "Alerta"; 
  else
    statusUmid = "OK"; 

  // Status da luminosidade
  String statusLum;
  if (luminosidade < (lumMin - margemLum) || luminosidade > (lumMax + margemLum))
    statusLum = "Critica"; 
  else if ((luminosidade >= (lumMin - margemLum) && luminosidade < lumMin) ||
           (luminosidade > lumMax && luminosidade <= (lumMax + margemLum)))
    statusLum = "Alerta"; 
  else
    statusLum = "OK"; 

  // LEDs → só um ativo
  digitalWrite(ledGreen, statusLum == "OK");
  digitalWrite(ledYellow, statusLum == "Alerta");
  digitalWrite(ledRed, statusLum == "Critica");

  // Controle do buzzer: toca 3s e espera 3s se alerta/crítico
  if (statusUmid != "OK" || statusLum != "OK") {
    unsigned long agora = millis();
    if (agora - ultimaAtivacaoBuzzer >= intervaloBuzzer * 2) {
      ultimaAtivacaoBuzzer = agora;
    }
    if (agora - ultimaAtivacaoBuzzer < intervaloBuzzer) {
      tone(buzzer, 1000);
    } else {
      noTone(buzzer);
    }
  } else {
    noTone(buzzer);
  }

  // Exibe temperatura
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperatura: ");
  lcd.print(temperatura, 1);
  lcd.print((char)223); 
  lcd.print("C");
  lcd.setCursor(0, 1);
  lcd.print("Status: ");
  lcd.print(statusTemp);
  delay(5000);

  // Exibe umidade
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Umid.: ");
  lcd.print(umidade, 0);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Status: ");
  lcd.print(statusUmid);
  delay(5000);

  // Exibe luminosidade
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Luz: ");
  lcd.print(luminosidade, 0);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Status: ");
  lcd.print(statusLum);
  delay(5000);
}