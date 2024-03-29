#!/bin/sh

#http://help.ubuntu.ru/wiki/using_gnu_autotools
#http://squadette.ru/autoconf-ru/autoconf-ru.html#SEC29

#out: configure.scan
#autoscan
#configure.scan -(edit)-> configure.in

#Руками создать: Makefile.am и src/Makefile.am

#Makefile.am -> Makefile.in -> Makefile

#in: configure.ac
#out: aclocal.m4
# aclocal

#out: config.h.in
# autoheader

#in: aclocal.m4
#out: Makefile.in
# automake --foreign --add-missing

#in: configure.ac
#out: ./configure
# autoconf

#out: Makefile
# ./configure

#in: Makefile
# make

aclocal && \
autoheader && \
automake --foreign --add-missing && \
autoconf
