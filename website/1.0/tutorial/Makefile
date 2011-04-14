WEB=${WEBTOP}/1.0/tutorial
TOPDIR=../..
FILES=${WEB}/index.html ${WEB}/intro.html ${WEB}/library.html \
      ${WEB}/restrainer.html ${WEB}/saxs.html ${WEB}/multifit.html
SUBDIRS=images

include ${TOPDIR}/Makefile.include

zips:
	for ex in multifit restrainer saxs; do \
	  ./get-filelist.pl $${ex} > filelist; \
          zip ${WEB}/$${ex}.zip `cat filelist`; \
          rm -f filelist; \
        done
