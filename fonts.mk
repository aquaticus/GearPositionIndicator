all: symbols8x8.c  
.PHONY: all

symbols8x8.c : symbols8x8.txt
	perl fontconvert.pl -t FONTTAB symbols8x8.txt symbols8x8.c

clean:
	rm symbols8x8.c
