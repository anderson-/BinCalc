/*
Codigo fonte do dispositivo de operação lógicas e aritméticas.
Projeto estruturado com a finalidade de aprovação na disciplina
de Oficina de Integração I, do 2º periodo do curso de Enge-
nharia da computação na Universidade Tecnologica Federal do Pa-
raná (UTFPR).

Copyright (C) 2012  
Anderson de Oliveira Antunes <anderson.utf@gmail.com>
Fernando Henrique Carvalho Ferreira <fhcf31@hotmail.com>
Mozarth Mateus Silvino do Nascimento <darconin@hotmail.com>
Rômulo Ianuch Souza <romulo92@hotmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/
*/

       //Bibliotecas:
    //Biblioteca do infravermelho
#include <Arduino.h>
#include <IRremote.h>
      //Definindo Pinos:
   //Pinos para exibição (conectados ao shift register 74HC595):
// Pino conectado ao ST_CP do 74HC595, Shift register de exibição 
#define LATCH_PIN_EXIBIR  5
// Pino conectado ao SH_CP do 74HC595, Shift register de exibição
#define CLOCK_PIN_EXIBIR  7
// Pino conectado ao DS do 74HC595, Shift register de exibição
#define DATA_PIN_EXIBIR  6
/*OBS.: Os 74HC595 são os shift registers responsaveis pela exibição dos
valores nos LEDs e Displays*/
   //Pinos para entrada do teclado (conectados ao shift register 74HC165):
//
#define PLOAD_PIN_LEITURA  2
// Pino conectado a entrada do 74HC165, shift register de leitura
#define DATA_PIN_LEITURA  3
// Pino conectado ao clock do 74HC165, shift register de leitura
#define CLOCK_PIN_LEITURA  4
   //Pino do sensor infravermelho(controle remoto):
#define INFRAVERMELHO_PIN  11

    //valores binarios passados para exibição nos displays.
const byte numeros[19] = {
                 // = Decimal = Hexadecimal = Octal 
      0b11111010,// =    0    =      0      =   0
      0b01100000,// =    1    =      1      =   1
      0b11011100,// =    2    =      2      =   2
      0b11110100,// =    3    =      3      =   3
      0b01100110,// =    4    =      4      =   4
      0b10110110,// =    5    =      5      =   5
      0b10111110,// =    6    =      6      =   6
      0b11100000,// =    7    =      7      =   7
      0b11111110,// =    8    =      8      =   --
      0b11100110,// =    9    =      9      =   --
      0b11101110,// =    --   =      A      =   --
      0b00111110,// =    --   =      B      =   --
      0b10011010,// =    --   =      C      =   --
      0b01111100,// =    --   =      D      =   --
      0b10011110,// =    --   =      E      =   --
      0b10001110,// =    --   =      F      =   --
      0b00000001,// = ponto
      0b00001100,// = r
      0b00011010,// = L
};

//variaveis global:
   //Dados de exibição e calculos:
byte entradaA = 0;
byte entradaB = 0;
byte saida = 0;
byte modoExibicao = 10;//Armazena a caracteristica da base que será exibida. 
boolean complemento = false;//variavel que indica se a saida vai ser ou não aproveitada no calculo
boolean erro = false;//variavel que indica se ouve um erro no programa
boolean inicio = true;//variavel que indica se é a primeira vez que o programa executa
   
   //Dados de entrada do teclado:
unsigned long entrada;//Armazena o valor dos botões
unsigned long velhaEntrada;//Armazena o valor dos botões na ultima leitura
   
   //Dados do infravermelho:
IRrecv ivReceptor(INFRAVERMELHO_PIN);//Objeto da classe IRrecv
decode_results codigoBotao;//Armazena o codigo do botao apertado no controle

void exibir();
void calculo(byte op);
unsigned long lerEntradaDados();
byte receberInfravermelho();
byte operacao(unsigned long e);

void setup()
{
  //OBS.:OUTPUT, indica que o pino envia informação e INPUT, indica que o pino recebe informação. 
  pinMode(LATCH_PIN_EXIBIR, OUTPUT);
  pinMode(CLOCK_PIN_EXIBIR, OUTPUT);
  pinMode(DATA_PIN_EXIBIR, OUTPUT);
  
  pinMode(PLOAD_PIN_LEITURA, OUTPUT);
  pinMode(CLOCK_PIN_LEITURA, OUTPUT);
  pinMode(DATA_PIN_LEITURA, INPUT);

  digitalWrite(PLOAD_PIN_LEITURA, LOW);
  delayMicroseconds(5);
  digitalWrite(PLOAD_PIN_LEITURA, HIGH);
  
  ivReceptor.enableIRIn();//inicia o receptor infravermelho
  
  velhaEntrada = 0;
}

void loop()
{ 
  entrada = lerEntradaDados();

  if(entrada != velhaEntrada)
  {
    inicio = false;//desabilita o inicio
    erro = false;//desabilita o erro caso a ultima operação tenha sido divisão por 0.
    velhaEntrada = entrada;
 
    /*esta operação realiza shift right na entrada até que só sobrar os valores do último byte,
    byte que armazena as informações das entradas e realiza um "e" logico para despresar os ultimos
    4 bits que contém o valor da entradaA*/
    entradaB ^= ((entrada >> 16) & 0xF);
    //esta operação realiza shift right na entrada até só sobrar os ultimos 4 bits, que é entradaA.
    entradaA ^= (entrada >> 20);
    calculo(operacao(entrada));
    /*se a operação em complemento estiver ativada realizamos a adequação dos valores para a exibição
    saida exibi um numero de no maximo 4 bits e a entrada A o mesmo valor da saida para ser reapro-
    veitado na proxima operação
    OBS.: a verificação só pode ocorrer após realizado os calculos e imediatamente antes da exibição,
    senão ocorre erros como de vez enquando aparecer valores diferentes na entradaA e saida*/
    if(complemento != 0)
    {   
      saida = saida % 16;
      entradaA = saida;
    }
    if(erro)
    {
      entradaA = 0;
      entradaB = 0;
      saida = 0;
    }
  }
  
  byte p = receberInfravermelho();//recebe o codigo da operação retornada pelo método receberInfravermelho()
  if (p)
  {
    inicio = false;//desabilita o inicio
    erro = false;//desabilita o erro caso a ultima operação tenha sido divisão por 0.

    calculo(p);
    /*se a operação em complemento estiver ativada realizamos a adequação dos valores para a exibição
    saida exibi um numero de no maximo 4 bits e a entrada A o mesmo valor da saida para ser reapro-
    veitado na proxima operação
    OBS.: a verificação só pode ocorrer após realizado os calculos e imediatamente antes da exibição,
    senão ocorre erros como de vez enquando aparecer valores diferentes na entradaA e saida*/
    if(complemento)
    {   
      saida = saida % 16;
      entradaA = saida;
    }
    if(erro)
    {
      entradaA = 0;
      entradaB = 0;
      saida = 0;
    }
  }
  exibir();
}

//Exibi os valores acendendo os displays e leds
void exibir()
{
  /*OBS.: A ordem das operações, deste metodo, alteram o resultado,
  por estarmos trabalhando com shift registers*/  
  //Entrada B(display):
  int dezenaB = entradaB%(modoExibicao*modoExibicao)/(modoExibicao);
  int unidadeB = entradaB%(modoExibicao*modoExibicao)%(modoExibicao);
  
  shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[dezenaB]);
  shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[unidadeB]);
  
  //LEDs Entradas:
  byte exibir;
  //concatenando byte a e byte b,ambos com somente 4 bits significativos, em 1 byte somente
  int aux = entradaA << 4;
  exibir = aux | entradaB;
  
  shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,MSBFIRST,exibir);

  //Entrada A(display):
 
  int dezenaA = entradaA%(modoExibicao*modoExibicao)/(modoExibicao);
  int unidadeA = entradaA%(modoExibicao*modoExibicao)%(modoExibicao);
  
  shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[dezenaA] + complemento);
  shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[unidadeA] + complemento);
  
  //LEDs da saida:
  shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,MSBFIRST,saida);
  
  //Saida(display):
  //exibe a mensagem de erro caso a divisão seja por 0.
  if (erro)
  {
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[14]);//exibi E
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[17]);//exibi r
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,(numeros[17]+1));//exibi r. //mais 1 é para o ponto
    velhaEntrada = 0;
  }
  else if(inicio)
  {
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[0]);//exibi O
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[18]);//exibi l
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,(numeros[10]));//exibi A
  }
  else
  {
    int centenaS = (saida/(modoExibicao*modoExibicao) > (modoExibicao-1))? (modoExibicao-1) : saida/(modoExibicao*modoExibicao);
    int dezenaS = saida%(modoExibicao*modoExibicao)/(modoExibicao);
    int unidadeS = saida%(modoExibicao*modoExibicao)%(modoExibicao);
    
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[centenaS]);
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[dezenaS]);
    shiftOut(DATA_PIN_EXIBIR,CLOCK_PIN_EXIBIR,LSBFIRST,numeros[unidadeS]);
  }
  
  digitalWrite(LATCH_PIN_EXIBIR,0);
  delayMicroseconds(5);
  digitalWrite(LATCH_PIN_EXIBIR,1);
}

//Faz a leitura dos botões
unsigned long lerEntradaDados()
{
  long valorBotao = 0;//Armazena se o botão está 1 ou 0
  long bytesEntrada = 0;//Armazena os bytes que contém a informação de cada botao
  
  digitalWrite(PLOAD_PIN_LEITURA, LOW);
  delayMicroseconds(5);
  digitalWrite(PLOAD_PIN_LEITURA, HIGH);
  
  /*Este "for" é para armazenar o bit de cada botão no local correspondente da varialvel bytesEntrada 74HC165.
  OBS.: como estamos utilizando 3 shift registers o máximo que o for tem que executar é 24, pois a entrada 
  tem no maximo 3 bytes = 24 bits*/
  for(int i = 0; i < 24; i++)
  {
    valorBotao = digitalRead(DATA_PIN_LEITURA);

    // armazenando o valor do botao, 0 ou 1, no  bit correspondente em bytesEntrada.
    bytesEntrada |= (valorBotao << ((24-1) - i));
    
    digitalWrite(CLOCK_PIN_LEITURA, HIGH);
    delayMicroseconds(5);
    digitalWrite(CLOCK_PIN_LEITURA, LOW);
  }

  return(bytesEntrada);
}

//Recebe a entrada e define e retorna um número que indica a operação a ser realizada.
/*OBS.: foi criado uma variavel "e" que é uma copia de entrada pois as operações executadas no metodo
são de carater destrutivo, ou seja, alteram de forma irreversivel o valor da variavel.*/
byte operacao(unsigned long e)
{
  //operação que exclui o byte da variavel "e" que não nos interessa.
  e &= 0xFFFF;
  
  //o if verifica se o usuario não apertou mais de um botao de operação ao mesmo tempo 
  if ((e & (e - 1)) == 0)
  {
    byte c;//armazena o número que define a operação
    
    /*percorre a variavel para saber qual botão foi apertado, cada bit da variavel 
    corresponde a um botao diferente*/
    for (c = 0; e; e >>= 1)
    {
      c++;
      if (e & 1) break;/*determina onde parar o "for" pois se "e & 1" for verdadeira
      a posição do bit foi encontrada e está armazenada n variavel "c"*/
    }
    return c;
  }
  return 0;
}

//Faz a operação desejada de acordo com o botão selecionado
void calculo(byte op)
{
  switch (op)
  {
    case 0:
      return;
    case 1:// ou entre as entradas
      saida = entradaA | entradaB;
      break;
    case 2:// pega a última saida e realiza um shift left
      entradaA = 0;
      entradaB = 0;
      saida = (saida << 1) & 0xFF;
      break;
    case 3:// pega a última saida e realiza um shift right
      entradaA = 0;
      entradaB = 0;
      saida = (saida >> 1) & 0xFF;
      break;
    case 4://altera a exibição entre decimal, hexadecimal e octal
      modoExibicao = (modoExibicao == 10)? 16 : ((modoExibicao == 16)? 8 :((modoExibicao == 8)? 10 : 10));
      break;
    case 5://faz a saida ser armazenada na entradaA, para ser usada em uma proxima operação
      complemento = !complemento;
      break;
    case 6://limpa todo os dados
      entradaA = 0;
      entradaB = 0;
      saida = 0;
      break;
    case 9://soma as entradas
      saida = entradaA + entradaB;
      break;
    case 10://subtai as entradas
      saida = entradaA - entradaB;
      break;
    case 11://multiplica as entradas
      saida = entradaA * entradaB;
      break;
    case 12://divide a entrada A pela entrada B ou exibi um erro caso a divisão seja por zero
      if (entradaB)//se a entrada B for válida faz a operação
      {
        saida = entradaA / entradaB;
      } 
      else//se não zera tudo e habilita a variavel erro
      {
        erro = true;
        complemento = false;
        entradaA = 0;
        entradaB = 0;
      }
      break;
    case 13:
      if (entradaB)//se a entrada B for válida, diferente de 0, faz a operação
      {
        saida = entradaA % entradaB;
      } 
      else//se não zera tudo e habilita a variavel erro
      {
        erro = true;
        complemento = false;
        entradaA = 0;
        entradaB = 0;
      }
      break;
    case 14:
      saida = entradaA & entradaB;
      break;
    case 15:
      saida = entradaA ^ entradaB;
      break;
    case 16://Operação not no ultimo resultado da saida
      entradaA = 0;
      entradaB = 0;
      saida = ~saida;
      break;
    default:
      break;
  }
}

//Recebe o sinal do infravermelho e executa a ação desejada
byte receberInfravermelho()
{
  if(ivReceptor.decode(&codigoBotao))
  {
    if ((codigoBotao.value & 0x20DF0000) == 0x20DF0000)
    {
      switch (codigoBotao.value ^ 0x20DF0000)
      {
        //os primeiros "case" modificam as entradas
        case 0x08F7:
          entradaA = entradaA ^ 0b00000001;
          ivReceptor.resume();
          return -1;
        case 0x18E7:
          entradaA = entradaA ^ 0b00000010;
          ivReceptor.resume();
          return -1;
        case 0xA857:
          entradaA = entradaA ^ 0b00000100;
          ivReceptor.resume();
          return -1;
        case 0x48B7:
          entradaA = entradaA ^ 0b00001000;
          ivReceptor.resume();
          return -1;
        case 0xCA35:
          entradaB = entradaB ^ 0b00000001;
          ivReceptor.resume();
          return -1;
        case 0xE817:
          entradaB = entradaB ^ 0b00000010;
          ivReceptor.resume();
          return -1;
        case 0x28D7:
          entradaB = entradaB ^ 0b00000100;
          ivReceptor.resume();
          return -1;
        case 0x8877:
          entradaB = entradaB ^ 0b00001000;
          ivReceptor.resume();
          return -1;
        
        //os demais executam as operações e funções:
        case 0x827D:
          ivReceptor.resume();
          return 1;
        case 0x00FF:
          ivReceptor.resume();
          return 2;
        case 0x807F:
          ivReceptor.resume();
          return 3; 
        case 0x7887:
          ivReceptor.resume();
          return 4;
        case 0x906F:
          ivReceptor.resume();
          return 5;
        case 0x22DD:
          ivReceptor.resume();
          return 6;
        case 0x40BF:
          ivReceptor.resume();
          return 9;
        case 0xC03F:
          ivReceptor.resume();
          return 10;
        case 0x0CF3:
          ivReceptor.resume();
          return 11;
        case 0x06F9:
          ivReceptor.resume();
          return 12;
        case 0xDA25:
          ivReceptor.resume();
          return 13;
        case 0x02FD:
          ivReceptor.resume();
          return 14;
        case 0x609F:
          ivReceptor.resume();
          return 15;
        case 0xE01F:
          ivReceptor.resume();
          return 16;
        default:
          ivReceptor.resume();
          return 0;
      }
    }
    ivReceptor.resume();
  }
  return 0;
}
