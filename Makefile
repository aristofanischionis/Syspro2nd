OBJS 	= main.o Input.o
SOURCE	= main.c src/Frontend/Input.c
HEADER  = HeaderFiles/Input.h
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


# clean house
clean:
	rm -f $(OBJS) $(OUT)

# do a bit of accounting
count:
	wc $(SOURCE) $(HEADER)