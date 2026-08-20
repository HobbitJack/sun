.POSIX:	
.PHONY:	all clean install release source uninstall
.SUFFIXES:

PREFIX 	?= /usr/local
DATADIR := $(PREFIX)/share/sun

CFLAGS += -DDATADIR=$(DATADIR)

all:	sun cities.dat

sun:	sun.c sun.g.h sun.g.h
	c99 -pedantic -lm -o $@ sun.c sun.g.c sun.g.h

sun.g.c sun.g.h:	sun.g
	gengetopt <sun.g
	sed -E 's/(\\n)?[[:blank:]]+\(default=.*\)//' <sun.g.c >sun.g.c.tmp
	mv -f sun.g.c.tmp sun.g.c

cities.dat:	worldcities.csv
	sed 's/","/\t/g' <worldcities.csv | tail -n+1 | tr -d '"' | cut -f1,2,3,4,10 | sort -hk5 -rt'	' | cut -f2,3,4 > cities.dat

clean:
	rm -f sun sun.g.? sun*.tar.gz sun.1.gz Makefile cities.dat

source:
	rm -f sun_source.tar.gz
	tar -cf sun_source.tar sun.c sun.g sun.1 worldcities.csv
	gzip sun_source.tar

release:	sun
	rm -f sun.tar.gz
	sed 7,36d makefile | sed '2c .PHONY:	install uninstall'> Makefile
	tar -cf sun.tar sun sun.1 cities.dat Makefile
	gzip sun.tar

install:	sun 
	mkdir -p $(PREFIX)/bin/
	install sun $(PREFIX)/bin/
	gzip -k sun.1
	mkdir -p $(PREFIX)/share/man/man1/
	install sun.1.gz $(PREFIX)/share/man/man1/
	mkdir -p $(DATADIR)
	install cities.dat $(DATADIR)/

uninstall:
	rm $(PREFIX)/bin/sun
	rm $(PREFIX)/share/man/man1/sun.1.gz
	rm $(DATADIR)/cities.dat
	rmdir $(DATADIR)
