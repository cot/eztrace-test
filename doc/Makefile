
all: user_manual

user_manual:
	pdflatex -interaction=nonstopmode user_manual.tex

bib: 
	pdflatex -interaction=nonstopmode user_manual.tex
	bibtex user_manual.aux
	pdflatex -interaction=nonstopmode user_manual.tex
	pdflatex -interaction=nonstopmode user_manual.tex

.PHONY: clean
clean:
	rm -f ._* *.log *.aux *.blg *.bbl *.toc *.snm *.nav *.out *.backup *~
