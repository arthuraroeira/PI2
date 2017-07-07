all:
	clear
	g++ picole.cpp  Stepper.o -lwiringPi
	./a.out 1 1 1 1
obejto:
	g++ -c Stepper.cpp -lwiringPi
kill:
	sudo pkill a.out
replace:
	rm picole.cpp
	nano picole.cpp
