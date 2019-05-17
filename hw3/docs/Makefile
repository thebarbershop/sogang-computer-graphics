FILENAME = HW3_S120085
SRC = $(FILENAME).md
OUTPUT = $(FILENAME).pdf

.PHONY: docs clean

docs: 
	pandoc $(SRC) --latex-engine=xelatex \
	-V mainfont="NanumGothic" \
	-V monofont="NanumMyeongjo" \
	-V papersize="a4" \
	-V documentclass="article" \
	-V margin=0.5in \
	-o $(OUTPUT); \
	code $(OUTPUT)

clean:
	rm $(OUTPUT)