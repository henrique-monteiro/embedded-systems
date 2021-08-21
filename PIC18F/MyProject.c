// ********************************************************************************
//     			Projeto Final: Semáforo inteligente

//      OBJETIVO: Aplicar os conhecimentos adquiridos durante a disciplina
//	para se utilizar os componentes previamente combinado com o professor 

//                       MICROCONTROLADOR: PIC18F4520.
//         http://ww1.microchip.com/downloads/en/DeviceDoc/39631E.pdf

// ********************************************************************************

// CHAVES DE FUN??O:
// ------- Chave 1 -----  -------- Chave 2 ------
// |GLCD\LCD ( 1) = ON   |DIS1     ( 1) = OFF   |
// |RX       ( 2) = ON  |DIS2     ( 2) = OFF   |
// |TX       ( 3) = ON  |DIS3     ( 3) = OFF   |
// |REL1     ( 4) = OFF  |DIS4     ( 4) = OFF   |
// |REL2     ( 5) = OFF  |INFR     ( 5) = ON    |
// |SCK      ( 6) = OFF  |RESIS    ( 6) = OFF   |
// |SDA      ( 7) = OFF  |TEMP     ( 7) = OFF   |
// |RTC      ( 8) = OFF  |VENT     ( 8) = OFF    |
// |LED1     ( 9) = OFF  |AN0      ( 9) = ON    |
// |LED2     (10) = ON   |AN1      (10) = OFF   |
//  ---------------------------------------------

// vARIAVEIS GLOBAIS
unsigned char ucTexto[10];   	// Matriz para armazenamento de texto.
unsigned int trafego = 0; 	// Define variável para armazenamento da leitura AD. (intensidade do tráfego)
int cicloTotal = 0;		// Variável para armazenamento de um evento completo (ex: pedestre atravessar avia principal)
int contador = 0;		// variável que controla o tempo que cada sinal ficará aberto ou fechado
int statusBotao = 0;		// variável para impedir que apertar o botão de atravessia de pedestre mais de uma vez altere os calculos
unsigned int Control = 1;  	// +++++++ VARIAVEL DE CONTROLE DA COMUNICAÇÃO ++++++++
                           	// LEMBRE DE ALTERAR ESSA VARIAVE TAMBEM NO ARDUINO

// CONFIGURA??O DOS PINOS DO LCD.
sbit LCD_RS at RE2_bit;
sbit LCD_EN at RE1_bit;
sbit LCD_D7 at RD7_bit;
sbit LCD_D6 at RD6_bit;
sbit LCD_D5 at RD5_bit;
sbit LCD_D4 at RD4_bit;

// DIRE??O DOS PINOS.
sbit LCD_RS_Direction at TRISE2_bit;
sbit LCD_EN_Direction at TRISE1_bit;
sbit LCD_D7_Direction at TRISD7_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D4_Direction at TRISD4_bit;

void interrupt(){
   if (INTCON.TMR0IF == 1){    		// Se o flag de estouro do TIMER0 for igual a 1, ent?o
      //PORTB.RB0 = ~PORTB.RB0;  	// Inverte o estado do PORTB.RB0.
      TMR0L = 0X7B;                  	// Carrega valores de contagem
      TMR0H = 0XE1;                  	// Carrega valores de contagem
      INTCON.TMR0IF = 0;             	// Seta T0IE, apaga flag de entouro do TIMER0
      contador++;			// alterada a cada segundo para controlar o tempo que cada led ficará aceso ou apagado 

      TMR1L = 0;                     	// Limpa contador.
   }
}

//calcula um ciclo inteiro, desde qndo o pedestre aperta o botão até quando o semaforo da via principal abre novamente
//ciclo = 220 -> 10 segundos para abrir o sinal do pedestre
//ciclo = 270 -> 15 segundos para abrir o sinal do pedestre
//ciclo = 320 -> 20 segundos para abrir o sinal do pedestre
int calculoCicloTotal(){
  if(trafego <= 350){ 				//se o valor lido do potenciômetro for menor que 350 significa que o tráfego na via principal é leve
    //Lcd_out(1,1,"Trafego leve    ");
    cicloTotal = 220;
  }
  else if(trafego > 350 && trafego <= 700){ 	//se o valor lido do potenciômetro for maior que 350 e menor que 700 significa que o tráfego na via principal é médio
    //Lcd_out(1,1,"Trafego medio   ");
    cicloTotal = 270;
  }
  else{
    //Lcd_out(1,1,"Trafego intenso "); 		//se o valor lido do potenciômetro for maior que 700 significa que o tráfego na via principal é intenso
    cicloTotal = 320;
  }
  return cicloTotal;
}

//Definiu-se 3 intensidades de trafego.
void imprimeTrafego(){
  if(trafego <= 350){				//se o valor lido do potenciômetro for menor que 350 imprime "trafego leve" no display
    Lcd_out(1,1,"Trafego leve    ");
  }
  else if(trafego > 350 && trafego <= 700){	//se o valor lido do potenciômetro for menor que 350 e maior que 700 imprime "trafego medio" no display
    Lcd_out(1,1,"Trafego medio   ");
  }
  else{						//se o valor lido do potenciômetro for maior que 700 imprime "trafego leve" no display
    Lcd_out(1,1,"Trafego intenso ");
  }
}

void main(){

   TRISD = 0;                        // Define PORTD como saida.
   TRISC.RC0 = 1;                    // Define PORTC.RC0 como entrada.
   TRISC.RC2 = 0;                    // Define PORTC.RC2 como saida.
   TRISE = 0;                        // Define PORTE como saida.

   // Configuracao das interrupoes
   INTCON.GIEH = 1;   // Habilita as interrupcoes e a interrupcao de alta prioridade.
   INTCON.GIEL = 1;   // Habilita as interrupcoes e a interrupcao de baixa prioridade
   RCON.IPEN = 1;     // Configura 2 niveis de interrupcao.

   // Timer 0
   INTCON.TMR0IF = 0;
   INTCON2.TMR0IP = 1;
   INTCON.TMR0IE = 1;

   T0CON = 0B10000100; // Configura timer modo 16 bits, com prescaler
   TMR0L = 0X7B;       // Carrega valores de contagem
   TMR0H = 0XE1;       // Carrega valores de contagem
   INTCON.TMR0IF = 0;  // Apaga flag de estouro do TIMER0

   ADCON0 = 0b00000001;              // Configura conversor A/D Canal 0, conversao desligada, A/D ligado.
   ADCON1 = 0b00001110;              // Configura todos canais como Digital menos AN0 e REF Interna.
   ADCON2 = 0b10111110;              // Configura conversor A/D para resultado justificado a direita, clock de 20 TAD, clock de Fosc/64.

   // Config. LCD no modo 4 bits
   Lcd_Init();                               // Inicializa LCD.
   Lcd_Cmd(_LCD_CLEAR);                      // Apaga display.
   Lcd_Cmd(_LCD_CURSOR_OFF);                 // Desliga cursor.
   
   TRISB.RB0=1;         // Define o pino RB0 do PORTB como entrada.
   
   UART1_Init(9600);  // Utiliza bibliotecas do compilador para configuração o Baud rate.

   TRISC.RC4 = 1;
   
   while(1){   // Aqui Definimos Uma Condi??o Sempre Verdadeira Como Parametro, Portanto Todo O Bloco Ser? Repetido Indefinidamente.

      trafego= ADC_Read(0);          // Le Canal AD 0 que representa a intensidade do fluxo de veículos na via principal
      
      imprimeTrafego();              // imprime trafego no lcd
      
      
      if(PORTB.RB0==0 && statusBotao == 0){//esse if garante que se o pedestre apertar mais de uma vez o botao a contagem nao altera
        statusBotao = 1;
        contador = 0;
        cicloTotal = calculoCicloTotal();
      }

      //essa duas linhas é apenas para controle para mostrar no display
      //WordToStr(contador, ucTexto);   // Converte o valor lido no A/D em string
      //Lcd_Out(2,1,ucTexto);
      
      //as letras A, B, C, D são as possíveis configuracoes do semaforo (está mais claro no codigo no arduino)
      //Configuração A: verde para a via principal, vermelho para pedestre e via de acesso
      //Configuração B: amarelo para via principal, vermelho para pedestre e via de acesso
      //Configuração C: vermelho para via principal, verde para pedestre e via de acesso
      //Configuração D: Vermelho para via principal, vermelho piscando para pedester e amarelo para via de acesso
      
      //o que torna o semaforo inteligente é o tempo de fechamento da via principal, uma vez que a alpicação deste
      //projeto em específico é voltado para desafogar a via principal
      
      //Configuracoes para trafego leve
      if(cicloTotal == 220){ 			//esse if trata de todo o ciclo para um tráfego leve
        if(contador < 100){  			//sinal verde da via principal demora 15 segundos para fechar
          UART1_Write('A');			//Comunicação com o arduino para execução da configuração 'A'
          WordToStr(100 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >=100 && contador <120){ 	//A configuração B fica ativa por apenas dois segundos
          UART1_Write('B'); 				//Comunicação com o arduino para execução da configuração 'B' 
          WordToStr(220 - contador, ucTexto);   	// Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >= 120 && contador < 200){	//A configuração C fica ativa por 80 segundos
          UART1_Write('C');				//Comunicação com o arduino para execução da configuração 'C'
          WordToStr(220 - contador, ucTexto);   	// Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >= 200 && contador < cicloTotal){	//A configuração D fica ativa por apenas 20 segundos
          UART1_Write('D');					//Comunicação com o arduino para execução da configuração 'D'
          WordToStr(220 - contador, ucTexto);   		// Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else{						//retorno para as condições iniciais
          UART1_Write('A');				//Comunicação com o arduino para execução da configuração 'A' (que é a condição incial)
          cicloTotal = 0;				//fim do ciclo
          contador = 0;					//prepara contagem para um possível novo evento
          statusBotao = 0;				//habilita o botão para o pedestre (lembrando que seo pedestre aperta mais de uma vez nao altera os valores)
          WordToStr(contador, ucTexto);   		// Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
      }

      //Configuracoes para trafego medio. 
      //Processo muito semelhante ao anterior, com excessão que neste caso o sinal verde da via principal demora 15 segundos para fechar
      else if(cicloTotal == 270){		//esse if trata de todo o ciclo para um tráfego medio
        if(contador < 150){ 			//sinal verde da via principal demora 15 segundos para fechar
          UART1_Write('A'); 			//Comunicação com o arduino para execução da configuração 'A'
          WordToStr(150 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >= 150 && contador < 170){
          UART1_Write('B');
          WordToStr(270 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >= 170 && contador < 250){
          UART1_Write('C');
          WordToStr(270 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >= 250 && contador < 270){
          UART1_Write('D');
          WordToStr(270 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
          
        }
        else{
          UART1_Write('A');
          cicloTotal = 0;
          contador = 0;
          statusBotao = 0;
          WordToStr(contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);

        }
      }

      //Configuracoes para trafego intenso
      ////Processo muito semelhante ao anterior, com excessão que neste caso o sinal verde da via principal demora 20 segundos para fechar
      else if(cicloTotal == 320){
        if(contador < 200){
          UART1_Write('A');
          WordToStr(200 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >= 200 && contador < 220){
          UART1_Write('B');
          WordToStr(320 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >= 220 && contador < 300){
          UART1_Write('C');
          WordToStr(320 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else if(contador >= 300 && contador < 320){
          UART1_Write('D');
          WordToStr(320 - contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
        else{
          UART1_Write('A');
          cicloTotal = 0;
          contador = 0;
          statusBotao = 0;
          WordToStr(contador, ucTexto);   // Converte o valor lido no A/D em string
          Lcd_Out(2,1,ucTexto);
        }
      }
      

   }
      


}