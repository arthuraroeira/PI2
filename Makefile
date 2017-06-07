all:
	gcc comunicacao.c -lwiringPi
	sudo ./a.out 1 1 1 1
kill:
	sudo pkill a.out
