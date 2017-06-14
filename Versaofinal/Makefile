all:
	clear
	g++ -c Stepper.cpp -lwiringPi
	g++ picole.cpp  Stepper.o -lwiringPi
	./a.out 1 1 1 1
kill:
	sudo pkill a.out
