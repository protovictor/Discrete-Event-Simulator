.SUFFIXES = .tex .dvi .pdf

DOXYGEN=doxygen
DOXYFILE=Doxyfile
PDFLATEX=pdflatex
MAKEINDEX=makeindex
BIBTEX=bibtex
RM=rm -f

all : ndes.pdf ndes-en.pdf doxy

doxy :
	${DOXYGEN} ${DOXYFILE}

%.pdf : %.tex
	${PDFLATEX}  $<
	test ! -f $*.idx || ${MAKEINDEX} $* 
	${PDFLATEX}  $<
	${PDFLATEX}  $<

clean :
	${RM} ndes.pdf ndes-en.pdf *.idx *.log *.aux *.bbl *.blg *.brf *.cb *.ind *.ilg *.inx  *.toc *.out
