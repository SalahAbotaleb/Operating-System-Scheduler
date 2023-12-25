DS:=./Data_Structures
build:
	gcc -g process_generator.c -o process_generator.out
	gcc -g clk.c -o clk.out
	gcc -g process.c -o process.out
	gcc -g test_generator.c -o test_generator.out
	gcc -g -c Data_Structures/pqueue.c -o Data_Structures/pqueue.out
	gcc -g -c Data_Structures/linked_list.c -o Data_Structures/linked_list.out
	gcc -g -c Memory/memory.c -o Memory/memory.out
	gcc -g -c scheduler.c -o schedulerStaged.out
	gcc -g -o scheduler.out schedulerStaged.out Data_Structures/pqueue.out Data_Structures/linked_list.out Memory/memory.out -lm

clean:
	rm -f *.out

all: clean build

run:
	./process_generator.out

priority_queue:
	gcc -c $(DS)/pqueue.c -o $(DS)/pqueue.out

priority_queue_sample:
	gcc $(DS)/sample.c -o $(DS)/sample.out $(DS)/pqueue.out

linked_list:
	gcc -c $(DS)/linked_list.c -o $(DS)/linked_list.out
