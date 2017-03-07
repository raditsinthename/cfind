PROJECT = cfind
HEADERS = /cslinux/adhoc/include/statexpr.h cfind.h sort.h
OBJ = cfind.o dfs.o listInfo.o sort.o

C99 = cc -std=c99
CFLAGS = -Wall -pedantic -Werror

$(PROJECT) : $(OBJ) $(HEADERS)
	$(C99) $(CFLAGS) -I. /cslinux/adhoc/lib/libstatexpr-osx.a -o $(PROJECT) $(OBJ) -lm

cfind.o : cfind.c $(HEADERS)
	$(C99) $(CFLAGS) -c cfind.c

dfs.o : dfs.c cfind.h
	$(C99) $(CFLAGS) -c dfs.c

listInfo.o : listInfo.c cfind.h
	$(C99) $(CFLAGS) -c listInfo.c

sort.o: sort.c cfind.h sort.h
	$(C99) $(CFLAGS) -c sort.c

clean:
	rm -f $(PROJECT) $(OBJ)