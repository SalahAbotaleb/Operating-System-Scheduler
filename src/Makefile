DS:=./Data_Structures
build:
	gcc process_generator.c -o process_generator.out
	gcc clk.c -o clk.out
	gcc scheduler.c -o scheduler.out
	gcc process.c -o process.out
	gcc test_generator.c -o test_generator.out

clean:
	rm -f *.out  processes.txt

all: clean build

run:
	./process_generator.out

priority_queue:
	gcc -c $(DS)/pqueue.c -o $(DS)/pqueue.out

priority_queue_sample:
	gcc $(DS)/sample.c -o $(DS)/sample.out $(DS)/pqueue.out

linked_list:
	gcc -c $(DS)/linked_list.c -o $(DS)/linked_list.out
