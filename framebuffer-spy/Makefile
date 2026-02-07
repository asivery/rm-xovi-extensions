objects = main.o xovi.o

CC += -D_GNU_SOURCE -fPIC
VPATH = src:../framebuffer-spy/src
name = framebuffer-spy
ifeq ($(OECORE_TARGET_ARCH), aarch64)
xovifile=framebuffer-spy-rmpp
else ifeq ($(OECORE_TARGET_ARCH), arm)
xovifile=framebuffer-spy-rm2
else
$(error Bad build target. Source the toolchain environment)
endif

framebuffer-spy : $(objects)
	${CC} ${CFLAGS} -shared -o $(name).so $(objects)


xovi.c xovi.h	&: $(xovifile).xovi
	python3 ${XOVI_REPO}/util/xovigen.py -o xovi.c -H xovi.h $(xovifile).xovi

main.o	: xovi.h src/main.c ../util.h framebuffer-spy.h

.PHONY  : clean
clean :
	rm -f $(name).so $(objects) xovi.c xovi.h
