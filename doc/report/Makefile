PWD = $(shell pwd)
SRC = slrk_internals.tex
OUT = $(PWD)/out
VIEWER = xdg-open

PDF = $(OUT)/$(SRC:.tex=.pdf)
AUX = $(OUT)/$(SRC:.tex=.aux)
BCF = $(OUT)/$(SRC:.tex=.bcf)

pdf: $(SRC)
	mkdir -p $(OUT)
	xelatex -shell-escape --output-directory=$(OUT) $(SRC)

bib:
	BIBINPUTS=$(PWD) BSTINPUT=$(PWD) TEXMFOUTPUT=$(OUT) biber $(BCF)

p: pdf
	$(VIEWER) $(PDF)

clean:
	$(RM) -r $(OUT)
