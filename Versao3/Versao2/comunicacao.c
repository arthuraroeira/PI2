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

void Sair()
{
        printf("Saindo...\n");
	exit(1);
}

int main (int argc, char **argv)
{
        int fd, temp, sinal_gps, count, i, posicoes[4], motor[4], girar_motor, sucesso;
        long lat = 0, lon = 0;
        float temperatura;

        signal(SIGINT, Sair);

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
        system("amixer  sset PCM,0 65%");//volume
        serialFlush (fd);

        while(1)
        {
                //iniciar variaveis
                temp = 0, lat = 0, lon = 0;
		int x, y;

                //temperatura
                x = ((int)serialGetchar(fd));
                y = ((int)serialGetchar(fd));
		temperatura = (float) (256*x+y) / 100 - 100;

                //posicao
                for(i = 0 ; i < 4 ; i++)
                {
			x = ((int)serialGetchar(fd));
			y = ((int)serialGetchar(fd));
                        posicoes[i] = 256*x+y;
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
