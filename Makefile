makedirs = fileman qt-resource-rebuilder random-suspend-screen framebuffer-spy webserver-remote
qmakedirs = qt-command-executor xovi-message-broker
alldirs = $(qmakedirs) $(makedirs)

.PHONY	: all $(alldirs) clean clean-%

all	: $(makedirs) $(qmakedirs)
clean	: $(addprefix clean-,$(alldirs))

$(makedirs)	:
	cd $@ && $(MAKE)

$(qmakedirs)	:
	cd $@ && qmake . && $(MAKE)

clean-%	:
	cd $* && if [ -r Makefile ]; then $(MAKE) clean; fi
