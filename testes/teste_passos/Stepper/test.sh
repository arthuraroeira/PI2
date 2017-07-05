gpio readall

gpio mode 21 in
gpio mode 22 in
gpio mode 23 in
gpio mode 24 in

gpio mode 0 in
gpio mode 2 in

gpio write 0 1
gpio write 2 1

for i in {0..50}; do
	gpio write 21 1
	gpio write 22 0
	gpio write 23 1
	gpio write 24 0
	sleep 0.00001
	
	gpio write 21 0
	gpio write 22 1
	gpio write 23 1
	gpio write 24 0
	sleep 0.00001
	
	gpio write 21 0
	gpio write 22 1
	gpio write 23 0
	gpio write 24 1
	sleep 0.00001
	
	gpio write 21 1
	gpio write 22 0
	gpio write 23 0
	gpio write 24 1
	sleep 0.00001
done;

echo ""

gpio readall
