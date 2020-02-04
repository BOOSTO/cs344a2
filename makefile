buildrooms: hirschet.buildrooms.c
	gcc -o hirschet.buildrooms hirschet.buildrooms.c

adventure: hirschet.adventure.c
	gcc -o hirschet.adventure hirschet.adventure.c

clean:
	rm hirschet.buildrooms hirschet.adventure
