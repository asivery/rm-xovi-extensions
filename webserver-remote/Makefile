objects = main.o xovi.o librust_proxy.a

CXXFLAGS += -D_GNU_SOURCE -fPIC `pkg-config --cflags-only-I Qt6Core Qt6Network`
name = webserver-remote

ifeq ($(OECORE_TARGET_ARCH), aarch64)
CARGO_BUILD_TARGET=aarch64-unknown-linux-gnu
else ifeq ($(OECORE_TARGET_ARCH), arm)
CARGO_BUILD_TARGET=armv7-unknown-linux-gnueabihf
else
$(error Bad build target. Source the toolchain environment)
endif

webserver-remote : $(objects)
	${CC} ${CFLAGS}  -shared -o $(name).so $(objects)


librust_proxy.a:
	cd rust-proxy && cargo build --release --lib --target $(CARGO_BUILD_TARGET)
	cp rust-proxy/target/${CARGO_BUILD_TARGET}/release/librust_proxy.a .



xovi.cpp xovi.h	&: $(name).xovi notification.qmd
	python3 ${XOVI_REPO}/util/xovigen.py -o xovi.cpp -H xovi.h $(name).xovi

main.o	: xovi.h main.cpp

.PHONY  : clean
clean :
	rm -f $(name).so $(objects) xovi.cpp xovi.h
