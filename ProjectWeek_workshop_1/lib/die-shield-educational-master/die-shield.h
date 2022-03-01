/*  ____  ____      _    __  __  ____ ___
 * |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
 * | | | | |_) |  / _ \ | |\/| | |  | | | |
 * | |_| |  _ <  / ___ \| |  | | |__| |_| |
 * |____/|_| \_\/_/   \_\_|  |_|\____\___/
 *                           research group
 *                             dramco.be/
 *
 *  KU Leuven - Technology Campus Gent,
 *  Gebroeders De Smetstraat 1,
 *  B-9000 Gent, Belgium
 *
 *         File: die-shield.h
 *      Created: 2018-10-21
 *       Author: Geoffrey Ottoy
 *
 *  Description: Header file for the DieShield class.
 *
 * MIT License
 * 
 * Copyright (c) 2018 DRAMCO - KU Leuven
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#ifndef __DIE_SHIELD_H__
#define __DIE_SHIELD_H__

#include <Arduino.h>
#include "circ-buffer.h"

#define DEFAULT_SENSITIVITY 0.25
#define BUFFER_SIZE         20

#define DEFAULT_CLK_PIN     7
#define DEFAULT_LED0_PIN    4

#define DEFAULT_X_AXIS_PIN  A0
#define DEFAULT_Y_AXIS_PIN  A1
#define DEFAULT_Z_AXIS_PIN  A2

typedef void (*ShakeCallback_t)();

typedef enum dieStatuses{DS_SUCCESS, DS_INIT_ERROR, DS_NOT_NULL, DS_SHAKING, DS_NOT_SHAKING} DieStatus_t;
typedef enum dieValues{NONE=0, ONE=1, TWO=2, THREE=3, FOUR=4, FIVE=5, SIX=6, ALL=7} DieValue_t;

class DieShield{
	public:
		DieShield();
	
		DieStatus_t begin(void);
		DieStatus_t begin(ShakeCallback_t cb);
		//DieStatus_t attachShakeCallback(ShakeCallback_t cb);
			
		void roll(void);
		void roll(int nrShakes);
		void show(DieValue_t value);
		bool isShaking();
		
		void setSensitivity(float s);
		float getSensitivity();

		DieStatus_t detectShake(void);
		void readADXL335(void);

	private:
		DieStatus_t init(ShakeCallback_t cb);
		void pinsInit(void);
		void timerOneInit(void);
		CircBufferStatus_t initBuffers(uint8_t size);
		
		CircBuffer x;
		CircBuffer y;
		CircBuffer z;
		
		float prevX;
		float prevY;
		float prevZ;
		
		float sensitivity;
		
		// pin numbers
		uint8_t clkPin;
		uint8_t led0Pin;
		uint8_t xAxisPin;
		uint8_t yAxisPin;
		uint8_t zAxisPin;
		
};

extern DieShield Die;

#endif /*__DIE_SHIELD_H__*/
