main:
	find . -name '*[0-9]*' -delete
	pdflatex paper.tex
	asy -nosafe *.asy
	pdflatex paper.tex
