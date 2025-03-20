objects = main.o xovi.o

CC += -D_GNU_SOURCE -fPIC
VPATH = src:../framebuffer-spy/src
name = framebuffer-spy

framebuffer-spy : $(objects)
	${CC} ${CFLAGS} -shared -o $(name).so $(objects)


xovi.c xovi.h	&: $(name).xovi
	python3 ${XOVI_REPO}/util/xovigen.py -o xovi.c -H xovi.h $(name).xovi

main.o	: xovi.h src/main.c ../util.h

.PHONY  : clean
clean :
	rm -f $(name).so $(objects) xovi.c xovi.h
