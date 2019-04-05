OBJS 	= main.o Input.o Inotify.o Actions.o FileOperations.o PipeOperations.o
SOURCE	= main.c src/Frontend/Input.c src/Inotify/Inotify.c src/Actions/Actions.c src/PipeOperations/PipeOperations.c src/FileOperations/FileOperations.c
HEADER  = HeaderFiles/Input.h HeaderFiles/Inotify.h HeaderFiles/Actions.h HeaderFiles/PipeOperations.h HeaderFiles/FileOperations.h 
OUT  	= mirror_client
CC		= gcc
FLAGS   = -g -c -Wall
# -g option enables debugging mode 
# -c flag generates object code for separate files

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT)

# create/compile the individual files >>separately<< 
main.o: main.c
	$(CC) $(FLAGS) main.c

Input.o: src/Frontend/Input.c
	$(CC) $(FLAGS) src/Frontend/Input.c

Inotify.o: src/Inotify/Inotify.c
	$(CC) $(FLAGS) src/Inotify/Inotify.c

Actions.o: src/Actions/Actions.c
	$(CC) $(FLAGS) src/Actions/Actions.c

FileOperations.o: src/FileOperations/FileOperations.c
	$(CC) $(FLAGS) src/FileOperations/FileOperations.c

PipeOperations.o: src/PipeOperations/PipeOperations.c
	$(CC) $(FLAGS) src/PipeOperations/PipeOperations.c


# clean house
clean:
	rm -f $(OBJS) $(OUT)
	rm -rf *_mirror && rm -rf common/ && rm log_*

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)