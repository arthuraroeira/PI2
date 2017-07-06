/*
	Universidade de Brasilia - FGA
	Trabalho de Projeto Integrador 2 - Maquina autonoma de vendas de picole

	Codigo para Raspberry Pi 3 elaborado por:
		>Arthur Luis Komatsu Aroeira
		>Furquim
	
	Referencia a codigos externos:
		>Sensor de Presenca - https://ninedof.wordpress.com/2013/07/16/rpi-hc-sr04-ultrasonic-sensor-mini-project/
		>Sensor de Temperatura - http://bradsrpi.blogspot.com.br/2013/12/c-program-to-read-temperature-from-1.html
*/

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <chrono>

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

//Pinos dos perifericos
#define PINTRIG1 3
#define PINECHO1 12
#define PINTRIG2 0
#define PINECHO2 2

#define RELE_COMPRESSOR 8

#define CONTATO 9

//Intervalos definidos
#define TEMPERATURA_ACIONAMENTO_LIGAR  2
#define TEMPERATURA_ACIONAMENTO_DESLIGAR -2
#define PROX 5

int area;

typedef struct memoria_compartilhada
{
	char musica;
	int Alarme_tocando;
} dado;

dado *a1;

void  Sinal_Alarme(int a);
void  Sinal_Sair(int a);
float Sensor_Temperatura(char *devPath);
int   Sensor_Presenca(int trigger, int echo);

int main(int argc, char **argv)
{
	//Abrir serial UART
	int fd;
	if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
		return 1 ;
	serialFlush (fd);
	//serialGetchar(fd)
	//serialPutchar (fd , )

	//Inicializar WiringPi
	if(wiringPiSetup () == -1)
		return 1 ;

	//Sinais
	signal(SIGINT, Sinal_Sair);
	signal(SIGALRM, Sinal_Alarme);

	//Posicao
	int trigger[2] = {PINTRIG1, PINTRIG2};
	int echo[2] = {PINECHO1, PINECHO2};
	for(int i = 0 ; i < 2 ; i++)
	{
		pinMode(trigger[i], OUTPUT);
		digitalWrite(trigger[i], LOW);
		pinMode(echo[i], INPUT);
	}

	//Contato
	pinMode(CONTATO, INPUT);

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
	pinMode(RELE_COMPRESSOR, OUTPUT);
	digitalWrite(RELE_COMPRESSOR, 1);

	//Setup inicial do sistema
	int volume, mudou_volume = 0;
	system("amixer cset numid=3 1");//setar saida para o jack
	system("vcgencmd display_power 0"); //desligar display
	system("amixer  sset PCM,0 65%");

	//Criar 1 processo novo
	area = shmget(IPC_PRIVATE, sizeof(dado), IPC_CREAT | 0644);
	delay(30);
	pid_t pid = fork();
	a1 = (dado *) shmat(area, 0, 0);
	a1->musica = 1;
	a1->Alarme_tocando = 0;

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
			//Posicao
			int posicao[2];
			for(int i = 0 ; i < 2 ; i++)
			{
				posicao[i] = Sensor_Presenca(trigger[i], echo[i]);
				printf("Distance: %dcm\n", posicao[i]);
			}
			if(posicao[0] <= PROX && posicao[0] > 0 && posicao[1] <= PROX && posicao[1] > 0 && a1->musica == 1 && a1->Alarme_tocando == 0)
			{
				a1->musica = 0;
				system("sudo pkill aplay");
				system("vcgencmd display_power 1");
				alarm(30);
			}

			//Temperatura
			tempC = Sensor_Temperatura(devPath);
			std::fstream fs;
			using namespace std::chrono;

			system_clock::time_point tp = system_clock::now();
			system_clock::duration dtn = tp.time_since_epoch();

			std::stringstream ss;
			ss.str (std::to_string(dtn.count() * system_clock::period::num / system_clock::period::den));
			
			fs.open (ss.str(), std::fstream::out);
			
			fs << tempC;
			
			fs.close();
			if(tempC < TEMPERATURA_ACIONAMENTO_DESLIGAR)
				digitalWrite(RELE_COMPRESSOR, 0);
			if(tempC > TEMPERATURA_ACIONAMENTO_LIGAR)
				digitalWrite(RELE_COMPRESSOR, 1);

			//Contato
			if(digitalRead(CONTATO) && a1->Alarme_tocando == 0)
			{
				a1->musica = 2;
				system("sudo pkill aplay");
				a1->Alarme_tocando = 1;
				alarm(1);
			}
				

			//Volume
			if(mudou_volume == 1)
			{
				char Volume[100] = "amixer  sset PCM,0 ", str[15];
				sprintf(str, "%d", volume);
				strcat(Volume, str);
				strcat(Volume, "%");
				system(Volume);//volume
				mudou_volume = 0;
			}

			delay(1000);
			printf("\n----------------\n");
		}
	}

	return 0 ;
}

void Sinal_Alarme(int a)
{
	if(a1-> Alarme_tocando == 1)
	{
		a1->musica = 2;
		alarm(1);
	}
	else
	{
		a1-> Alarme_tocando = 0;
		a1->musica = 1;
		system("vcgencmd display_power 0");
		system("sudo pkill aplay");
	}
}

void Sinal_Sair(int a)
{
	printf("Saindo...\n");
	shmctl(area, IPC_RMID, 0);
	system("sudo pkill a.out");
	exit(1);
}

float Sensor_Temperatura(char *devPath)
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

int Sensor_Presenca(int trigger, int echo)
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

