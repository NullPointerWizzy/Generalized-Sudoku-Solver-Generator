EXE = sudoku
EXE_COLORS_TESTS = colors_tests
EXE_GRID_TESTS = grid_tests
MAIN_FILE = report

all: build 

build:
	@cd src && $(MAKE)
	@cp -f src/$(EXE) ./

report: $(MAIN_FILE).pdf

$(MAIN_FILE).pdf: report/$(MAIN_FILE).tex
	@cd report && $(MAKE) $(MAIN_FILE).pdf
	@cp report/$(MAIN_FILE).pdf ./

test:
	@cd tests && $(MAKE)
	@cp -f tests/$(EXE_COLORS_TESTS) ./
	@cp -f tests/$(EXE_GRID_TESTS) ./

clean:
	@cd src && $(MAKE) clean
	@cd tests && $(MAKE) clean
	@cd report && $(MAKE) clean
	@rm -f $(EXE)
	@rm -f $(EXE_COLORS_TESTS)
	@rm -f $(EXE_GRID_TESTS)
	@rm -f $(MAIN_FILE).pdf

help:
	@echo "Usage:"
	@echo " make [all] Build"
	@echo " make build Build the software"
	@echo " make report Generate the PDF report"
	@echo " make test Build the test executables"
	@echo " make clean Remove all files generated by make"
	@echo " make help Display this help"

.PHONY: all build report test clean help
