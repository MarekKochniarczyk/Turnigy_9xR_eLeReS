/*
 * Author - Marek Kochniarczyk <mkochniarczyk@wp.pl>
 * Adapted from frsky.h code by Jean-Pierre PARISY
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

#ifndef eleres_h
#define eleres_h

	extern volatile bool date_is_recived;
	void Check_Date (void);
	void menuProc_eleres1(uint8_t event);
	void menuProc_eleres2(uint8_t event);
	void menuProc_eleres3(uint8_t event);
	void title(char x);
	void initval(uint8_t num, uint8_t pack, uint8_t val);
	void ELERES_Init(void);
	void Check_ELERES(void);
	void ELERES_EnableRXD (void);
	void menuProceLeReS(uint8_t event);
#endif

