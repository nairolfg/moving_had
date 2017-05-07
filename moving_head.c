#include<avr/io.h>
#include<avr/interrupt.h>



//Globale Variablen//


int red_pwm;							// variable für PWM der roten LED
int green_pwm;							// variable für PWM der roten LED
int blue_pwm;							// variable für PWM der roten LED
int servo1_pwm;							// variable für PWM der roten LED
int servo2_pwm;							// variable für PWM der roten LED

int kanal_red = 4;
int kanal_green = 5;
int kanal_blue = 6;
int kanal_servo1 = 1;
int kanal_servo2 = 2;

volatile uint8_t gDmxState;						// IDLE
int dmxkanal = 0;						// anfang der Kanäle




//ende Globale Variablen//


void datadirec_init(void){

	DDRA |=(1<<PA0)|(1<<PA1)|(1<<PA4)|(1<<PA5)|(1<<PA6);		// PA0 als Ausgang
    DDRA &=~((1<<PA2)|(1<<PA3)|(1<<PA7));						// PA0 als Eingang
	DDRD &=~(1<<PD0);
	DDRD |=(1<<2);
	PORTD &=~(1<<2);


}

void timer_init(void){
	
	TCCR0=0b00100001; 					//löscht output-compare bei Überlauf
	TIMSK|=(1<<TOIE0); 					//Interrupt bei Overflow (TCNT0 gleich OCR0 (Vergleich))
	
}


void UART_init (void){

	UBRRH = 0;
	UBRRL = 1;							// Baudrate = 250kbit/s
	UCSRA=0;
	UCSRB=(1<<RXCIE)|(1<<RXEN);			// receive interrupt enable, receive enable
	UCSRC=(1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
}


void anfangszustand(void){


	PORTA&=~((1<<PA0)|(1<<PA1)|(1<<PA4)|(1<<PA5)|(1<<PA6));
	red_pwm=0;								// festlegen der PWM Schaltschwelle 0...30 -> High, 31...255 -> Low
	green_pwm=0;							// festlegen der PWM Schaltschwelle 0...30 -> High, 31...255 -> Low									
	blue_pwm=0;							// festlegen der PWM Schaltschwelle 0...30 -> High, 31...255 -> Low
	servo1_pwm=190;							// Servo unten
	servo2_pwm=200;


}

ISR(TIMER0_OVF_vect){

 volatile static int pwm_count=0;		// zählvariable für den PWM-Schritt

	if (pwm_count<=255){				// wenn PWM_counter unter 255 ist, führe PORTA beschaltung aus
		
		if(pwm_count<=red_pwm){			// wenn PWM_counter kleiner als PWM Schaltschwelle
			PORTA|=(1<<PA4);			// schalte PA0 ein
			pwm_count++;				// inkrementiere PWM_Counter
		}
		else{
			PORTA&=~(1<<PA4);			// wenn PWM_Counter größer als PWM Schaltschwelle schalte PA0 aus
			pwm_count++;				// Inkrementiere PWM_Counter
		}
	
		if(pwm_count<=green_pwm){		// wenn PWM_counter kleiner als PWM Schaltschwelle
			PORTA|=(1<<PA5);			// schalte PA0 ein
			pwm_count++;				// inkrementiere PWM_Counter
		}
		else{
			PORTA&=~(1<<PA5);			// wenn PWM_Counter größer als PWM Schaltschwelle schalte PA0 aus
			pwm_count++;				// Inkrementiere PWM_Counter
		}

		if(pwm_count<=blue_pwm){		// wenn PWM_counter kleiner als PWM Schaltschwelle
			PORTA|=(1<<PA6);			// schalte PA0 ein
			pwm_count++;				// inkrementiere PWM_Counter
		}
		else{
			PORTA&=~(1<<PA6);			// wenn PWM_Counter größer als PWM Schaltschwelle schalte PA0 aus
			pwm_count++;				// Inkrementiere PWM_Counter
		}

		if(pwm_count<=servo1_pwm){		// wenn PWM_counter kleiner als PWM Schaltschwelle
			PORTA|=(1<<PA0);			// schalte PA0 ein
			pwm_count++;				// inkrementiere PWM_Counter
		}
		else{
			PORTA&=~(1<<PA0);			// wenn PWM_Counter größer als PWM Schaltschwelle schalte PA0 aus
			pwm_count++;				// Inkrementiere PWM_Counter
		}

		if(pwm_count<=servo2_pwm){		// wenn PWM_counter kleiner als PWM Schaltschwelle
			PORTA|=(1<<PA1);			// schalte PA0 ein
			pwm_count++;				// inkrementiere PWM_Counter
		}
		else{
			PORTA&=~(1<<PA1);			// wenn PWM_Counter größer als PWM Schaltschwelle schalte PA0 aus
			pwm_count++;				// Inkrementiere PWM_Counter
		}
		

	}
	else{
		pwm_count=0;					// wenn PWM_Counter = 255 setze ihn wieder zu 0 (eine Periode abgeschlossen)
		}

}


ISR (USART_RX_vect){
	
	cli();								// deaktivieren der Interrupts
	static uint16_t DmxCount;			// Variable für momentanen Kanal
	uint8_t USARTstate = UCSRA;			// Variable für Status des UCSRA
	uint8_t DmxByte = UDR;				// einlesen des wertes aus dem UDR
	uint8_t DmxState = gDmxState;		// DMX-Status (IDLE->0,BREAK->1,
										// STARTBYTE->2,STARTADRESSE->3)

	if(USARTstate&(1<<4)){				// schaut nach FrameError
		UCSRA &=~(1<<FE);				// setzt FrameError zu 0
		DmxCount = 1;					// 
		gDmxState = 1;					// DMX-Status -> BREAK
		return;
		}

	
	else if(DmxState==1){				// wenn DMX-Status = BREAK
		if (DmxByte==0) gDmxState = 2;	// DMX-Status -> STARTBYTE		
		else gDmxState = 0;				// IDLE
		}


	else if(DmxState==2){				// wenn DMX-Status = STARTBYTE	
		if(--DmxCount == 0){
			DmxCount = 1;
			servo1_pwm = DmxByte;		// zuweisung des Wertes zum ersten Aktor
			gDmxState = 3;				// DMX-Status -> STARTADRESSE
			}
		}

	else if(DmxState==3){
		DmxCount++;
		if(DmxCount==kanal_servo2)servo2_pwm = DmxByte;	// zuweisung zum 2. Aktor
		if(DmxCount==3);
		if(DmxCount==kanal_red)red_pwm = DmxByte;		// zuweisung zum 3. Aktor
		if(DmxCount==kanal_green)green_pwm = DmxByte;	// zuweisung zum 4. Aktor
		if(DmxCount==kanal_blue)blue_pwm = DmxByte;		// zuweisung zum 5. Aktor
		if(DmxCount>=7) gDmxState = 0;	// DMX-Status -> IDLE
		}

	sei();								// einschalten der Interrupts
	return;
}


int main(void){

	datadirec_init();					// Initialisierung der I/O ports
	timer_init();						// Timer-Initialisierung
	UART_init();						// UART-Initialisierung
	anfangszustand();					// festlegen des Anfangszustandes
	gDmxState = 0;						// DMX-Status -> IDLE
	sei();								// einschalten der Interrupts

	

	while(1){

						


	}



}
