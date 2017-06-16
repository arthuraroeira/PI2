all:
	gcc comunicacao.c -lwiringPi
	sudo ./a.out 1 1 1 1
kill:
	sudo pkill a.out
git:
	git add .
	git commit -m "oi"
	git push origin arduino_to_rpi

