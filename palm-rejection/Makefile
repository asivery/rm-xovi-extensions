objects = Main.o TouchPipe.o HalManager.o xovi.o

CC += -D_GNU_SOURCE -fPIC
VPATH = src:../fileman/src
name = palm-rejection

fileman : $(objects)
	${CC} ${CFLAGS} -shared -o $(name).so $(objects)


xovi.c xovi.h	&: $(name).xovi
	python3 ${XOVI_REPO}/util/xovigen.py -o xovi.c -H xovi.h $(name).xovi

Main.o		: xovi.h fileman.h ../util.h
HalManager.o	: xovi.h files.h ../util.h fileman.h
TouchPipe.o	: xovi.h files.h ../util.h

.PHONY  : clean
clean :
	rm -f $(name).so $(objects) xovi.c xovi.h
