#!/bin/bash

INSTALLPATH=`realpath -m ./_installdir/usr/`
rm -Rf ${INSTALLPATH}
mkdir -p ${INSTALLPATH}

./configure --prefix=${INSTALLPATH} && \
make                                && \
make install
