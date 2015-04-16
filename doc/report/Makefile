PWD = $(shell pwd)
SRC = slrk_internals.tex
OUT = $(PWD)/out
VIEWER = xdg-open

PDF = $(OUT)/$(SRC:.tex=.pdf)
AUX = $(OUT)/$(SRC:.tex=.aux)
BCF = $(OUT)/$(SRC:.tex=.bcf)
SCHEME = $(wildcard $(PWD)/res/*.svg)
SCHEME_OUT = $(SCHEME:.svg=.pdf)

pdf: $(SRC) $(SCHEME)
	mkdir -p $(OUT)
	xelatex -shell-escape --output-directory=$(OUT) $(SRC)

bib:
	BIBINPUTS=$(PWD) BSTINPUT=$(PWD) TEXMFOUTPUT=$(OUT) biber $(BCF)

p: pdf
	$(VIEWER) $(PDF)

%.pdf: %.svg
	inkscape -D -z --file=$< --export-pdf=$@

scheme: $(SCHEME_OUT)

clean:
	$(RM) -r $(OUT)
	$(RM) -r $(SCHEME_OUT)
