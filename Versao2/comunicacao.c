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

#define PROX 5

int area;

typedef struct memoria_compartilhada {
	char musica;
} dado;

dado *a1;

void Alarme()
{
	a1->musica = 1;
	system("vcgencmd display_power 0");
	system("sudo pkill aplay");
}

void Sair()
{
        printf("Saindo...\n");
	shmctl(area, IPC_RMID, 0);
        system("sudo pkill a.out");
	exit(1);
}

int main (int argc, char **argv)
{
        int fd, temp, sinal_gps, count, i, posicoes[4], motor[4], girar_motor, sucesso;
        long lat = 0, lon = 0;
        float temperatura;

        signal(SIGINT, Sair);
	signal(SIGALRM, Alarme);

        //Dados do motor
        girar_motor = 1;
        for(i = 0 ; i < 4 ; i++)
                motor[i] = atoi(argv[i+1]);

        //Erros
        if ((fd = serialOpen ("/dev/ttyAMA0", 9600)) < 0)
                return 1 ;

        if (wiringPiSetup () == -1)
                return 1 ;

        system("amixer cset numid=3 1");//setar saida para o jack
        system("amixer  sset PCM,0 80%");//volume
	system("vcgencmd display_power 0");

        serialFlush (fd);
	area = shmget(IPC_PRIVATE, sizeof(dado), IPC_CREAT | 0644);
	pid_t pid = fork();
	a1 = (dado *) shmat(area, 0, 0);
	a1->musica = 1;

	if(pid == 0)
	{
		while(1)
		{
			if(a1->musica)
				system("aplay propaganda.wav");
                        else
                                system("aplay compra.wav");

		}
	}
        while(1)
        {
                //iniciar variaveis
                temp = 0, lat = 0, lon = 0;

                //temperatura
                temp |= ((int)serialGetchar(fd));
                temp |= ((int)serialGetchar(fd))<<8;
		temperatura = (float) temp / 100-102;

                //posicao
                for(i = 0 ; i < 4 ; i++)
                {
			posicoes[i] = 0;
                        posicoes[i] |= ((int)serialGetchar(fd));
                        posicoes[i] |= ((int)serialGetchar(fd)) << 8;
                	if(posicoes[i] <= PROX && posicoes[i] > 0 && a1->musica == 1)
			{
				a1->musica = 0;
				system("sudo pkill aplay");
				system("vcgencmd display_power 1");
				alarm(30);
			}
		}

                //sinal gps
                sinal_gps = serialGetchar(fd);

                //latitude e longitude
                for(i = 0 ; i < 4 ; i++)
                        lat |= ((long)serialGetchar(fd))<<(i*8);

                for(i = 0 ; i < 4 ; i++)
                        lon |= ((long)serialGetchar(fd))<<(i*8);

                if(girar_motor)
                {
                        for(i = 0 ; i < 4 ; i++)
                                while(motor[i]>0)
                                {
                                        serialPutchar (fd ,i+1);
                                        sucesso = serialGetchar(fd);
                                        if(i+1 == sucesso)
                                                printf("Sucesso, motor %d acionado\n", sucesso);
                                        else
                                                printf("Erro no acionamento do motor\n");
                                        motor[i]--;
                                }
                        girar_motor = 0;
                }

                printf("Temperatura: %.2f\n", temperatura);

                for(i = 0 ; i < 4 ; i++)
                        printf("Posicao %d: %d\n", i+1, posicoes[i]);

                if(sinal_gps)
                        printf("Sinal GPS\nLatitude: %.6lf\nLongitude: %.6lf\n", (double) lat/100000, (double) lon/100000);
                else
                        printf("Sem Sinal GPS!\n");

                printf("\n----------------\n");
        }


        return 0 ;
}
