#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <signal.h>

int main (int argc, char **argv)
{
	int musica = 1, sinal_gps, fd, count, i, posicoes[4], motor[4], girar_motor = 1;
	char x, y;
	long lat = 0, lon = 0;
	float temperatura;

	if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
	{
		fprintf (stderr, "Unable to open serial device: %s\n", strerror (errno)) ;
		return 1 ;
	}

	if (wiringPiSetup () == -1)
	{
		fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
		return 1 ;
	}

	for(i = 0 ; i < 4 ; i++)
		motor[i] = atoi(argv[i+1]);

	while(1)
	{


		if(girar_motor)
		{
			for(i = 0 ; i < 4 ; i++)
				while(motor[i]>0)
				{
					serialPutchar (fd ,i+1);
//					while(!serialDataAvail (fd)){delay(1);}
					x = serialGetchar(fd);
					if(i+1 == x)
						printf("Sucesso, motor %d acionado\n", x);
					else
						printf("Erro no acionamento do motor\n");
					motor[i]--;
				}
			girar_motor = 0;
		}

//		while(!serialDataAvail (fd)){delay(1);}
		x = serialGetchar(fd);

//		while(!serialDataAvail (fd)){delay(1);}
		y = serialGetchar(fd);
		temperatura = (float) (256*x+y)/100 - 100;
		printf("Temperatura: %.2f\n", temperatura);

		for(i = 0 ; i < 4 ; i++)
		{
//			while(!serialDataAvail (fd)){delay(1);}
			x = serialGetchar(fd);
//			while(!serialDataAvail (fd)){delay(1);}
			y = serialGetchar(fd);
			posicoes[i] = 256*x+y;
			printf("Posicao %d: %d\n", i+1, posicoes[i]);
		}

//		while(!serialDataAvail (fd)){delay(1);}
		sinal_gps = serialGetchar(fd);
		if(sinal_gps == 1)
		{
			printf("Sinal GPS\n");
			lat = 0;
			for(i = 0 ; i < 4 ; i++)
			{
//				while(!serialDataAvail (fd)){delay(1);}
				lat |= ((long)serialGetchar(fd))<<(i*8);
			}

			lon = 0;
			for(i = 0 ; i < 4 ; i++)
			{
//				while(!serialDataAvail (fd)){delay(1);}
				lon |= ((long)serialGetchar(fd))<<(i*8);
			}

			printf("Latitude: %.6lf\nLongitude: %.6lf\n", (double) lat/1000000, (double) lon/1000000);
		}
		else
		{
			printf("Sem Sinal GPS!\n");
		}
		printf("\n----------------\n");
	}


	return 0 ;
}

