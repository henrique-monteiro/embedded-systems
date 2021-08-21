#include <Thermistor.h>
#include <LiquidCrystal.h> // Define a biblioteca para utilização das funções referentes ao display LCD.
LiquidCrystal lcd(12, 11, 53, 52, 51, 50);  // Define as GPIO para o envio dos caracteres.
Thermistor temp(2);

const int pinoVerde1     = 2;
const int pinoAmarelo1   = 3;
const int pinoVermelho1  = 4;
const int pinoVerde2     = 5;
const int pinoVermelho2  = 6;
const int pinoVerde3     = 7;
const int pinoAmarelo3   = 8;
const int pinoVermelho3  = 9;
const int pinoBotao       = 22;
const int pinoPotenciometro = A0;
const int pinoReflexivo = 13;
const int pinoLDR = A1;
const int pinoRele = 23;
const int pinoBuzzer = 24;

const int a = 34;
const int b = 36;
const int c = 38;
const int d = 40;

int pwmLDR;
int estadoRele = 1;
long millisLuzPedestre = 0;
int estadoBuzzer = 0;
int timePiscaBuzzer = 500;

const int tempoLedAmarelo     = 2000;
const int tempoEspera      = 1000;
const int tempoLedVerdePedestre     = 6000;
const int tempoLedVerdePedestreFim = 2000;
int tempoCiclo = 0;
int tempoAtraso = 0;

int flagPedestre = 0;
int flagBotao = 0;
int flagClear = 0;
int flagClearSemaforoPrincipal = 0;
int flagFechamentoSinalPedestre = 0;

int flagSinal = 0;

int flagReferencia2 = 0;

long millis_referencia     = 0;
long millis_referencia2 = 0;
long millisAtual;
long millisTemperatura = 5000;


int temperatura;

void setup()
{
  pinMode(pinoVerde1,    OUTPUT);
  pinMode(pinoAmarelo1,  OUTPUT);
  pinMode(pinoVermelho1, OUTPUT);
  pinMode(pinoVerde2,    OUTPUT);
  pinMode(pinoVermelho2, OUTPUT);
  pinMode(pinoVerde3,    OUTPUT);
  pinMode(pinoAmarelo3,  OUTPUT);
  pinMode(pinoVermelho3, OUTPUT);
  pinMode(pinoBotao,      INPUT);
  Serial.begin(9600);
  pinMode(pinoReflexivo, INPUT);
  lcd.begin(16, 2);
  pinMode (pinoRele, OUTPUT);
  pinMode (pinoBuzzer, OUTPUT);
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
}

void loop()
{
  pwmLDR = map(analogRead(pinoLDR), 0, 1023, 10, 255);
  Serial.println(pwmLDR);
  Serial.println(digitalRead(pinoReflexivo));
  //Serial.print("pinoPotenciometro = ");
  //Serial.println(analogRead(pinoPotenciometro));
  if (eventoOcorrido())
    semaforoSecundario();
  else
    semaforoPrincipal();
}

int eventoOcorrido()
{
  /*
     Este if garante que apertar o botão mais de uma vez não irá interferir
     Além disso, ele já salva nas variáveis globais os valores utilizados nos calculos para trocar o sinal
  */
  if ( (millis_referencia == 0 ) && (digitalRead(pinoBotao) || !digitalRead(pinoReflexivo)  ))
  {
    millis_referencia = millis();
    tempoAtraso = calculoTempoAtraso();
    tempoCiclo = calculoTempoCiclo();
    if (digitalRead(pinoBotao) && pwmLDR < 200) {
      estadoRele = 0;
      estadoBuzzer = 1;
    }
  }

  if ( (millis_referencia  > 0 ) && ( millis() < (millis_referencia + tempoCiclo) ) ) {
    digitalWrite(pinoRele, estadoRele);
    return true;
  }
  estadoRele = 1;
  digitalWrite(pinoRele, estadoRele);

  millis_referencia = 0;

  return false;
}

void piscaLed(int pin, int timeOn, int timeOff, int atraso, long millis_referencia = 0)
{

  long ajuste_referencia = millis_referencia % (timeOn + timeOff);
  long resto             = (millis() + timeOn + timeOff - ajuste_referencia - atraso) % (timeOn + timeOff);
  if (resto < timeOn)
    analogWrite(pin, pwmLDR);
  else
    digitalWrite(pin, LOW);
  //digitalWrite(pin, resto < timeOn ? HIGH : LOW);
}

int verificaTime(int time1, int time2, int atraso, long millis_referencia = 0)
{
  long ajuste_referencia = millis_referencia % (time1 + time2);
  long resto             = (millis() + time1 + time2 - ajuste_referencia - atraso) % (time1 + time2);
  return  (resto < time1 ? 1 : 2 );
}

void semaforoPrincipal()
{
  /*
     Esse if limpa o LCD logo depois que o sinal da via principal volta a abrir
  */
  if (flagClearSemaforoPrincipal == 0) {
    lcd.clear();
    flagClearSemaforoPrincipal = 1;
  }

  analogWrite(pinoVerde1,    pwmLDR);
  digitalWrite(pinoAmarelo1,  LOW);
  digitalWrite(pinoVermelho1, LOW);
  digitalWrite(pinoVerde2,    LOW);
  analogWrite(pinoVermelho2, pwmLDR);
  digitalWrite(pinoVerde3,    LOW);
  digitalWrite(pinoAmarelo3,  LOW);
  analogWrite(pinoVermelho3, pwmLDR);
  temperaturaLCD();
  digitalWrite(pinoBuzzer, LOW);
  digitalWrite(a, HIGH);//DIGITO 0
  digitalWrite(b, HIGH);
  digitalWrite(c, HIGH);
  digitalWrite(d, HIGH);

}

void semaforoSecundario()
{
  /*
     O if chama a função responsável pela contagem que o pedestre deve esperar para atravessa a avenida
     O else if chama a função responsável pela contagem que o pedestre terá para atravessar a rua
  */
  if ((millis() - millis_referencia) <= (tempoAtraso + tempoLedAmarelo + tempoEspera))
    msgAbrirSinal();
  else if (((millis() - millis_referencia) > (tempoAtraso + tempoLedAmarelo + tempoEspera)) && ((millis() - millis_referencia) <= (tempoAtraso + tempoLedAmarelo + tempoEspera + tempoLedVerdePedestre)))
    msgFecharSinal();
  else
    msgAlertaSinal();

  piscaLed(pinoVerde1, tempoAtraso, tempoCiclo - tempoAtraso, 0, millis_referencia );
  piscaLed(pinoAmarelo1, tempoLedAmarelo, tempoCiclo - tempoLedAmarelo, tempoAtraso, millis_referencia );
  piscaLed(pinoVermelho1, tempoEspera * 2 + tempoLedVerdePedestre + tempoLedVerdePedestreFim, tempoAtraso + tempoLedAmarelo, tempoAtraso + tempoLedAmarelo, millis_referencia);


  piscaLed(pinoVerde3, tempoLedVerdePedestre, tempoCiclo - tempoLedVerdePedestre, tempoAtraso + tempoLedAmarelo + tempoEspera, millis_referencia);
  piscaLed(pinoAmarelo3, tempoLedAmarelo, tempoCiclo - tempoLedAmarelo, tempoAtraso + tempoLedAmarelo + tempoEspera + tempoLedVerdePedestre, millis_referencia);
  piscaLed(pinoVermelho3, tempoCiclo - tempoLedVerdePedestre - tempoLedVerdePedestreFim - 1000, tempoLedVerdePedestre + tempoLedAmarelo, 0, millis_referencia);

  int t;
  t = verificaTime(tempoCiclo - tempoLedVerdePedestreFim, tempoLedVerdePedestreFim, tempoCiclo - tempoEspera, millis_referencia);

  if (t == 1)
    piscaLed(pinoVerde2, tempoLedVerdePedestre, tempoCiclo - tempoLedVerdePedestre, tempoAtraso + tempoLedAmarelo + tempoEspera, millis_referencia);

  else
    piscaLed(pinoVerde2, 100, 100, 100, millis_referencia);

  piscaLed(pinoVermelho2, tempoCiclo - tempoLedVerdePedestre - tempoLedVerdePedestreFim, tempoLedVerdePedestre + tempoLedVerdePedestreFim, tempoCiclo - tempoEspera, millis_referencia);
}

int calculoTempoCiclo()
{
  //int temp = calculoTempoAtraso();
  return (tempoLedAmarelo + tempoEspera * 2 + tempoLedVerdePedestre + tempoLedVerdePedestreFim + tempoAtraso);
}

int calculoTempoAtraso()
{

  if (analogRead(pinoPotenciometro) < 341)
    return 5000;
  else if ((analogRead(pinoPotenciometro) >= 341) && (analogRead(pinoPotenciometro) < 682))
    return 10000;
  else
    return 15000;
}

void temperaturaLCD() {

  if (millis() - millisTemperatura > 1000) {
    millisTemperatura = millis();
    temperatura = temp.getTemp();
  }

  lcd.setCursor(0, 0);
  lcd.print(temperatura);
  lcd.setCursor(2, 0);
  lcd.print("Graus Celsius");
  lcd.setCursor(0, 1);
  lcd.print("Trafego ");
  lcd.setCursor(8, 1);

  if (calculoTempoAtraso() == 5000) {
    lcd.print("baixo");
    lcd.setCursor(13, 1);
    lcd.print("   ");
  }
  else if (calculoTempoAtraso() == 10000) {
    lcd.print("medio");
    lcd.setCursor(13, 1);
    lcd.print("   ");
  }
  else
    lcd.print("intenso");
}

void msgAbrirSinal() {
  flagReferencia2 = 0;
  flagClearSemaforoPrincipal = 0;
  lcd.setCursor(0, 0);
  lcd.print("AGUARDE!       ");
  lcd.setCursor(0, 1);
  lcd.print("Sinal abre em ");
  lcd.setCursor(14, 1);
  millisAtual = millis();


  if (tempoAtraso == 5000) {
    if ((millisAtual - millis_referencia > 0) && (millisAtual - millis_referencia <= (1000))) {
      lcd.print("8");
      digitalWrite(a, HIGH);//DIGITO 7
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 1000) && (millisAtual - millis_referencia <= 2000)) {
      lcd.print("7");
      digitalWrite(a, LOW);//DIGITO 6
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 2000) && (millisAtual - millis_referencia <= 3000)) {
      lcd.print("6");
      digitalWrite(a, HIGH);//DIGITO 5
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);



    }
    else if ((millisAtual - millis_referencia > 3000) && (millisAtual - millis_referencia <= 4000)) {
      lcd.print("5");
      digitalWrite(a, LOW);//DIGITO 4
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 4000) && (millisAtual - millis_referencia <= 5000)) {
      lcd.print("4");
      digitalWrite(a, HIGH);//DIGITO 3
      digitalWrite(b, HIGH);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);



    }
    else if ((millisAtual - millis_referencia > 5000) && (millisAtual - millis_referencia <= 6000)) {
      lcd.print("3");
      digitalWrite(a, LOW);//DIGITO 2
      digitalWrite(b, HIGH);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 6000) && (millisAtual - millis_referencia <= 7000)) {
      lcd.print("2");
      digitalWrite(a, HIGH);//DIGITO 1
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 7000) && (millisAtual - millis_referencia <= 8000)) {
      lcd.print("1");
      digitalWrite(a, LOW);//DIGITO 0
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
  }
  else if (tempoAtraso == 10000) {
    if ((millisAtual - millis_referencia > 0) && (millisAtual - millis_referencia <= (1000)))
      lcd.print("13");
    else if ((millisAtual - millis_referencia > 1000) && (millisAtual - millis_referencia <= 2000))
      lcd.print("12");
    else if ((millisAtual - millis_referencia > 2000) && (millisAtual - millis_referencia <= 3000))
      lcd.print("11");
    else if ((millisAtual - millis_referencia > 3000) && (millisAtual - millis_referencia <= 4000)) {
      lcd.print("10");
      digitalWrite(a, HIGH); //DIGITO 9
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, HIGH);

    }
    else if ((millisAtual - millis_referencia > 4000) && (millisAtual - millis_referencia <= 5000)) {
      lcd.print("9");
      digitalWrite(a, LOW);//DIGITO 8
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, HIGH);
      /*
        Esse if conserta a contagem de 10, 90, 80... para 10, 9, 8...
      */
      if (flagClear == 0) {
        Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
        lcd.setCursor(15, 1);
        lcd.print(" ");
        flagClear = 1;
      }
    }
    else if ((millisAtual - millis_referencia > 5000) && (millisAtual - millis_referencia <= 6000)) {
      lcd.print("8");
      flagClear = 0;
      digitalWrite(a, HIGH);//DIGITO 7
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);
    }
    else if ((millisAtual - millis_referencia > 6000) && (millisAtual - millis_referencia <= 7000)) {
      lcd.print("7");
      digitalWrite(a, LOW);//DIGITO 6
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 7000) && (millisAtual - millis_referencia <= 8000)) {
      lcd.print("6");
      digitalWrite(a, HIGH);//DIGITO 5
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 8000) && (millisAtual - millis_referencia <= 9000)) {
      lcd.print("5");
      digitalWrite(a, LOW);//DIGITO 4
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 9000) && (millisAtual - millis_referencia <= 10000)) {
      lcd.print("4");
      digitalWrite(a, HIGH);//DIGITO 3
      digitalWrite(b, HIGH);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 10000) && (millisAtual - millis_referencia <= 11000)) {
      lcd.print("3");
      digitalWrite(a, LOW);//DIGITO 2
      digitalWrite(b, HIGH);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 11000) && (millisAtual - millis_referencia <= 12000)) {
      lcd.print("2");
      digitalWrite(a, HIGH);//DIGITO 1
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 12000) && (millisAtual - millis_referencia <= 13000)) {
      lcd.print("1");
      digitalWrite(a, LOW);//DIGITO 0
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);



    }
  }
  else {
    if ((millisAtual - millis_referencia > 0) && (millisAtual - millis_referencia <= (1000)))
      lcd.print("18");
    else if ((millisAtual - millis_referencia > 1000) && (millisAtual - millis_referencia <= 2000))
      lcd.print("17");
    else if ((millisAtual - millis_referencia > 2000) && (millisAtual - millis_referencia <= 3000))
      lcd.print("16");
    else if ((millisAtual - millis_referencia > 3000) && (millisAtual - millis_referencia <= 4000))
      lcd.print("15");
    else if ((millisAtual - millis_referencia > 4000) && (millisAtual - millis_referencia <= 5000))
      lcd.print("14");
    else if ((millisAtual - millis_referencia > 5000) && (millisAtual - millis_referencia <= 6000))
      lcd.print("13");
    else if ((millisAtual - millis_referencia > 6000) && (millisAtual - millis_referencia <= 7000))
      lcd.print("12");
    else if ((millisAtual - millis_referencia > 7000) && (millisAtual - millis_referencia <= 8000))
      lcd.print("11");
    else if ((millisAtual - millis_referencia > 8000) && (millisAtual - millis_referencia <= 9000)) {
      lcd.print("10");
      digitalWrite(a, HIGH); //DIGITO 9
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, HIGH);

    }
    else if ((millisAtual - millis_referencia > 9000) && (millisAtual - millis_referencia <= 10000)) {
      lcd.print("9");
      digitalWrite(a, LOW);//DIGITO 8
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, HIGH);
      /*
         Esse if conserta a contagem de 10, 90, 80... para 10, 9, 8...
      */
      if (flagClear == 0) {
        lcd.setCursor(15, 1);
        lcd.print(" ");
        flagClear = 1;
      }
    }
    else if ((millisAtual - millis_referencia > 10000) && (millisAtual - millis_referencia <= 11000)) {
      lcd.print("8");
      flagClear = 0;
      digitalWrite(a, HIGH);//DIGITO 7
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);
    }
    else if ((millisAtual - millis_referencia > 11000) && (millisAtual - millis_referencia <= 12000)) {
      lcd.print("7");
      digitalWrite(a, LOW);//DIGITO 6
      digitalWrite(b, HIGH);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 12000) && (millisAtual - millis_referencia <= 13000)) {
      lcd.print("6");
      digitalWrite(a, HIGH);//DIGITO 5
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 13000) && (millisAtual - millis_referencia <= 14000)) {
      lcd.print("5");
      digitalWrite(a, LOW);//DIGITO 4
      digitalWrite(b, LOW);
      digitalWrite(c, HIGH);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 14000) && (millisAtual - millis_referencia <= 15000)) {
      lcd.print("4");
      digitalWrite(a, HIGH);//DIGITO 3
      digitalWrite(b, HIGH);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 15000) && (millisAtual - millis_referencia <= 16000)) {
      lcd.print("3");
      digitalWrite(a, LOW);//DIGITO 2
      digitalWrite(b, HIGH);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 16000) && (millisAtual - millis_referencia <= 17000)) {
      lcd.print("2");
      digitalWrite(a, HIGH);//DIGITO 1
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
    else if ((millisAtual - millis_referencia > 17000) && (millisAtual - millis_referencia <= 18000)) {
      lcd.print("1");
      digitalWrite(a, LOW);//DIGITO 0
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);


    }
  }
}

void msgFecharSinal() {
  if (flagReferencia2 == 0) {
    millis_referencia2 = millis();
    flagReferencia2 = 1;
  }


  long ajusteReferenciaBuzzer   = millis_referencia % (2 * timePiscaBuzzer);
  long restoBuzzer             = (millis() + (2 * timePiscaBuzzer) - ajusteReferenciaBuzzer - tempoAtraso) % (2 * timePiscaBuzzer);
  if (restoBuzzer < timePiscaBuzzer)
    digitalWrite(pinoBuzzer, HIGH);
  else
    digitalWrite(pinoBuzzer, LOW);


  lcd.setCursor(15, 0);
  lcd.print(" ");
  lcd.setCursor(0, 0);
  lcd.print("Prossiga!      ");
  lcd.setCursor(0, 1);
  lcd.print("Sinal fecha em ");
  lcd.setCursor(15, 1);
  millisAtual = millis();
  if ((millisAtual - millis_referencia2 > 0) && (millisAtual - millis_referencia2 <= 1000)) {
    lcd.print("8");
    digitalWrite(a, HIGH); //DIGITO 9
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(d, HIGH);
  }
  else if ((millisAtual - millis_referencia2 > 1000) && (millisAtual - millis_referencia2 <= 2000)) {
    lcd.print("7");
    digitalWrite(a, LOW);//DIGITO 8
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(d, HIGH);
  }
  else if ((millisAtual - millis_referencia2 > 2000) && (millisAtual - millis_referencia2 <= 3000)) {
    lcd.print("6");
    digitalWrite(a, HIGH);//DIGITO 7
    digitalWrite(b, HIGH);
    digitalWrite(c, HIGH);
    digitalWrite(d, LOW);

  }
  else if ((millisAtual - millis_referencia2 > 3000) && (millisAtual - millis_referencia2 <= 4000)) {
    lcd.print("5");
    digitalWrite(a, LOW);//DIGITO 6
    digitalWrite(b, HIGH);
    digitalWrite(c, HIGH);
    digitalWrite(d, LOW);

  }
  else if ((millisAtual - millis_referencia2 > 4000) && (millisAtual - millis_referencia2 <= 5000)) {
    lcd.print("4");
    digitalWrite(a, HIGH);//DIGITO 5
    digitalWrite(b, LOW);
    digitalWrite(c, HIGH);
    digitalWrite(d, LOW);

  }
  else if ((millisAtual - millis_referencia2 > 5000) && (millisAtual - millis_referencia2 <= 6000)) {
    lcd.print("3");
    digitalWrite(a, LOW);//DIGITO 4
    digitalWrite(b, LOW);
    digitalWrite(c, HIGH);
    digitalWrite(d, LOW);
    flagFechamentoSinalPedestre = 0;


  }

}

void msgAlertaSinal() {

  digitalWrite(pinoBuzzer, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("AGUARDE!        ");
  lcd.setCursor(0, 1);
  if(flagFechamentoSinalPedestre == 0)
    lcd.print("Sinal fecha em ");
  else
    lcd.print("FECHOU!       ");
  lcd.setCursor(15, 1);
  millisAtual = millis();
  if ((millisAtual - millis_referencia2 > 6000) && (millisAtual - millis_referencia2 <= 7000)) {
    lcd.print("2");
    digitalWrite(a, HIGH);//DIGITO 3
    digitalWrite(b, HIGH);
    digitalWrite(c, LOW);
    digitalWrite(d, LOW);
  }
  else if ((millisAtual - millis_referencia2 > 7000) && (millisAtual - millis_referencia2 <= 8000)) {
    lcd.print("1");
    digitalWrite(a, LOW);//DIGITO 2
    digitalWrite(b, HIGH);
    digitalWrite(c, LOW);
    digitalWrite(d, LOW);
    
    
    
  }
  else if ((millisAtual - millis_referencia2 > 8000) && (millisAtual - millis_referencia2 <= 9000)) {
    //lcd.print("0");
    flagFechamentoSinalPedestre = 1;
    lcd.setCursor(15, 1);
    lcd.print(" ");
    digitalWrite(a, HIGH);//DIGITO 1
    digitalWrite(b, LOW);
    digitalWrite(c, LOW);
    digitalWrite(d, LOW);
  }
}

/*
  int luzPedestre() {
  if (millis() - millisLuzPedestre > 5000)
  {
    millisLuzPedestre = millis();
    if (pwmLDR < 100)
      digitalWrite(pinoRele, HIGH);
    else
      digitalWrite(pinoRele, LOW);
  }

  }
*/



