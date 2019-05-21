FILENAME = hw4
SRC = $(FILENAME).tex
OUTPUT = $(FILENAME).pdf
AUX = $(FILENAME).aux $(FILENAME).log

.PHONY: hw4 clean

hw4: 
	xelatex $(SRC) \
	code $(OUTPUT)

clean:
	rm $(OUTPUT) $(AUX)