#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <termios.h>

#define PINTRIG 29
#define PINECHO 28
 
void setup() {
        wiringPiSetup();
        pinMode(PINTRIG, OUTPUT);
        pinMode(PINECHO, INPUT);
 
        //PINTRIG pin must start LOW
        digitalWrite(PINTRIG, LOW);
        delay(30);
}
 
int getCM() {
        //Send trig pulse
        digitalWrite(PINTRIG, HIGH);
        delayMicroseconds(20);
        digitalWrite(PINTRIG, LOW);
 
        //Wait for echo start
        while(digitalRead(PINECHO) == LOW);
 
        //Wait for echo end
        long startTime = micros();
        while(digitalRead(PINECHO) == HIGH);
        long travelTime = micros() - startTime;
 
        //Get distance in cm
        int distance = travelTime / 58;
 
        return distance;
}
 
int main(void) {
        setup();
 
        printf("Distance: %dcm\n", getCM());
 

//	wiringPiSetup () ;
//	float cmMsec, inMsec;
//	while(1)
//	{
//		long microsec = ultrasonic.timing();
//		printf("Posicao: %.2f cm\n", cmMsec);
//	}
	return 0 ;
}

