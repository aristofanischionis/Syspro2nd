OBJS 	= main.o Input.o Inotify.o Actions.o
SOURCE	= main.c src/Frontend/Input.c src/Inotify/Inotify.c src/Actions/Actions.c
HEADER  = HeaderFiles/Input.h HeaderFiles/Inotify.h HeaderFiles/Actions.h
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


# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)