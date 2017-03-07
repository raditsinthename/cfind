cfind : cfind.o dfs.o listInfo.o statexpr.o parsedate.o sort.o 
	cc -std=c99 -Wall -pedantic -Werror -o cfind \
		cfind.o listInfo.o dfs.o statexpr.o parsedate.o sort.o -lm

cfind.o : cfind.c cfind.h sort.h
	cc -std=c99 -Wall -pedantic -Werror -c cfind.c

dfs.o : dfs.c cfind.h
	cc -std=c99 -Wall -pedantic -Werror -c dfs.c

listInfo.o : listInfo.c cfind.h
	cc -std=c99 -Wall -pedantic -Werror -c listInfo.c

statexpr.o : statexpr.c statexpr.h
	cc -std=c99 -Wall -pedantic -Werror -c statexpr.c

parsedate.o : parsedate.c statexpr.h
	cc -std=c99 -Wall -pedantic -Werror -c parsedate.c

sort.o: sort.c cfind.h sort.h
	cc -std=c99 -Wall -pedantic -Werror -c sort.c