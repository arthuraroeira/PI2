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
#include <Stepper.h>

#define MOTOR1 21
#define MOTOR2 22
#define MOTOR3 23
#define MOTOR4 24
#define MOTOR5 0
#define MOTOR6 1

int main (int argc, char **argv)
{
	Stepper myStepper(200, MOTOR1, MOTOR2, MOTOR3, MOTOR4); 
	pinMode(MOTOR5, OUTPUT); // Pinos 3 e 4 configurados como saída
	pinMode(MOTOR6, OUTPUT);
	digitalWrite(MOTOR5, HIGH);
	digitalWrite(MOTOR6, HIGH);
	myStepper.setSpeed(60);
	myStepper.step(200);
	system("gpio readall");
	delay(1000);
	return 0 ;
}

