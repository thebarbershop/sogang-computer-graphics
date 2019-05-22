FILENAME = hw4
PREAMBLE = $(FILENAME)-preamble
SRC = $(FILENAME).tex
OUTPUT = $(FILENAME).pdf
AUX = $(FILENAME).aux $(FILENAME).log $(PREAMBLE).aux $(PREAMBLE).fdb_latexmk $(PREAMBLE).fls $(PREAMBLE).log

.PHONY: hw4 clean

hw4: 
	xelatex $(SRC) \
	code $(OUTPUT)

clean:
	rm $(OUTPUT) $(AUX)