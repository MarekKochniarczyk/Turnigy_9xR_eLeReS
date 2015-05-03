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

#include "er9x.h"
#include "eleres.h"
const
#include "ant.lbm"

//#define VALSTR(val)  (rbuf[val][0] ? rbuf[val] : val_unknown)
#define APSIZE (BSS | DBLSIZE)


#define RX_BUFF_SIZE	64
#define MAX_COUNT_NO_FRAME 100 //200*10ms = 2000ms

uint8_t Rxbuff[RX_BUFF_SIZE];
uint8_t Rx_count;
uint16_t stat_frame;

typedef enum{
	no_date,
	date_ok,

}st;

typedef struct {
	uint8_t count_no_frame;
	st status;
	char value[10];
}dt;

#define SIZE_DT 23
dt DT[SIZE_DT];

enum {
	RSSI,	//= siła sygnału (RSSI=100)
	RCQ,	//= jakość sygnału (RCQ=100)
	URX,	//= napięcie w modelu (U=04.9V)
	T,		//= temperatura odbiornika (T=29°C)
	I,		//= prąd poboru z odbiornika
	t,		//= czas z gps (t=00:00:00)
	CH,		//= to wartosci 8 kanalow RC z nadajnika w HEX 00-FF, 80 srodek. (CH=7E82028000808080939C0000)
	P,		//= to cisnienie z czujnika baro. (P=51010)
	F,		//= to tryb lotu MuliWii. (F=04)
	Deb1,	//= to wartosci zmiennych debug z MuliWii. (Deb=00000 00000 00000 00000)
	Deb2,	//= to wartosci zmiennych debug z MuliWii. (Deb=00000 00000 00000 00000)
	Deb3,	//= to wartosci zmiennych debug z MuliWii. (Deb=00000 00000 00000 00000)
	Deb4,	//= to wartosci zmiennych debug z MuliWii. (Deb=00000 00000 00000 00000)
	Pos,	//= to pozycja GPS (Pos=00.000000N, 000.000000E)
	f,		//= to FIX GPS 0=brak, 1=2D fix, 2=3D fix (f=0)
	s,		//= to ilosc satelit (s=00)
	c,		//= to kurs w stopniach (c=021)
	v,		//= to predkosc w km/h (v=000)
	h,		//= to wysokosc w metrach (h=0000)
	UTX,	//= to napiecie baterii nadajnika RC (UTX=00.6V)
	STX,
	TTX,	//to temperatura nadajnika

};



void menuProc_eleres1(uint8_t event);
void menuProc_eleres2(uint8_t event);
void menuProc_eleres3(uint8_t event);
void menuProc_eleres4(uint8_t event);
void menuProc_eleres5(uint8_t event);
void menuProc_eleres6(uint8_t event);
void menuProc_eleres7(uint8_t event);
void menuProc_eleres8(uint8_t event);
void title(char x);
void initval(uint8_t num, uint8_t pack, uint8_t val);


//--------------------------------------------------------------------------
// Called every 10 mS in interrupt routine
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

//--------------------------------------------------------------------------
uint8_t Find_ID(const char *ptr){

	uint8_t len = strlen(ptr);

	for(uint8_t idx = 0; idx < RX_BUFF_SIZE - len; idx++){

		if( strncmp((char*)&Rxbuff[idx],ptr,len) == 0){
			return idx + len + 1;
		}
	}

	return -1;
}
//--------------------------------------------------------------------------
void Check_Date (void){

	//RSSI=100 RCQ=100 U=00.0V T=20\0xb0C P=51025 F=01 I=00.0A
	int8_t idx = 0;

	if( (idx = Find_ID("RSSI")) >= 0){

		memcpy(DT[RSSI].value,&Rxbuff[idx],3);
		DT[RSSI].value[4] = '\0';
		DT[RSSI].count_no_frame = 0;
		DT[RSSI].status = date_ok;

		if( (idx = Find_ID("RCQ")) >= 0){
			memcpy(DT[RCQ].value,&Rxbuff[idx],3);
			DT[RCQ].value[3] = '%';
			DT[RCQ].value[4] = '\0';
			DT[RCQ].count_no_frame = 0;
			DT[RCQ].status = date_ok;
		}

		if( (idx = Find_ID("U")) >= 0){
			memcpy(DT[URX].value,&Rxbuff[idx],5);
			DT[URX].value[5] = '\0';
			DT[URX].status = date_ok;
			DT[URX].count_no_frame = 0;
		}

		if( (idx = Find_ID("T")) >= 0){
			memcpy(DT[T].value,&Rxbuff[idx],2);
		}

		if( (idx = Find_ID("P")) >= 0){
			memcpy(DT[P].value,&Rxbuff[idx],5);
		}

		if( (idx = Find_ID("F")) >= 0){
			memcpy(DT[F].value,&Rxbuff[idx],2);

		}

	}else {

		//UTX=12.0V STX=092 TTX=38\0xb0C
		if( (idx = Find_ID("UTX")) >= 0){

			memcpy(DT[UTX].value,&Rxbuff[idx],4);
			DT[UTX].value[4] = 'V';
			DT[UTX].value[5] = '\0';
			DT[UTX].status = date_ok;
			DT[UTX].count_no_frame = 0;

			if( (idx = Find_ID("STX")) >= 0){
				memcpy(DT[STX].value,&Rxbuff[idx],3);
				DT[STX].status = date_ok;
			}

			if( (idx = Find_ID("TTX")) >= 0){
				memcpy(DT[TTX].value,&Rxbuff[idx],2);
				DT[TTX].status = date_ok;
			}
		}


	}
}
//--------------------------------------------------------------------------
ISR (USART0_RX_vect){

    uint8_t iostat;
    iostat = UCSR0A;

    if (iostat & ((1 << FE0) | (1 << DOR0) | (1 << UPE0))){
        Rx_count = 0;
        //if(iostat & ((1 << FE0)))stat_frame |= 0x02;
        //if(iostat & ((1 << DOR0)))stat_frame |= 0x04;
        //if(iostat & ((1 << UPE0)))stat_frame |= 0x08;
        //return;
    }

    Rxbuff[Rx_count] = UDR0;

    if(Rxbuff[Rx_count] == 0x0A && Rxbuff[Rx_count-1] == 0x0D){
    	Check_Date();
    	Rx_count = 0;
    }else if(++Rx_count >= RX_BUFF_SIZE){
    	Rx_count = 0;
   	}

}

//--------------------------------------------------------------------------
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
//--------------------------------------------------------------------------
void ELERES_DisableRXD (void){

    UCSR0B &= ~(1 << RXEN0);            // disable RX
    UCSR0B &= ~(1 << RXCIE0);           // disable Interrupt
}
//--------------------------------------------------------------------------
void ELERES_EnableRXD (void){

    Rx_count  = 0;
    for(uint8_t i =0; i < SIZE_DT;i++){
    	memcpy(DT[i].value,"??\0\0\0\0\0\0",8);
    	DT[i].status = no_date;
    	DT[i].count_no_frame = 0;
    }
    UCSR0B |=  (1 << RXEN0);		    // enable RX
    UCSR0B |=  (1 << RXCIE0);		    // enable Interrupt
}
//--------------------------------------------------------------------------
void menuProceLeReS(uint8_t event){

    menuProc_eleres1(event);
}
//--------------------------------------------------------------------------
void menuProc_eleres1(uint8_t event){
    switch(event)						// new event received, branch accordingly
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres8);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres2);
        break;
    case EVT_KEY_FIRST(KEY_MENU):
        //ELERES_DisableRXD();
        //chainMenu(menuProcStatistic);
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

    lcd_puts_P  (0*FW, 4*FH, PSTR("UTx:"));
    lcd_putsAtt(4*FW, 4*FH,DT[URX].value,BSS);

    lcd_hbar( 35, 17, 90, 14, per);
    //if(DT[RSSI].status == no_date){
    //	lcd_putsAtt(5*FW, 3*FH,PSTR("No Telemetry!!"),BLINK|DBLSIZE);
    //}


}

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
    lcd_puts_P  (1*FW, 1*FH, PSTR("RSSI    RCQ"));
    //lcd_putsAtt (2*FW, 2*FH, VALSTR(0), APSIZE);
    lcd_puts_P  (1*FW, 4*FH, PSTR("voltage   tempe") );
    //lcd_putsAtt (2*FW, 5*FH, VALSTR(1), APSIZE);
}

void menuProc_eleres3(uint8_t event)
{
    switch(event)
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres2);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres4);
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
        break;
    }

	//initval (0, PACK_T, 2);
    //initval (1, PACK_T, 3);
	title ('3');

    lcd_puts_P  (1*FW, 1*FH, PSTR(" fix | sat | K"));
    //lcd_putsAtt (2*FW, 2*FH, VALSTR(0), APSIZE);
    lcd_puts_P  (1*FW, 4*FH, PSTR("speed | height") );
    //lcd_putsAtt (2*FW, 5*FH, VALSTR(1), APSIZE);
}

void menuProc_eleres4(uint8_t event)
{
    switch(event)
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres3);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres5);
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
    break;
    }
    //initval (0, PACK_POS, CRS);
    //initval (1, PACK_POS, BER);
    title ('4');
    lcd_puts_P  (1*FW, 1*FH, PSTR(" Course"));
    //lcd_putsAtt (2*FW, 2*FH, VALSTR(0), APSIZE);
    lcd_puts_P  (1*FW, 4*FH, PSTR(" Bearing"));
    //lcd_putsAtt (2*FW, 5*FH, VALSTR(1), APSIZE);
}

void menuProc_eleres5(uint8_t event)
{
    switch(event)
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres4);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres6);
        break;
    case EVT_KEY_FIRST(KEY_MENU):
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
    break;
    }
    //initval (0, PACK_POS, WPN);
    //initval (1, PACK_POS, DST);
    title ('5');
    lcd_puts_P  (1*FW, 1*FH, PSTR(" Way Point # "));
    //lcd_putsAtt (2*FW, 2*FH, VALSTR(0), APSIZE);
    lcd_puts_P  (1*FW, 4*FH, PSTR(" Distance "));
    //lcd_putsAtt (2*FW, 5*FH, VALSTR(1), APSIZE);
}

void menuProc_eleres6(uint8_t event)
{
    switch(event)
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres5);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres7);
        break;
    case EVT_KEY_FIRST(KEY_MENU):
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
        break;
    }
    //initval (0, PACK_POS, ASP);
    //initval (1, PACK_POS, THH);
    title ('6');
    lcd_puts_P  (1*FW, 1*FH, PSTR(" Air Speed "));
    //lcd_putsAtt (2*FW, 2*FH, VALSTR(0), APSIZE);
    lcd_puts_P  (1*FW, 4*FH, PSTR(" Climb Rate "));
    //lcd_putsAtt (2*FW, 5*FH, VALSTR(1), APSIZE);
}

void menuProc_eleres7(uint8_t event)
{
    switch(event)
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres6);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres8);
        break;
    case EVT_KEY_FIRST(KEY_MENU):
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
    break;
    }
    //initval (0, PACK_POS, RLL);
    //initval (1, PACK_POS, PCH);
    title ('7');
    lcd_puts_P  (1*FW, 1*FH, PSTR(" Roll Angle"));
    //lcd_putsAtt (2*FW, 2*FH, VALSTR(0), APSIZE);
    lcd_puts_P  (1*FW, 4*FH, PSTR(" Pitch Angle"));
    //lcd_putsAtt (2*FW, 5*FH, VALSTR(1), APSIZE);
}

void menuProc_eleres8(uint8_t event)
{
    switch(event)
    {
    case EVT_KEY_FIRST(KEY_UP):
        chainMenu(menuProc_eleres7);
        break;
    case EVT_KEY_FIRST(KEY_DOWN):
        chainMenu(menuProc_eleres1);
        break;
    case EVT_KEY_FIRST(KEY_MENU):
        break;
    case EVT_KEY_FIRST(KEY_EXIT):
        ELERES_DisableRXD();
        chainMenu(menuProc0);
        break;
    }
    //initval (0, PACK_POS, RLL);
    //initval (1, PACK_POS, PCH);
    title ('8');
    lcd_puts_P  (1*FW, 1*FH, PSTR(" eLeReS Mode"));
    //lcd_putsAtt (2*FW, 2*FH, VALSTR(0), APSIZE);
    lcd_puts_P  (1*FW, 4*FH, PSTR(" RTL Distance"));
    //lcd_putsAtt (2*FW, 5*FH, VALSTR(1), APSIZE);
}

void title(char x){

	uint8_t stat = 0;
	if(DT[RSSI].status == no_date){
		stat = BLINK;
	}

    lcd_putsAtt (0, 0, PSTR(" eLeReS Telemetry ?/8"),stat);
    lcd_putcAtt(18*FW, 0*FH, x, stat);
    lcd_hline(0,0,128);
}

void initval(uint8_t num, uint8_t pack, uint8_t val)
{
	/*
    rbuf[0][0] = '1';
    rbuf[1][0] = '2';

    return;
    if (xpack[num] != pack || xval[num] != val)
    {
        ibuf[num] = rbuf[num][0] = 0;
		rbuf[num][0]=0; //zerowanie bufora
        xpack[num] = pack;
        xval[num] = val;
        state = WAIT_PACKET;			// synchronize to the next packet
    }
    */
}
