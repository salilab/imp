WEB=${WEBTOP}/1.0/tutorial/images
TOPDIR=../../..

include ${TOPDIR}/Makefile.include

all: ${WEB}

${WEB}: *.png
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	cp *.png ${WEB}
