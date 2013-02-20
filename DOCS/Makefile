.SUFFIXES = .tex .dvi .pdf

PDFLATEX=pdflatex
MAKEINDEX=makeindex
BIBTEX=bibtex
RM=rm -f

all : ndes.pdf


%.pdf : %.tex
	${PDFLATEX}  $<
	test ! -f $*.idx || ${MAKEINDEX} $* 
	${PDFLATEX}  $<
	${PDFLATEX}  $<

clean :
	${RM} ndes.pdf  *.idx *.log *.aux *.bbl *.blg *.brf *.cb *.ind *.ilg *.inx  *.toc *.out
