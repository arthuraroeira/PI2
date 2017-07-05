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
#include "Stepper.h"
#include <dirent.h>
#include <fcntl.h>

#define MOTOR1 0
#define MOTOR2 2
#define MOTOR3 3
#define MOTOR4 12
#define MOTOR_SELECAO1 8
#define MOTOR_SELECAO2 9

#define PINTRIG1 13
#define PINECHO1 14
#define PINTRIG2 30
#define PINECHO2 21
#define RELE_MOTOR 22

#define PROX 5


int area;

typedef struct memoria_compartilhada {
	char musica;
} dado;

dado *a1;

float temperatura(char *devPath)
{
	char buf[256], tmpData[6];
	int fd = open(devPath, O_RDONLY);
	ssize_t numRead;
	float tempC;

	if(fd == -1)
	{
		perror ("Couldn't open the w1 device.");
		return 1;   
	}
	if((numRead = read(fd, buf, 256)) > 0) 
	{
		strncpy(tmpData, strstr(buf, "t=") + 2, 5); 
		tempC = strtof(tmpData, NULL);
	}
	close(fd);
	return tempC / 1000;
}

void Alarme(int a)
{
	a1->musica = 1;
	system("vcgencmd display_power 0");
	system("sudo pkill aplay");
}

void Sair(int a)
{
	printf("Saindo...\n");
	shmctl(area, IPC_RMID, 0);
	system("sudo pkill a.out");
	exit(1);
}

int getCM(int trigger, int echo)
{
	//Send trig pulse
	digitalWrite(trigger, HIGH);
	delayMicroseconds(20);
	digitalWrite(trigger, LOW);

	//Wait for echo start
	while(digitalRead(echo) == LOW);

	//Wait for echo end
	long startTime = micros();
	while(digitalRead(echo) == HIGH);
	long travelTime = micros() - startTime;

	//Get distance in cm
	int distance = travelTime / 58;

	return distance;
}

int main (int argc, char **argv)
{
	//Abrir serial UART
	int fd;
	if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
		return 1 ;
	//serialGetchar(fd)
	//serialPutchar (fd , )

	if(wiringPiSetup () == -1)
		return 1 ;

	//Sinais
	signal(SIGINT, Sair);
	signal(SIGALRM, Alarme);

	//Dados do motor
	int girar_motor = 0, motor[4];
	for(int i = 0 ; i < 4 ; i++)
		motor[i] = atoi(argv[i+1]);
	Stepper myStepper(200, MOTOR1, MOTOR2, MOTOR3, MOTOR4);
	myStepper.setSpeed(60);
	pinMode(MOTOR_SELECAO1, OUTPUT); // Pinos 3 e 4 configurados como saida
	pinMode(MOTOR_SELECAO2, OUTPUT);
	pinMode(RELE_MOTOR, OUTPUT);

	//Posicao
	int trigger[3] = {PINTRIG1, PINTRIG2};
	int echo[3] = {PINECHO1, PINECHO2};
	for(int i = 0 ; i < 2 ; i++)
	{
   		pinMode(trigger[i], OUTPUT);
   		digitalWrite(trigger[i], LOW);
   		pinMode(echo[i], INPUT);
	}

	//Temperatura
	float tempC;
	DIR *dir;
	struct dirent *dirent;
	char dev[16], devPath[128], path[] = "/sys/bus/w1/devices";
	dir = opendir (path);

	if (dir != NULL)
	{
		while ((dirent = readdir (dir)))
			if (dirent->d_type == DT_LNK && strstr(dirent->d_name, "28-") != NULL)
			{
				strcpy(dev, dirent->d_name);
				printf("\nDevice: %s\n", dev);
			}
		(void) closedir (dir);
	}
	else
	{
		perror ("Couldn't open the w1 devices directory");
		return 1;
	}
	sprintf(devPath, "%s/%s/w1_slave", path, dev);

	//Setup inicial do sistema
	system("amixer cset numid=3 1");//setar saida para o jack
	system("amixer  sset PCM,0 65%");//volume
	system("vcgencmd display_power 0"); //desligar display

	serialFlush (fd);
	area = shmget(IPC_PRIVATE, sizeof(dado), IPC_CREAT | 0644);
	delay(30);
	pid_t pid = fork();
	a1 = (dado *) shmat(area, 0, 0);
	a1->musica = 1;


	if(pid == 0)
	{
		while(1)
		{
			if(a1->musica == 0)
				system("aplay compra.wav");
			else if(a1->musica == 1)
				system("aplay propaganda.wav");
			else if(a1->musica == 2)
				system("aplay alarme.wav");
			delay(10);
		}
	}
	else
	{
		while(1)
		{
			//Motor
			for(int i = 0 ; i < 4 ; i++)
				while(motor[i]>0)
				{
					digitalWrite(RELE_MOTOR, 1);//Ligar o RELE
					delay(100);//Esperar o RELE ligar
					//Escolher qual motor vai girar
					digitalWrite(MOTOR_SELECAO1, i%2);
					digitalWrite(MOTOR_SELECAO2, i/2);

					printf("Motor %d acionado\n", i);//Mostrar no terminal qual motor foi acionado
					myStepper.step(200); // Girar o motor 1 revolucao
					motor[i]--;
					delay(500);
				}

			digitalWrite(RELE_MOTOR, 0);//Desligar RELE

			//Posicao
			for(int i = 0 ; i < 2 ; i++)
			{
				int posicao = getCM(trigger[i], echo[i]);
				if(posicao <= PROX && posicao > 0 && a1->musica == 1)
				{
					a1->musica = 0;
					system("sudo pkill aplay");
					system("vcgencmd display_power 1");
					alarm(30);
				}
				printf("Distance: %dcm\n", posicao);
			}

			//Temperatura
			tempC = temperatura(devPath);
	        printf("Temperatura: %.2f\n", tempC);

			delay(1000);
	        printf("\n----------------\n");
		}
	}



        return 0 ;
}


