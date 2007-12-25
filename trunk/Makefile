compile: cvshelpers

dirutils:
	gcc -Wall -c dirutils.c

cvshelpers: dirutils
	gcc -Wall cvshelpers.c dirutils.o -o cvshelpers

clean:
	rm -rf dirutils.o cvshelpers


