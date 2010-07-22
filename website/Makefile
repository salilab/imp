WEB=~www/html/imp/

.PHONY: all

all: ${WEB}/groups.html ${WEB}/index.html ${WEB}/imp.css \
     ${WEB}/images ${WEB}/doc.html  ${WEB}/download.html ${WEB}/get.php \
     ${WEB}/download-windows.html ${WEB}/download-mac.html

${WEB}/groups.html::
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	(cat make-get.php index.php | php -- page=groups) > phpout
	@grep -q "</html>" phpout && cp phpout $@ && rm phpout || (echo "Error occurred during production of $@: check phpout for errors"; exit 1)

${WEB}/index.html::
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	(cat make-get.php index.php | php -- page=home) > phpout
	@grep -q "</html>" phpout && cp phpout $@ && rm phpout || (echo "Error occurred during production of $@: check phpout for errors"; exit 1)

${WEB}/pubs.html::
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	(cat make-get.php index.php | php -- page=pubs) > phpout
	@grep -q "</html>" phpout && cp phpout $@ && rm phpout || (echo "Error occurred during production of $@: check phpout for errors"; exit 1)

${WEB}/doc.html::
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	(cat make-get.php index.php | php -- page=doc) > phpout
	@grep -q "</html>" phpout && cp phpout $@ && rm phpout || (echo "Error occurred during production of $@: check phpout for errors"; exit 1)


${WEB}/download.html::
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	(cat make-get.php index.php | php -- page=download) > phpout
	@grep -q "</html>" phpout && cp phpout $@ && rm phpout || (echo "Error occurred during production of $@: check phpout for errors"; exit 1)

${WEB}/download-windows.html::
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	(cat make-get.php index.php | php -- page=download-windows) > phpout
	@grep -q "</html>" phpout && cp phpout $@ && rm phpout || (echo "Error occurred during production of $@: check phpout for errors"; exit 1)

${WEB}/download-mac.html::
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	(cat make-get.php index.php | php -- page=download-mac) > phpout
	@grep -q "</html>" phpout && cp phpout $@ && rm phpout || (echo "Error occurred during production of $@: check phpout for errors"; exit 1)

${WEB}/get.php: get.php get.php.in misc.inc.php header.txt footer.txt
	@if [ ! -d ${WEB} ]; then mkdir -p ${WEB}; fi
	(php get.php.in; cat get.php) > phpout
	@grep -q "</html>" phpout && cp phpout $@ && rm phpout || (echo "Error occurred during production of $@: check phpout for errors"; exit 1)

${WEB}/imp.css: imp.css
	cp $< $@

${WEB}/images: images/*
	@if [ ! -d ${WEB}/images ]; then mkdir -p ${WEB}/images; fi
	cp images/* ${WEB}/images
