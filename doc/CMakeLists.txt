cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Doc VERSION 1.0.0 DESCRIPTION "Latex-based documentation")
include(UseLATEX.cmake)

add_latex_document(gneis-daq.tex
	INPUTS fe-v1720.tex
	BIBFILES gneis-daq.bib
	USE_BIBLATEX
	FORCE_PDF
	)
