objects = main.o hash.o indexfile.o systemversion.o rccload.o xovi.o libqmldiff.a

CXX += -I ${SDKTARGETSYSROOT}/usr/include/QtCore/ -fPIC
CC += -D_GNU_SOURCE -fPIC
VPATH = src
name = qt-resource-rebuilder
ifeq ($(OECORE_TARGET_ARCH), aarch64)
CARGO_BUILD_TARGET=aarch64-unknown-linux-gnu
else ifeq ($(OECORE_TARGET_ARCH), arm)
CARGO_BUILD_TARGET=armv7-unknown-linux-gnueabihf
else
$(error Bad build target. Source the toolchain environment)
endif

fileman : $(objects)
	${CC} ${CFLAGS} -shared -o $(name).so $(objects)

libqmldiff.a	:
	cd qmldiff && cargo build --release --lib --target $(CARGO_BUILD_TARGET)
	cp qmldiff/target/${CARGO_BUILD_TARGET}/release/libqmldiff.a .

xovi.c xovi.h	&: $(name).xovi
	python ${XOVI_REPO}/util/xovigen.py -o xovi.c -H xovi.h $(name).xovi

main.o		: xovi.h types.h indexfile.h qmldiff.h ../util.h systemversion.h
hash.o		: hash.h
indexfile.o	: xovi.h rccload.h ../util.h


.PHONY  : clean
clean :
	rm -f $(name).so $(objects) xovi.c xovi.h
	cd qmldiff && cargo clean
