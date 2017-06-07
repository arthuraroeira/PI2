#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <signal.h>
#define PROX 5
#define TEMPO_DESLIGAR_TELA 20

pid_t pid_filho;

//void desligar()
//{
//	system("sudo /opt/vc/bin/tvservice -p && /opt/vc/bin/tvservice -o");
//}

int main (int argc, int **argv)
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

	system("amixer cset numid=3 1");//setar saida para o jack
	system("amixer  sset PCM,0 80%");//volume
//	signal(SIGALRM, desligar);

	for(i = 0 ; i < 4 ; i++)
		motor[i] = atoi(argv[i+1]);

        pid_filho = fork();
        if (pid_filho == 0)
        {
                while(1)
                {
                        if(musica)
                                system("aplay compra.wav");
                        else
                        {
                                system("aplay karine.wav");
                                musica = 1;
                        }

                }
        }
	else
	while(1)
	{

		if(girar_motor)
		{
			for(i = 0 ; i < 4 ; i++)
				while(motor[i]>0)
				{
					serialPutchar (fd ,i+1);
					while(!serialDataAvail (fd)){}
					x = serialGetchar(fd);
					if(i+1 == x)
						printf("Sucesso, motor %d acionado\n", x);
					else
						printf("Erro no acionamento do motor\n");
					motor[i]--;
				}
			girar_motor = 0;
		}

		while(!serialDataAvail (fd)){}
		x = serialGetchar(fd);

		while(!serialDataAvail (fd)){}
		y = serialGetchar(fd);
		temperatura = (float) (256*x+y)/100 - 100;
		printf("Temperatura: %.2f\n", temperatura);

		for(i = 0 ; i < 4 ; i++)
		{
			while(!serialDataAvail (fd)){}
			x = serialGetchar(fd);
			while(!serialDataAvail (fd)){}
			y = serialGetchar(fd);
			posicoes[i] = 256*x+y;
			printf("Posicao %d: %d\n", i+1, posicoes[i]);
			if(posicoes[i] <= PROX)
			{
//				system("sudo /opt/vc/bin/tvservice -p && chvt 1 && chvt 7");
//				alarm(TEMPO_DESLIGAR_TELA);
				musica = 0;
				system("sudo pkill aplay");
			}
		}

		while(!serialDataAvail (fd)){}
		sinal_gps = serialGetchar(fd);
		if(sinal_gps == 1)
		{
			printf("Sinal GPS\n");
			lat = 0;
			for(i = 0 ; i < 4 ; i++)
			{
				while(!serialDataAvail (fd)){}
				lat |= ((long)serialGetchar(fd))<<(i*8);
			}

			lon = 0;
			for(i = 0 ; i < 4 ; i++)
			{
				while(!serialDataAvail (fd)){}
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

