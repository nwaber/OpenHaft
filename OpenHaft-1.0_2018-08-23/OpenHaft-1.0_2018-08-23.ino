/*
************ OpenHaft 1.0 ***************
Pressure recording knife haft for microblade experimentation
2018/08/23
Nick Waber nick.waber@telus.net

This software is released under the MIT License
Copyright (c) 2018 Nicholas Waber

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Load recording based on the Sparkfun HX711 Example sketch by Nathan Seidle

Goal: To build a haft that outputs recordable data 
regarding the force exerted on a hafted lithic edge during use.

Hardware:
Teensy 3.2 Microcontroller
HC-06 Bluetooth Module
10K Potentiometer
HX711 load cell amplifier
20kg load cell (wheatstone bridge)
3.7V 250mAh 1S LiPo battery, harness

Hardware setup:
1) HX711 connects to Teensy D2 (CLK), D3 (DAT), 3.3V (VCC), GND
2) HX711 connects to load cell:
  a) HX711 E+ (red): red wire
  b) HX711 E- (black): black wire
  c) HX711 A+ (white): white wire
  d) HX711 A- (green): green wire
3) Potentiometer connects to A8
4) HC-06:
  a) TX to d0
  b) RX to d1
  c) VCC to 3.3V
  d) GND to GND
5) Pushbutton #1 connects to Teensy 7
6) Pushbutton #2 connects to Teensy 8


Software:
Coolterm is used to collect/export serial data on Windows.  
Putty is used to collect/export serial data on Ubuntu 16.04.
*/

  #include "HX711.h"
//If errors are encountered, try using the older HX711.h library, available at https://github.com/nwaber/HX711.h-old-version  

  #define calibration_factor -121230.00 //This value is obtained using the SparkFun_HX711_Calibration sketch

  #define DOUT  3
  #define CLK  2
  const int potPin = 21;  //potentiometer input for interval adjustment on Analog port 7 (digital 21)
  const int buttonPin = 7;  //momentary pushbutton for stroke count advancement on digital port 7.
  const int buttonPin2 = 8;  //momentary pushbutton for test count advancement on digital port 8.


  HX711 scale(DOUT, CLK);

  int test_num; //experiment set number
  int stroke_num;  //individual stroke number
  
void setup() {
  
  analogReadResolution(11); // set analog bit depth resolution for potentiometer.  Teensy max resolution is 16 bit, 13 "usable"
  

/*
Resolution reference
8 bit = 256
10 bit = 1024
11 bit = 2048
12 bit = 4096
13 bit = 8192
16 bit = 65536
*/
  pinMode(buttonPin, INPUT);  //reads input from button
  pinMode(buttonPin2, INPUT); //reads input from button 2

  
  Serial1.begin(9600);      // open the hardware serial port #1 (pins d0,d1) at default 9600 bps
  //replace all references to Serial1 with Serial to use USB serial data communication

    delay(20000);       // Lets the serial monitor connect to the bluetooth before printing data



  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
    // https://codebender.cc/sketch:123175#SparkFun_HX711_Calibration.ino
  scale.tare();  //Assuming there is no weight on the scale at start up, reset the scale to 0

//set up header line for spreadsheet:

  Serial1.print("interval");  //how quickly the samples are taken, i.e. every 5 milliseconds
  Serial1.print(",");
  Serial1.print("time");    //how much time has passed in the experiment
  Serial1.print(",");
  Serial1.print("set");
  Serial1.print(",");
  Serial1.print("stroke");
  Serial1.print(",");
  Serial1.print("load g");  //force exerted on the tool edge in kg.  Multiply by 981 for N.
  Serial1.println("");
  
delay(5000);
}

void loop() {
  int potInterval;    //sample interval value mapped from intervalStep
  int intervalStep;   //interval read from potentiometer
  intervalStep = analogRead(potPin); //read potentiometer to set speed  
  potInterval = map(intervalStep, 1, 2047, 1000, 100); //low = 1 second interval, high = 100ms  
   //set to correspond with analogRead bit depth!!! 
  int buttonState;
  buttonState = digitalRead(buttonPin); 
  int button2State;
  button2State = digitalRead(buttonPin2); 

 if ((buttonState == HIGH) && (button2State == HIGH))
  {
delay(1000); //if both buttons are pressed, then delay loop.
  }
   else
   {
    
   
  Serial1.print(potInterval);  //default: potInterval
  Serial1.print(",");
  Serial1.print(millis());  //Experiment time stamp
  Serial1.print(",");
  Serial1.print(test_num);
  Serial1.print(",");
  Serial1.print(stroke_num);
  Serial1.print(",");
  Serial1.print(scale.get_units(), 2); //scale.get_units() returns a float with two decimal place precision
  //this measures to the nearest 10 grams
  //sensor accuracy is +/-0.05% of max load (20kg)
  Serial1.println("");

  if ((buttonState == HIGH) && (button2State == LOW))
  {
    stroke_num += 1;  // push the button once to advance the stroke number by one
    delay(100);   //wait 100 ms before continuing the loop
                  //prevents double-advancement if the button is not released quickly enough
  }
    else{
    stroke_num +=0;  //if the button is not pushed, the stroke number stays the same
  }

  if ((buttonState == LOW) && (button2State == HIGH))
  {
    test_num += 1;  // push the button once to advance the test number by one
    delay(100);   //wait 100 ms before continuing the loop
                  //prevents double-advancement if the button is not released quickly enough
  }
    else{
    test_num +=0;  //if the button is not pushed, the test number stays the same
  }

  if(Serial1.available())
  {
    char temp = Serial1.read();
    if(temp == '+' || temp == 'a')   //send "+" or "a" to advance the experimental set number one
      test_num += 1;
    else if(temp == '-' || temp == 'z')  //send "-" or "z" to decrease the set number one
      test_num -= 1;
  }
  delay(potInterval);  //use potentiometer to adjust delay interval
   }
}
