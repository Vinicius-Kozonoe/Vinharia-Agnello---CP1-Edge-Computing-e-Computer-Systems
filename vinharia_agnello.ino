/*	
  Projeto: Vinheria Agnello — Monitoramento Ambiental
  Grupo:
Gabriel Amparo Evangelista Santo Rm: 568274
César Andre Zanin Filho        Rm: 567615
Murilo Jeronimo Ferreira Nunes  Rm:560641
Pedro Olavo Jung da Silva        Rm: 567182
Vinicius Kozonoe Guaglini       Rm: 567264
    
*/

// Inclusão das bibliotecas necessárias
#include <LiquidCrystal.h>    // Biblioteca para controlar o display LCD
#include <math.h>             // Biblioteca para cálculos matemáticos

// Definição dos pinos do display LCD (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(10, 9, 8, 5, 4, 3, 2);

// Declaração dos pinos dos sensores
const int pinLDR     = A0;  // Sensor de luminosidade (LDR)
const int pinTMP36   = A1;  // Sensor de temperatura (TMP36)
const int pinUmidade = A2;  // Sensor de umidade (simulado por um potenciômetro)

// Constantes para cálculos
const float VCC = 5.0;         // Tensão de alimentação (5V)
const int R_FIXED = 10000;     // Resistor fixo do divisor de tensão do LDR (10kΩ)

// Pinos dos LEDs e buzzer
const int ledRed     = 13;
const int ledYellow  = 12;
const int ledGreen   = 11;
const int buzzer     = 6;

// Configuração do tipo de LED (ativo em nível alto)
const bool LedActiveHIGH = true;

// Constantes para o cálculo de luminosidade (ajustadas para o sensor)
const float A = 600000.0;
const float B = -1.25;

// Limites de luminosidade (lux)
const float minLuminosidade         = 100.0;
const float maxLuminosidade         = 200.0;
const float marginAlertLuminosidade = 20.0;

// Limites de umidade (%)
const int minUmidade          = 60;
const int maxUmidade          = 80;
const int marginAlertUmidade  = 10;

// Controle de tempo para alternar telas
unsigned long tempoAnterior = 0;
const unsigned long intervaloDisplay = 5000; // alterna a cada 5 segundos
int estadoDisplay = 0; // controla qual tela está sendo mostrada

// Animação personalizada (taça de vinho) — byte para o LCD
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

// Função para imprimir texto centralizado no LCD
void printCentralizado(int linha, String texto){
  int col = (16 - texto.length()) / 2; // calcula a posição central
  if (col < 0) col = 0;
  lcd.setCursor(col, linha);           // posiciona o cursor
  lcd.print(texto);                    // imprime o texto
}

// Função para calcular lux a partir da leitura analógica do LDR
float calcularLux(int adc){
  if (adc <= 1) return 0.0;            // evita divisão por zero
  if (adc >= 1022) return 1e6;         // valor máximo de saturação
  float Vout = adc * (VCC / 1023.0);   // converte leitura em tensão
  float R_LDR = R_FIXED * (VCC / Vout - 1.0); // calcula resistência do LDR
  if (R_LDR <= 0.0 || !isfinite(R_LDR)) return 1e6;
  float lux = A * pow(R_LDR, B);       // aplica a equação empírica
  if (!isfinite(lux) || lux < 0.0) lux = 0.0;
  return lux;                          // retorna valor em lux
}

// Função para acender os LEDs conforme necessidade
void acenderLed(bool green, bool yellow, bool red) {
  digitalWrite(ledGreen,  LedActiveHIGH ? green  : !green);
  digitalWrite(ledYellow, LedActiveHIGH ? yellow : !yellow);
  digitalWrite(ledRed,    LedActiveHIGH ? red    : !red);
}

// Função para ler temperatura em Celsius (sensor TMP36)
float lerTemperaturaCelsius(){
  int leituraADC = analogRead(pinTMP36);
  float tensao = leituraADC * (VCC / 1023.0);     // converte para tensão
  float temperatura = (tensao - 0.5) * 100.00;    // fórmula do TMP36
  return temperatura;
}

// Função para ler umidade (simulada por potenciômetro)
int lerUmidadePct(){
  int leituraADC = analogRead(pinUmidade);
  int umidadePct = map(leituraADC, 0, 1023, 0, 100); // converte para porcentagem
  return umidadePct;
}

// Configuração inicial
void setup () {
  lcd.begin(16, 2);           // inicializa o LCD 16x2
  lcd.createChar(0, wineCheio); // cria o caractere personalizado
  lcd.clear();

  // Mostra o logo inicial com a taça
  lcd.setCursor(7, 0); lcd.write(byte(0)); delay(500);
  lcd.clear();
  printCentralizado(0, "Bem-vindo(a) a ");
  printCentralizado(1, "Vinheria Agnello!!");

  tempoAnterior = millis();  // registra o tempo inicial
  estadoDisplay = 0;

  Serial.begin(9600);        // inicializa comunicação serial (debug)
  
  // Define os pinos como saída
  pinMode(ledGreen, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(buzzer, OUTPUT);

  acenderLed(false, false, false); // todos LEDs apagados
  digitalWrite(buzzer, LOW);       // buzzer desligado
}

// Loop principal
void loop (){
  // Leitura dos sensores
  int leituraADC = analogRead(pinLDR);
  float lux = calcularLux(leituraADC);               // luminosidade em lux
  int luminosidadePct = map((int)lux, 0, 1000, 0, 100); // converte para %
  float temperatura = lerTemperaturaCelsius();       // temperatura em °C
  int temperaturaInt = (int) temperatura;            // arredonda
  int umidadePct = lerUmidadePct();                  // umidade em %

  // Alternância das telas a cada intervalo
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervaloDisplay){
    tempoAnterior = tempoAtual;
    estadoDisplay++;                 // passa para a próxima tela
    if (estadoDisplay > 2) estadoDisplay = 0;
    lcd.clear();
  }

  // ======= TELA 1: TEMPERATURA =======
  if (estadoDisplay == 0) {
    printCentralizado(0, "TEMPERATURA");
    String linha2 = String(temperaturaInt) + (char)223 + "C "; // símbolo de grau
    // Verifica se está fora da faixa ideal
    if (temperaturaInt < 12) linha2 += "BAIXA";
    else if (temperaturaInt > 14) linha2 += "ALTA";
    else linha2 += "IDEAL";
    printCentralizado(1, linha2);
  }

  // ======= TELA 2: LUMINOSIDADE =======
  else if (estadoDisplay == 1) {
    printCentralizado(0, "LUMINOSIDADE");
    String linha2 = String(luminosidadePct) + "% ";
    // Verifica faixas de alerta e ideal
    if (lux < minLuminosidade - marginAlertLuminosidade || lux > maxLuminosidade + marginAlertLuminosidade) {
      linha2 += "ALM"; // alerta máximo
    }
    else if ((lux >= minLuminosidade - marginAlertLuminosidade && lux < minLuminosidade) ||
             (lux > maxLuminosidade && lux <= maxLuminosidade + marginAlertLuminosidade)) {
      linha2 += "ALT"; // atenção
    }
    else {
      linha2 += "OK";  // ideal
    }
    printCentralizado(1, linha2);
  }

  // ======= TELA 3: UMIDADE =======
  else if (estadoDisplay == 2) {
    printCentralizado(0, "UMIDADE");
    String linha2 = String(umidadePct) + "% ";
    if (umidadePct < minUmidade - marginAlertUmidade ||
        umidadePct > maxUmidade + marginAlertUmidade) {
      linha2 += "ALM";
    }
    else if ((umidadePct >= minUmidade - marginAlertUmidade && umidadePct < minUmidade) ||
             (umidadePct > maxUmidade && umidadePct <= maxUmidade + marginAlertUmidade)) {
      linha2 += "ALT";
    }
    else {
      linha2 += "OK"; 
    }
    printCentralizado(1, linha2);
  }

  // ======= SISTEMA DE ALERTAS VISUAIS E SONOROS =======
  if (lux < minLuminosidade - marginAlertLuminosidade || lux > maxLuminosidade + marginAlertLuminosidade) {
    acenderLed(false, false, true); // vermelho = fora da faixa
    digitalWrite(buzzer, LOW);      // buzzer desligado
    delay(500);
  }
  else if ((lux >= minLuminosidade - marginAlertLuminosidade && lux < minLuminosidade) ||
           (lux > maxLuminosidade && lux <= maxLuminosidade + marginAlertLuminosidade)) {
    acenderLed(false, true, false); // amarelo = atenção
    digitalWrite(buzzer, HIGH);     // buzzer ligado
    delay(3000);
    digitalWrite(buzzer, LOW);
    delay(3000);
  }
  else {
    acenderLed(true, false, false); // verde = tudo ideal
    digitalWrite(buzzer, LOW);
    delay(500);
  }
}
