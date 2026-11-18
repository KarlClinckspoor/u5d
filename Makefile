# The matching gate: build the 25 game files from source with Microsoft C 5.1/5.0 and MASM 4.0
# in DOSBox-X, link them with tools/u5link.py and compare their SHA-1 with the GOG release and
# with the 1988 release (docs/MATCHING.md; RELEASE=gog or RELEASE=1988 checks one). PYTHON is
# the interpreter of the tools' virtual environment.
PYTHON ?= python
RELEASE ?= both
check:
	$(PYTHON) tools/check.py --target $(RELEASE)

.PHONY: check
