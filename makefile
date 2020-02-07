buildrooms: hirschet.buildrooms.c
	gcc -g -o hirschet.buildrooms hirschet.buildrooms.c

adventure: hirschet.adventure.c
	gcc -g -o hirschet.adventure hirschet.adventure.c

clean:
	rm hirschet.buildrooms hirschet.adventure
