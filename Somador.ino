
uint8_t byte_[2];
bool initial_config, sum_config, summed, which_byte;

void blink_led(uint8_t which, uint8_t time_ms);
void config_initial_mode(void);
void config_sum_mode(void);
void show_byte(uint8_t which);
void get_byte(uint8_t which);

void setup() {
  
  //configuração das portas como entradas ou saídas
  DDRD = 0xFF;//saídas pin0 a pin7 (PD0:PD7)
  DDRC = 0x00;//entradas A0 a A5 (PC0:PC5)
  DDRB = 0x1A;// 0001 1010  //entradas: pin8(PB0), pin10(PB2), pin13(PB5)
  //saídas:pin9(PB1), pin11(PB3), pin12(PB4),
  
  //setar as entradas como pull-up ou não e saídas em 0 ou 1
  PORTD = 0xFF; // 1111 1111; liga todos leds de saída
  PORTC = 0x00;  //0000 0000; entradas sem pull up
  PORTB = 0x26; // 0010 0110; entrada PB2 e PB5 com pull up, saída PB1 ligada

  delay(1000);
  PORTD = 0x00;//zera os leds de saída
}

void loop() {


  if(PINB & 0x20){//dipswitch pin13(PB5) OFF
  //modo pega byte

    config_initial_mode();

    get_byte(0);
    get_byte(1);

  }else{//dipswitch pin13(PB5) ON
  //modo somador 
    if(!summed){
      //execução da soma do byte completo
      uint8_t a, b, s, c_in, c_out, result = 0;

      for(uint8_t i = 0; i < 8; i++){//calcula 1bit por volta, até soma todos os 7bits de cada byte

        a = (byte_[0] >> i) & 0x01;
        b = (byte_[1] >> i) & 0x01;

        s = (a^b^c_in);
        c_out = (a & b) | (c_in & (a^b));

        c_in = c_out;
        result |= s << i;     

      }
      PORTD = result; 
    }

    config_sum_mode();

    show_byte(0);
    show_byte(1);
  }


}//loop END

//FUNÇÕES

void blink_led(uint8_t which_led, uint8_t time_ms){
  PORTB &= ~0x18;
  delay(time_ms);
  PORTB |= which_led;
  delay(time_ms);
}

void config_sum_mode(void){

  if(!sum_config){
    PORTB &= ~0x02; //0000 0010 coloca o pin9(PB1) em OFF - chave 4066 desligada
    DDRC = 0x3F;//0011 1111  A0:A5 saídas 
    DDRB = 0x1B;// 0001 1011  //pin8(PB0) saída, mantém as demais

    sum_config = true;
    summed = true;
    initial_config = false;
  }
}

void config_initial_mode(void){
   
  if(!initial_config){
    DDRC = 0x00;//entradas 
    DDRB = 0x1A;// 0001 1010  //pin8(PB0) entrada, mantém as demais
    PORTC = 0x00;  //0000 0000;
    PORTB &= ~0x01; //zera o primeiro bit pb0
    PORTB |= 0x02; //coloca o pin9(pb1) em ON - chave 4066 ligada  
    PORTD = 0x00;//zera os leds de saída

    byte_[0] = byte_[1] = 0;
    initial_config = true;
    sum_config = false;
    summed = false;
  }

}


void get_byte(uint8_t which){
    
    
    if(!(PINB & 0x20)){
      PORTD = byte_[(which + 1)%2];//representa o outro byte nos leds de saída
      PORTB &= ~0x18;//1110 0111
      PORTB |= 0x08 << which;//0000 1000//seta o led azul, para o byte_1
    }
    //mantém lendo o byte_1 enquanto a chave está solta
    while(PINB & 0x04){//0x04 é o índice do pin10(pb2), em que está ligado o botão 
      if(!(PINB & 0x20)){
        break;
        delay(100);  
      }
      byte_[which] = (PINC & 0x3F) | ((PINB & 0x01) << 6);//lê os bits de entrada
      /*
      Os 7 bits de entrada estão distribuídos entre os pins a0(pc0), a1(pc1), a2(pc2), a3(pc3), a4(pc4), a5(pc5) e o pin8(pb0)
      D8 A5 A4 A3 A2 A1 A0
        0  0  0  0  0  0  0
      */
    }
    while(!(PINB & 0x04)) blink_led((0x08 << which), 100);
    delay(100);

}


void show_byte(uint8_t which){

    if(PINB & 0x20){
      PORTB &= ~0x18;//1110 0111
      PORTB |= 0x08 << which;//0000 1000//seta o led amarelo para o byte_2
    }
    //mantém mostrando o byte_2 enquanto a chave está solta
    while(PINB & 0x04){
      if(PINB & 0x20){
        break;
        delay(100); 
      }
      PORTC = byte_[which] & 0x3F;//seta apenas os bits de A0 até A5 
      PORTB &= ~0x01; //zera o primeiro bit pb0
      PORTB |= (byte_[which] >> 6) & 0x01;//seta o pin8(pb0)  
    }
    while(!(PINB & 0x04)) blink_led((0x08 << which), 100);

}

