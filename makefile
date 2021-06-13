all: decoder.c
	g++ -o decoder decoder.c

clean:
	rm decoder
