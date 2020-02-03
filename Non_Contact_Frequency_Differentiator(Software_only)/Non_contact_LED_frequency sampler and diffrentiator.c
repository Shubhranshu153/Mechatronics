#include "teensy_general.h" 
#include "t_usb.h" 
#include <avr/io.h>

int main(void)
{
	
	teensy_clockdivide(0);
	
	clear(DDRF,5); //Input pin for recording the signal we clear DDRF5. 
	
	set(DDRB,6);//230 Hz DDRB,6 set to output used for PWM
	set(DDRD,0);//1.6 KHz DDRD,0 set to output used for PWM
    set(DDRC,7);//625 Hz; DDRC,7 set to output used for PWM
	set(DDRF,0); //Blue for 1.6 KHz sets DDRF0 output lights up the blue LED
    set(DDRF,1); //yellow for 625 Hz set DDRF1 to output lights up the yellow LED
	set(DDRB,1);//Red for 230Hz sets up DDRB1 to output lights up the red LED
	
	m_usb_init(); //initialization

	clear(TCCR3B,CS31);set(TCCR3B,CS30);clear(TCCR3B,CS32); //clock speed /1 as this is a detector timer we want is to run fast. 
	clear(TCCR3B,WGM33);clear(TCCR3B,WGM32);clear(TCCR3A,WGM30);clear(TCCR3A,WGM31);// up to 0xFFFF
	
	set(TCCR1B,CS11);clear(TCCR1B,CS10);clear(TCCR1B,CS12); //clock speed /8
	set(TCCR1B,WGM13);set(TCCR1B,WGM12);set(TCCR1A,WGM10);set(TCCR1A,WGM11);// up to OCR1A PWM mode 15
	OCR1A = 8695; // calculated for 230 Hz
	OCR1B=OCR1A/2;//50% duty cycle
	set(TCCR1A,COM1B1);clear(TCCR1A,COM1B0);//clear at OCR1B and set at rollover for B6
	
	clear(TCCR0B,CS01);clear(TCCR0B,CS00);set(TCCR0B,CS02); //clock speed  /256 as it is a 8 bit timer we have limitation on value that 
	set(TCCR0B,WGM02);set(TCCR0A,WGM00);set(TCCR0A,WGM01); //Upto OCR0A PWM for 1.6 KHz.
	OCR0A=40;// calculated 
	OCR0B=20;//50% duty cycle
	set(TCCR0A,COM0B1);clear(TCCR0A,COM0B0);//clear at OCR0B and set at rollover D0
	
	set(TCCR4B,CS41);set(TCCR4B,CS40);set(TCCR4B,CS42);clear(TCCR4B,CS43);  //clock speed /8
	// default mode is Upto OCR4C so not initialized
    OCR4C = 200;// calculated
	OCR4A = 200;//50% duty cycle
	clear(TCCR4A,PWM4A);clear(TCCR4A,COM4A1);set(TCCR4A,COM4A0);//toggle at OCR4A and set at rollover C7

	
	int frq1 =230; // these 3 frq can be user inputs
	int frq2 = 625;
	long frq3 = 1600;
	
	int cv,lv; // for storing current and last value
    lv=0;
	cv=0;
	int i; // for the controlling for loop
	long counter =0;
	while(1)
	{
	for(i=1;i<=400;i++) 
	{		
   	while(TCNT3<20000) //combined with for loop gives a time of 1/2 a second of evaluating the signal can be lower but will be inaccurate for close frequency
    {
	    
	    
         cv =bit_is_set(PINF,5); // stores current val of PIN F,5 
		 if(lv!=cv) // if not equal means a edge has crossed so we update counter. 
			 counter++;
		 lv=cv; // stores val for next iteration

	}
	TCNT3=0;//reinitialized for next looping 
	}
	

				m_usb_tx_long(counter);
				  m_usb_tx_char('\n');
	

			 if(counter<frq1+10 && counter>frq1-10) // when only 230 Hz is present
			 {
				m_usb_tx_int(230);
				m_usb_tx_char('\n');
				set(PORTB,1); // activates PORTB1 to glow the red led
				clear(PORTF,0);//Disables Port F0 to shut off blue light
				clear(PORTF,1);//Disables Port F1 to shut off yellow light
			 }
			 else if(counter>frq2-10 && counter <frq2+10)// when only 625Hz is present
			 {
				m_usb_tx_int(625);
				  m_usb_tx_char('\n');
				 clear(PORTB,1); //Disables Port B1 to shut off red light
				clear(PORTF,0);//Disables Port F0 to shut off blue light
				set(PORTF,1); //activates PORTF1 to glow the yellow light
                
			 }
			 else if(counter>frq3-100 && counter<frq3+100)// when only 1.6KHz is present
			 {
			 m_usb_tx_int(1600);
			   m_usb_tx_char('\n');
			    clear(PORTB,1);//Disables Port B1 to shut off red light
				set(PORTF,0); //activates PORTF1 to glow the blue light
				clear(PORTF,1);//Disables Port F1 to shut off yellow light
				
			   
			 }
			 else if(counter<((frq2+frq1)/2)+50 && counter>  ((frq1+frq2)/2)-50) //both 230 and 625 Hz presnt
			 {
		        m_usb_tx_int(230);
				  m_usb_tx_char('\n');
				m_usb_tx_int(625);
				  m_usb_tx_char('\n');
				  set(PORTB,1);// activates PORTB1 to glow the red led
				clear(PORTF,0);//Disables Port F0 to shut off blue light
				set(PORTF,1);//activates PORTF1 to glow the yellow light
			 }
			 
			 else if(counter<(frq1/2) + (frq3/2)+100 && counter> (frq1/2) + (frq3/2) -100) //both 230 and 1.6 KHz
			 {
		        m_usb_tx_int(230);
				m_usb_tx_char('\n');
				m_usb_tx_int(1600);
				m_usb_tx_char('\n');
				 set(PORTB,1);// activates PORTB1 to glow the red led
				set(PORTF,0);//activates PORTF1 to glow the yellow light
				clear(PORTF,1);//Disables Port F1 to shut off yellow light
			 }
			 
			 else if(counter<(frq2/2) + (frq3/2)+100 && counter> (frq2/2) + (frq3/2) -100) //both 625 and 1.6 KHz
			 {
		        m_usb_tx_int(625);
				  m_usb_tx_char('\n');
				m_usb_tx_int(16);
				  m_usb_tx_char('\n');
				  clear(PORTB,1);//Disables Port B1 to shut off red light
				set(PORTF,0);//activates PORTF1 to glow the yellow light
				set(PORTF,1);//activates PORTF1 to glow the yellow light
			 }
			  else if(counter<(frq1/4) + (frq2/4) + (frq3/4) + 50 && counter>(frq1/4) + (frq2/4) + (frq3/4) -50)// 625, 230 and 16
			 {
		        m_usb_tx_int(625);
				  m_usb_tx_char('\n');
				m_usb_tx_int(230);
				  m_usb_tx_char('\n');
				m_usb_tx_int(16);
				 m_usb_tx_char('\n');
				  set(PORTB,1);// activates PORTB1 to glow the red led
				set(PORTF,0);//activates Port F0 to shut off blue light
				set(PORTF,1);//activates PORTF1 to glow the yellow light
			 }
			 else
			 {
				  clear(PORTB,1);//Disables Port B1 to shut off red light
				clear(PORTF,0);//Disables Port F0 to shut off blue light
				clear(PORTF,1); //Disables Port F1 to shut off yellow light
			 }
			 
		 counter=0; //reinitialized for next iteration
		 TCNT3=0;  //reinitialized for next iteration so that no roll over happens and we get consistent values. 
	}
	
	return 0;
	}
		 
         		 
      
	

 