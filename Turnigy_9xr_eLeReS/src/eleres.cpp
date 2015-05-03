/*
 *
 * Author - Marek Kochniarczyk <mkochniarczyk@wp.pl>
 * Author - Uphiearl and Jean-Pierre PARISY
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "inc/er9x.h"
#include "inc/eleres.h"
const
#include "lbm/ant.lbm"


#define RX_BUFF_SIZE	80
#define MAX_COUNT_NO_FRAME 100 //200*10ms = 2000ms

uint8_t Rxbuff[RX_BUFF_SIZE];
uint8_t Rx_count;
uint16_t stat_frame;
volatile bool date_is_recived;

typedef enum{
	no_date,
	date_ok,

}st;

typedef struct {
	uint8_t count_no_frame;
	st status;
	char value[12];
}dt;

#define SIZE_DT 23
dt DT[SIZE_DT];

enum {

	//RSSI=100 RCQ=100 U=01.5V T=13\0xb0C P=51007 F=01 I=00.0A
	RSSI,	//= siła sygnału (RSSI=100)
	RCQ,	//= jakość sygnału (RCQ=100)
	VRX,	//= napięcie w modelu (U=04.9V)
	Temp,	//= temperatura odbiornika (T=29°C)
	P,		//= to cisnienie z czujnika baro. (P=51010)
	F,		//= to tryb lotu MuliWii. (F=04)
	Amp,		//= prąd poboru z odbiornika


	CH,		//= to wartosci 8 kanalow RC z nadajnika w HEX 00-FF, 80 srodek. (CH=7E82028000808080939C0000)

	Deb1,	//= to wartosci zmiennych debug z MuliWii. (Deb=00000 00000 00000 00000)
	Deb2,	//= to wartosci zmiennych debug z MuliWii. (Deb=00000 00000 00000 00000)
	Deb3,	//= to wartosci zmiennych debug z MuliWii. (Deb=00000 00000 00000 00000)
	Deb4,	//= to wartosci zmiennych debug z MuliWii. (Deb=00000 00000 00000 00000)

	//Pos=50.319569N, 019.153139E HD=00.0
	Pos_LAT,//= to pozycja GPS 00.000000N, latitude
	Pos_LON,//= to pozycja GPS 00.000000N, longtitude
	HDOP,	//= to Horizontal Dilution of Precision

	//t=18:19:45 f=1 s=04 c=279 v=000 h=0273
	time,		//= czas z gps (t=00:00:00)
	fix,		//= to FIX GPS 0=brak, 1=2D fix, 2=3D fix (f=0)
	sat,		//= to ilosc satelit (s=00)
	course,		//= to kurs w stopniach (c=021)
	speed,		//= to predkosc w km/h (v=000)
	alt,		//= to wysokosc w metrach (h=0000)

	//UTX=11.7V STX=100 TTX=28\0xb0C
	VTX,	//= to napiecie baterii nadajnika RC (UTX=00.6V)
	STX,
	TTX,	//to temperatura nadajnika

};




/*------------------------------------------------------------------------*//**
* \brief Called every 10 mS in interrupt routine
*//*-------------------------------------------------------------------------*/
void Check_ELERES(void){

	for(uint8_t i = 0; i < SIZE_DT;i++){

		if(DT[i].count_no_frame >= MAX_COUNT_NO_FRAME){
			memcpy(DT[i].value,"??\0\0\0\0\0\0",8);
			DT[i].status = no_date;
		}else{
			DT[i].count_no_frame++;
		}
	}
}

/*------------------------------------------------------------------------*//**
* \brief Check frame form eLeReS
* \param pointer to finding string
*//*-------------------------------------------------------------------------*/
uint8_t Find_ID(const char *ptr){

	uint8_t len = strlen(ptr);

	if( strncmp((char*)Rxbuff,ptr,len) == 0){
		return 1;
	}
	return 0;
}
/*------------------------------------------------------------------------*//**
* \brief Check frame form eLeReS
*//*-------------------------------------------------------------------------*/
void Check_Date (void){

	//RSSI=100 RCQ=100 U=01.5V T=13\0xb0C P=51007 F=01 I=00.0A
	//Pos=50.319569N, 019.153139E HD=00.0
	//t=18:19:45 f=1 s=04 c=279 v=000 h=0273
	//CH=8080007F0000000080808080
	//UTX=11.7V STX=100 TTX=28\0xb0C
	//Deb= 00000  00000  00000  00000
	date_is_recived = false;
	char *ptr = (char*)&Rxbuff;

	//RSSI=100 RCQ=100 U=01.5V T=13\0xb0C P=51007 F=01 I=00.0A
	if(Find_ID("RSSI")){

		memcpy(DT[RSSI].value,ptr+5,3);
		memcpy(DT[RCQ].value,ptr+13,3);
		DT[RCQ].value[3] = '%';
		memcpy(DT[VRX].value,ptr+19,5);
		//memcpy(DT[Temp].value,&Rxbuff[idx],2);
		//memcpy(DT[P].value,&Rxbuff[idx],5);
		//memcpy(DT[F].value,&Rxbuff[idx],2);

		for(uint8_t i = RSSI; i < Amp+1; i++){
			DT[i].count_no_frame = 0;
			DT[i].status = date_ok;
		}

	}else //UTX=12.0V STX=092 TTX=38\0xb0C
		if( Find_ID("UTX")){

			memcpy(DT[VTX].value,ptr+3,4);
			memcpy(DT[STX].value,ptr+10,3);
			memcpy(DT[TTX].value,ptr+18,2);
			for(uint8_t i = VTX; i < TTX+1; i++){
				DT[i].count_no_frame = 0;
				DT[i].status = date_ok;
			}

	}else//Pos=50.319569N, 019.153139E HD=00.0

		if( Find_ID("Pos") ){

			memcpy(DT[Pos_LAT].value,ptr+4,10);
			memcpy(DT[Pos_LON].value,ptr+16,11);
			memcpy(DT[HDOP].value,ptr+31,4);
			for(uint8_t i = Pos_LAT; i < HDOP+1; i++){
				DT[i].count_no_frame = 0;
				DT[i].status = date_ok;
			}

	}else//t=18:19:45 f=1 s=04 c=279 v=000 h=0273
		if( Find_ID("t=")){

			memcpy(DT[time].value,ptr+2,8);
			memcpy(DT[fix].value,ptr+13,1);DT[fix].value[1] = '\0';
			memcpy(DT[sat].value,ptr+17,2);
			memcpy(DT[course].value,ptr+22,3);
			memcpy(DT[speed].value,ptr+28,3);
			memcpy(DT[alt].value,ptr+34,4);

			for(uint8_t i = time; i < alt+1; i++){
				DT[i].count_no_frame = 0;
				DT[i].status = date_ok;
			}
	}

	Rx_count = 0;

}
/*------------------------------------------------------------------------*//**
* \brief Interrupt form USART0
*//*-------------------------------------------------------------------------*/
ISR (USART0_RX_vect){

    uint8_t iostat;
    iostat = UCSR0A;

    if (iostat & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))){
        Rx_count = 0;
    }

    Rxbuff[Rx_count] = UDR0;

    if(Rxbuff[Rx_count] == 0x0A && Rxbuff[Rx_count-1] == 0x0D){
    	date_is_recived = true;
    }else if(++Rx_count >= RX_BUFF_SIZE){
    	Rx_count = 0;
   	}

}

/*------------------------------------------------------------------------*//**
* \brief Initialize port
*//*-------------------------------------------------------------------------*/
void ELERES_Init (void){

	stat_frame = 0;
    DDRE  &= ~(1 << DDE0);              // set RXD0 pin as input
    PORTE &= ~(1 << PORTE0);            // disable pullup on RXD0 pin

    #undef BAUD
    #define BAUD 58823
    #include <util/setbaud.h>

    UCSR0A &= ~(1 << U2X0); // disable double speed operation.
    UBRR0L = UBRRL_VALUE;
    UBRR0H = UBRRH_VALUE;

    // set 8N1
    UCSR0B = 0 | (0 << RXCIE0) | (0 << UDRIE0) | (0 << RXEN0) | (0 << UCSZ02);
    UCSR0C = 0 | (1 << UCSZ01) | (1 << UCSZ00);
    while (UCSR0A & (1 << RXC0)) UDR0; // flush receive buffer
    stat_frame  |= 0x01;

}
/*------------------------------------------------------------------------*//**
* \brief OFF POrt
*//*-------------------------------------------------------------------------*/
void ELERES_DisableRXD (void){

    UCSR0B &= ~(1 << RXEN0);            // disable RX
    UCSR0B &= ~(1 << RXCIE0);           // disable Interrupt
}
/*------------------------------------------------------------------------*//**
* \brief On port USART and zeroing date
*//*-------------------------------------------------------------------------*/
void ELERES_EnableRXD (void){

    Rx_count  = 0;
    for(uint8_t i =0; i < SIZE_DT;i++){
    	memcpy(DT[i].value,"??\0\0\0\0\0\0\0\0\0\0",12);
    	DT[i].status = no_date;
    	DT[i].count_no_frame = 0;
    }
    UCSR0B |=  (1 << RXEN0);		    // enable RX
    UCSR0B |=  (1 << RXCIE0);		    // enable Interrupt
}
/*------------------------------------------------------------------------*//**
* \brief
* \param event key
*//*-------------------------------------------------------------------------*/
void menuProceLeReS(uint8_t event){

    menuProc_eleres1(event);
}
/*------------------------------------------------------------------------*//**
* \brief Display screen 1
* \param event key
*//*-------------------------------------------------------------------------*/
void menuProc_eleres1(uint8_t event){
    switch(event)						// new event received, branch accordingly
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres3);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres2);
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
        break;
    }


    title('1');
    lcd_img(0,2*FH,ant,0);
    lcd_putsAtt(10, 2*FH,DT[RSSI].value,BSS);
    lcd_putsAtt(10, 3*FH,DT[RCQ].value,BSS);
    uint8_t per = 0;
    if(DT[RCQ].status == date_ok){
    	per = atoi(DT[RCQ].value);
    }

    lcd_puts_P  (0*FW, 4*FH, PSTR("VRx:"));
    lcd_putsAtt(4*FW, 4*FH,DT[VRX].value,BSS);

    lcd_hbar( 35, 17, 90, 14, per);

}

/*------------------------------------------------------------------------*//**
* \brief Display screen 2
* \param event key
*//*-------------------------------------------------------------------------*/
void menuProc_eleres2(uint8_t event)
{
    switch(event)
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres1);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres3);
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
        break;
    }
    //initval (0, PACK_RSSI, 1);
    //initval (1, PACK_RSSI, 3);
    title ('2');
    lcd_puts_P  (1*FW, 2*FH, PSTR("LATIT.:"));
    lcd_putsAtt (10*FW, 2*FH, DT[Pos_LAT].value,BSS);
    lcd_puts_P  (1*FW, 3*FH, PSTR("LONG. :"));
    lcd_putsAtt (10*FW, 3*FH, DT[Pos_LON].value,BSS);

    lcd_hline(0,25,128);

    lcd_puts_P  (1*FW, 5*FH, PSTR("TIME:") );
    lcd_putsAtt (7*FW, 5*FH, DT[time].value,BSS);
    lcd_puts_P  (1*FW, 6*FH, PSTR("HDOP:"));
    lcd_putsAtt (7*FW, 6*FH, DT[HDOP].value,BSS);
}
/*------------------------------------------------------------------------*//**
* \brief Display screen 3
* \param event key
*//*-------------------------------------------------------------------------*/
void menuProc_eleres3(uint8_t event)

{
    switch(event)
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres2);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres1);
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
        break;
    }

	title ('3');

    lcd_puts_P  (FW, 2*FH, PSTR(" FIX | SAT | COURSE"));
    lcd_putsAtt  (3*FW, 3*FH, DT[fix].value,BSS);
    lcd_putsAtt  (8*FW, 3*FH, DT[sat].value,BSS);
    lcd_putsAtt  (15*FW, 3*FH, DT[course].value,BSS);

    lcd_hline(0,25,128);

    lcd_puts_P  (2*FW, 5*FH, PSTR("SPEED | ALTITUDE") );
    lcd_putsAtt  (3*FW, 6*FH, DT[speed].value,BSS);
    lcd_putsAtt  (12*FW, 6*FH, DT[alt].value,BSS);

}
/*------------------------------------------------------------------------*//**
* \brief Display title screen
* \param number screen
*//*-------------------------------------------------------------------------*/
void title(char x){

	uint8_t stat = 0;
	if(DT[RSSI].status == no_date){
		stat = BLINK;
	}
    lcd_putsAtt (0, 0, PSTR(" eLeReS Telemetry ?/3"),stat);
    lcd_putcAtt(18*FW, 0*FH, x, stat);
    lcd_hline(0,0,128);
}
