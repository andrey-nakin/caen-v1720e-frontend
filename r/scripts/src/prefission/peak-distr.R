#!/usr/bin/Rscript
# Arguments: [<options>] <peaks file>

library(gneis.daq)

my.cmd.args <- commandArgs(trailingOnly=TRUE)
my.file.peaks <- my.cmd.args[1]

