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
 *         File: RollTheDie_Callback.ino
 *      Created: 2018-10-22
 *       Author: Geoffrey Ottoy
 *
 *  Description: Callback-based operation of DIE SHIELD
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
 
#include "die-shield.h"

#define RELEASE_TRESHOLD 500 // ms

#define BUTTON_S1 2 // D2 (INT0)

bool buttonPressed = false;
bool newThrow = false;
bool dieShaking = false;
bool dieReleased = false;
unsigned long prevShakeTime = 0;  
unsigned int shakeCounter = 0;

/* ISR for Button S1 */
void s1Pressed(void){
  if(!newThrow){ // ignore button press when roll cycle has started
    buttonPressed = true;
  }
}

/* Callback for the Die shake event */
ShakeCallback_t iAmShaking(void){
  if(!dieReleased){ // ignore shake when roll animation has started
    dieShaking = true;
  }
}

void setup(){
  // Initialize Button S1
  pinMode(BUTTON_S1, INPUT);
  attachInterrupt(digitalPinToInterrupt(BUTTON_S1), s1Pressed, FALLING);
  
  /// TODO: Initialize Die Shield for callback-based operation
  Die.begin();
}

void loop(){
  // take time stamp
  unsigned long shakeTime = millis();
  
  // shake event has been called
  if(dieShaking){
    /// TODO: handle shake
  }
  
  // throw cycle has started
  if(newThrow){
	/// TODO: new throw handling
  }
  
  // button has been pressed (start roll animation without shaking)
  if(buttonPressed){
	/// TODO: (optionally) what happens when button is pressed
  }

}