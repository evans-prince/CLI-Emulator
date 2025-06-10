# Makefile for C emulator and LaTeX guide

# File names
EXEC = emulator
SRC = main.c
GUIDE_TEX = emulator-guide.tex
GUIDE_PDF = emulator-guide.pdf

# Default target: compile emulator
all: $(EXEC)

$(EXEC): $(SRC)
	@echo "Compiling C program..."
	@gcc -o $(EXEC) $(SRC)
	@echo "Executable '$(EXEC)' created "

# Build LaTeX guide PDF
guide: $(GUIDE_TEX)
	@echo "Building LaTeX guide..."
	@pdflatex $(GUIDE_TEX) > /dev/null
	@pdflatex $(GUIDE_TEX) > /dev/null
	@echo "Guide '$(GUIDE_PDF)' generated "

# Clean everything
clean:
	@echo "Cleaning up... "
	@rm -f $(EXEC) $(GUIDE_PDF) *.aux *.log *.out *.toc *.lof *.lot
	@echo "All build files removed "

.PHONY: all guide clean
