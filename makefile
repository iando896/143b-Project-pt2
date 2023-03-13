CC = g++
CFLAGS = -g 
OBJ = main.o 

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
	
main.exe: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ 

run: main
	./main

clean:
	find . -name "*.o" -type f -delete