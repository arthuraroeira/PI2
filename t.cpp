#include <stdlib.h>
#include <stdio.h>
#include <bits/stdc++.h>
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
#include <dirent.h>
#include <fcntl.h>

using namespace std;

int main()
{
	
	//Inicializar WiringPi
	if(wiringPiSetup () == -1)
		return 1 ;
	
	pinMode(9,INPUT);
	
	cout << "Start" << endl;
	while(1) {
		if (digitalRead(9) == 1) {
			cout << "1" << endl;
		}
		else {
			cout << "0"<< endl;
		}
		delay(1000);
	}
	
	return 0;
}
