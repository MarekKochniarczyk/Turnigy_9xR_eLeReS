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
	void ELERES_Init(void);
	void Check_ELERES(void);
	void ELERES_EnableRXD (void);
	void menuProceLeReS(uint8_t event);
#endif

