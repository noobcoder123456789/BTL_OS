all: sum_multi-thread.o sum_serial.o
	gcc sum_serial.c -o sum_serial  && gcc sum_multi-thread.c -o sum_multi-thread
	./sum_serial 1000000
	./sum_multi-thread 10 1000000
sum_serial: sum_serial.o	
	gcc sum_serial.c -o sum_serial
	./sum_serial 1000000
sum_multi-thread: sum_multi-thread.o
	gcc sum_multi-thread.c -o sum_multi-thread
	./sum_multi-thread 10 1000000	
clean:
	rm -f sum_serial sum_multi-thread
	rm -f sum_serial.o sum_multi-thread.o
