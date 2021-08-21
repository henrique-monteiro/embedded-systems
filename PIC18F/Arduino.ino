
const int pinoVerde1     = 2; //Todos os led's com 1 na frente representa os led's da via principal
const int pinoAmarelo1   = 3;
const int pinoVermelho1  = 4;
const int pinoVerde2     = 5; //Todos os led's com 2 na frente representa os led's da travessia do pedestre
const int pinoVermelho2  = 6;
const int pinoVerde3     = 7; //Todos os led's com 3 na frente representa os led's da via de acesso
const int pinoAmarelo3   = 8;
const int pinoVermelho3  = 9;
  
int flagPisca = 0; //utilizada para fazer os led's piscarem  
long millisPisca = 0; //utilizado para fazer os led's piscarem (ambos serão explicados melhor mais pra frente)

int Control = 1;  // +++++++ VARIAVEL DE CONTROLE DA COMUNICAÇÃO ++++++++
// LEMBRE DE ALTERAR ESSA VARIAVE TAMBEM NO PIC

void setup() {
  //setando todos os pinos como saída
  pinMode(pinoVerde1,    OUTPUT);
  pinMode(pinoAmarelo1,  OUTPUT);
  pinMode(pinoVermelho1, OUTPUT);
  pinMode(pinoVerde2,    OUTPUT);
  pinMode(pinoVermelho2, OUTPUT);
  pinMode(pinoVerde3,    OUTPUT);
  pinMode(pinoAmarelo3,  OUTPUT);
  pinMode(pinoVermelho3, OUTPUT);
  
  Serial.begin(9600);
  Serial1.begin(9600);
  
  //setando os led's para condição inicial: verde para a via principal, vermelho para o pedestre e via de acesso
  digitalWrite(pinoVerde1, HIGH);
  digitalWrite(pinoAmarelo1, LOW);
  digitalWrite(pinoVermelho1, LOW);
  digitalWrite(pinoVerde2, LOW);
  digitalWrite(pinoVermelho2, HIGH);
  digitalWrite(pinoVerde3, LOW);
  digitalWrite(pinoAmarelo3, LOW);
  digitalWrite(pinoVermelho3, HIGH);

}

void loop()
{
  if (Control == 1) {
    if (Serial1.available()) { // Verificar se há caracteres disponíveis
      char caractere = Serial1.read(); // Armazena caracter lido.
      
      if (caractere == 'A') { //configuração de condição inicial: verde para via principal, vermelho para pedestre e via de acesso
        flagPisca = 1;        
        digitalWrite(pinoVerde1, HIGH);
        digitalWrite(pinoAmarelo1, LOW);
        digitalWrite(pinoVermelho1, LOW);
        digitalWrite(pinoVerde2, LOW);
        digitalWrite(pinoVermelho2, HIGH);
        digitalWrite(pinoVerde3, LOW);
        digitalWrite(pinoAmarelo3, LOW);
        digitalWrite(pinoVermelho3, HIGH);
      }
      if (caractere == 'B') { //amarelo para via principal, vermelho para pedestre e via de acesso (sinal da via principal fechando)
        digitalWrite(pinoVerde1, LOW);
        digitalWrite(pinoAmarelo1, HIGH);
        digitalWrite(pinoVermelho1, LOW);
        digitalWrite(pinoVerde2, LOW);
        digitalWrite(pinoVermelho2, HIGH);
        digitalWrite(pinoVerde3, LOW);
        digitalWrite(pinoAmarelo3, LOW);
        digitalWrite(pinoVermelho3, HIGH);
      }
      if (caractere == 'C') { //sinal fechado para via principal: vermelho para via principal, verde para pedestre e via de acesso      
        digitalWrite(pinoVerde1, LOW);
        digitalWrite(pinoAmarelo1, LOW);
        digitalWrite(pinoVermelho1, HIGH);
        digitalWrite(pinoVerde2, HIGH);
        digitalWrite(pinoVermelho2, LOW);
        digitalWrite(pinoVerde3, HIGH);
        digitalWrite(pinoAmarelo3, LOW);
        digitalWrite(pinoVermelho3, LOW);
      }
      if (caractere == 'D') { //sinal do pedestre/via de acesso fechando: 
        if (flagPisca == 1) { 
          millisPisca = millis(); //millisPisca é utilizado para fazer com que os led's pisquem conforme a função millis() vai aumentando
          flagPisca = 0; // desabilita a flag fazendo com que não entre mais neste if (esta flag recebe 1 na linha 50 para reabilitar o if)
        }        
        digitalWrite(pinoVerde1, LOW);
        digitalWrite(pinoAmarelo1, LOW);
        digitalWrite(pinoVermelho1, HIGH);
        digitalWrite(pinoVerde2, LOW);

        //apenas o led vermelho do pedestre que deve piscar, em intevalos de 200milisegundos, cujo trecho de código encontra-selogo abaixo
        if (millis() - millisPisca < 200) {
          digitalWrite(pinoVermelho2, HIGH);
        }
        else if (millis() - millisPisca >= 200 && millis() - millisPisca <= 400) {
          digitalWrite(pinoVermelho2, LOW);
        }
        else if (millis() - millisPisca >= 400 && millis() - millisPisca <= 600) {
          digitalWrite(pinoVermelho2, HIGH);
        }
        else if (millis() - millisPisca >= 600 && millis() - millisPisca <= 800) {
          digitalWrite(pinoVermelho2, LOW);
        }
        else if (millis() - millisPisca >= 800 && millis() - millisPisca <= 1000) {
          digitalWrite(pinoVermelho2, HIGH);
        }        
        else if (millis() - millisPisca >= 1000 && millis() - millisPisca <= 1200) {
          digitalWrite(pinoVermelho2, LOW);
        }
        else if (millis() - millisPisca >= 1200 && millis() - millisPisca <= 1400) {
          digitalWrite(pinoVermelho2, HIGH);
        }
        else if (millis() - millisPisca >= 1400 && millis() - millisPisca <= 1600) {
          digitalWrite(pinoVermelho2, LOW);
        }
        else if (millis() - millisPisca >= 1600 && millis() - millisPisca <= 1800) {
          digitalWrite(pinoVermelho2, HIGH);
        }
        else if (millis() - millisPisca >= 1800 && millis() - millisPisca <= 2000) {
          digitalWrite(pinoVermelho2, LOW);
        }

        //atribui valores aos led's restante
        digitalWrite(pinoVerde3, LOW);
        digitalWrite(pinoAmarelo3, HIGH);
        digitalWrite(pinoVermelho3, LOW);
      }
    }
  }
}



