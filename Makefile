default:		all
INSTALLDIR=$(realpath .)/install

all:			$(INSTALLDIR)/z3libsbuilt.ph

$(INSTALLDIR)/z3libsbuilt.ph:
ifeq ($(shell uname), Linux)
	CXXFLAGS="-O3 -flto" LDFLAGS="-O3 -flto" ./configure
	$(MAKE) -j 8 -C build
	./install.sh; ./clean.sh; touch $@
else
	CXXFLAGS="-O3" ./configure
	$(MAKE) -C build
	./install.sh; ./clean.sh; touch $@
endif

.PHONY:			clean

clean:
	./clean.sh; rm -rf $(INSTALLDIR)
	rm -rf scripts/*.pyc
