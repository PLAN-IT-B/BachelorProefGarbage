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
 *         File: die-shield.cpp
 *      Created: 2018-10-21
 *       Author: Geoffrey Ottoy
 *
 *  Description: Implementation of the DieShield class. This class offers an API to
 *               the DRAMCO DIE SHIELD.
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
 
#include <Arduino.h>
#include "die-shield.h"

/*****************************************************************************
 * Global symbols                                                            *
 *****************************************************************************/

// preinstatiate
DieShield Die;
static bool dsInitialized = false;

volatile int sampleCtr;
volatile bool shaking;
static ShakeCallback_t scb;

// Convert value to the die display format.
uint8_t ledLUT[8][7]={
// Position on the die shield:	
//
//         4       3
//
//         5   0   2
//
//         6       1	
//
// LED 6 5 4 3 2 1 0 	
      {0,0,0,0,0,0,0}, // NONE
      {0,0,0,0,0,0,1}, // ONE
      {1,0,0,1,0,0,0}, // TWO
      {1,0,0,1,0,0,1}, // THREE
      {1,0,1,1,0,1,0}, // FOUR
      {1,0,1,1,0,1,1}, // FIVE
      {1,1,1,1,1,1,0}, // SIX
      {1,1,1,1,1,1,1}, // ALL
};

// Timer 1 compare match Interrupt Service Routine (fires every 5 ms).
ISR(TIMER1_COMPA_vect){
	// Sample ADXL335
	sampleCtr++;
	Die.readADXL335();
	
	///TODO: include callback
	
	// Detect Shake
	if(sampleCtr==BUFFER_SIZE){
		sampleCtr=0;
		if(Die.detectShake() == DS_SHAKING){
			shaking = true;
		}
		else{
			shaking = false;
		}
	}
	
}

/*****************************************************************************
 * Public methods                                                            *
 *****************************************************************************/

/* Constructor (set pin numbers)
 */
DieShield::DieShield(){
	// Set pin numbers
	this->clkPin = DEFAULT_CLK_PIN;
	this->led0Pin = DEFAULT_LED0_PIN;
	this->xAxisPin = DEFAULT_X_AXIS_PIN;
	this->yAxisPin = DEFAULT_Y_AXIS_PIN;
	this->zAxisPin = DEFAULT_Z_AXIS_PIN;
}

/* Begin operation of the die and do not assign a shake callback.
 */
DieStatus_t DieShield::begin(){
	return this->init(NULL);
}

/* Begin operation of the die and assign a shake callback.
 */
 /// TODO: overload begin() method to pass ShakeCallback_t to init()


/* Return if a shake has been detected. 
 */
bool DieShield::isShaking(){
	return shaking;
}

/* Generate random value and show
 */
void DieShield::roll(){
	DieValue_t value = (DieValue_t) random(1,7);
	this->show(value);
}

/* Roll animation
 */
void DieShield::roll(int nrShakes){
	int i;
	int rem = (nrShakes-3)*4;
	if(nrShakes>3){
		for(i=0;i<rem;i++){
			this->roll();
			delay(100);
		}
	}
	for(i=0;i<4;i++){
		this->roll();
		delay(200);
	}
	this->roll();
	delay(300);
	this->roll();
	delay(300);
	this->roll();
	delay(600);
	
	// Done rolling show final value
	DieValue_t value = (DieValue_t) random(1,7);
	this->show(value);
	delay(1000);
	this->show(NONE);
	delay(400);
	this->show(value);
	delay(200);
	this->show(NONE);
	delay(200);
	this->show(value);
}

/* Show a value on the LEDs of the die shield.
 */
void DieShield::show(DieValue_t value){
	/// TODO: display value using the shield's LEDs
	uint8_t* rij = ledLUT[value];
	
	for(uint8_t i = 0 ; i< 7; i++ ){
		digitalWrite(DEFAULT_LED0_PIN,rij[i]);
		if(i!=6){
			digitalWrite(DEFAULT_CLK_PIN,1);
			digitalWrite(DEFAULT_CLK_PIN,0);
		}	
	}

	

}

/* Detect a shake
 *  If the average acceleration (in any direction) is higher than the previous average +/- a given
 *  threshold, then a shake is detected.
 */
DieStatus_t DieShield::detectShake(void){
	bool shakeDetected = false;

	float xAvg = this->x.getAverage();
	float yAvg = this->y.getAverage();
	float zAvg = this->z.getAverage();
	// detect shake
	if(abs(xAvg - this->prevX) > this->sensitivity){
		// shake detected
		shakeDetected = true;
	}
	if(abs(yAvg - this->prevY) > this->sensitivity){
		// shake detected
		shakeDetected = true;
	}
	if(abs(zAvg - this->prevZ) > this->sensitivity){
		// shake detected
		shakeDetected = true;
	}
	// update previous value
	this->prevX = xAvg;
	this->prevY = yAvg;
	this->prevZ = zAvg;
	// return shaking / not shaking
	if(shakeDetected){
		return DS_SHAKING;
	}
	
	return DS_NOT_SHAKING;
}

/* Set the "shake detect" sensitivity
 */
void DieShield::setSensitivity(float s){
	this->sensitivity = s;
}

/* Read the "shake detect" sensitivity
 */
float DieShield::getSensitivity(){
	return this->sensitivity;
}

/*****************************************************************************
 * Private methods                                                           *
 *****************************************************************************/

/* Initialize die-shield operation:
 *  set shake callback
 *  initialize sampling buffers
 *  initialize pins
 *  initialize Timer1 to 200 Hz
 */
DieStatus_t DieShield::init(ShakeCallback_t cb){
	// Avoid more than one instance
	if(dsInitialized){
		return DS_INIT_ERROR;
	}
	dsInitialized = true;
	
	/// TODO:
	// Attach callback
		// check if callback has not been set previously

		// set callback
		
	
	// Initialize attributes
	sampleCtr=0;
	shaking = false;
	//this->rolling = false;
	this->sensitivity = DEFAULT_SENSITIVITY;
	
	// Initialize random generator
	randomSeed(analogRead(A5));
	
	// Initialize measurement buffers
	if(this->initBuffers(BUFFER_SIZE) != CB_SUCCESS){
		return DS_INIT_ERROR;
	}
	
	// Initialize pins
    this->pinsInit();
	
	// Initialize timer 1
	this->timerOneInit();
	
	return DS_SUCCESS;
}

/* Initialize pins used by the die-shield
 */
void DieShield::pinsInit(){
	// ADXL335 (analog) inputs
	pinMode(this->xAxisPin, INPUT);
    pinMode(this->yAxisPin, INPUT);
    pinMode(this->zAxisPin, INPUT);
	
	// LED control
	pinMode(this->clkPin, OUTPUT);
	pinMode(this->led0Pin, OUTPUT);
	
	// Init clock line (LOW)
	digitalWrite(this->clkPin, LOW);
}

/* Initialize Timer1 (compare interrupt every 5 ms)
 */
void DieShield::timerOneInit(){
#if defined(__AVR__)
	cli();
	//set timer1 interrupt at 200 Hz
	TCCR1A = 0; // set entire TCCR1A register to 0
	TCCR1B = 0; // same for TCCR1B
	TCNT1  = 0; //initialize counter value to 0
	
	/// TODO: set compare match register for 200 Hz increments
	OCR1A = 9999;
	
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// set CS11 bit for /8 prescaler
	TCCR1B |= (1 << CS11);  
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);
	
	sei();
#else
	#error "Only AVR controllers are supported (for now)."
#endif
}

/* Initialize circular buffers for storing ADXL335 measurements
 */
CircBufferStatus_t DieShield::initBuffers(uint8_t size){
	CircBufferStatus_t status = this->x.init(size);
	if(status != CB_SUCCESS){
		return status;
	}
	status = this->y.init(size);
	if(status != CB_SUCCESS){
		return status;
	}
	status = this->z.init(size);
	if(status != CB_SUCCESS){
		return status;
	}
	return CB_SUCCESS;
}

/* Read x,y and z acceleration (analog voltage) from ADXL335 and store in circular buffer.
 */
void DieShield::readADXL335(void){
	// Read ADXL335 voltages
	float x_val = (float)5*analogRead(this->xAxisPin)/1024;
	float y_val = (float)5*analogRead(this->yAxisPin)/1024;
	float z_val = (float)5*analogRead(this->zAxisPin)/1024;
	
	// Store measurements into buffer
	this->x.put(x_val);
	this->y.put(y_val);
	this->z.put(z_val);
}