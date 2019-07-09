#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Functions
########################################################

my.filter <- function(df) {
  my.indices <- NULL

  return(my.indices)
}

my.file.comment <- function() {
  return(
    paste(
      paste("data column =", my.opt$options$column), 
      paste("distribution range >=", my.opt$options$min, "and <", my.opt$options$max), 
      paste("distribution step =", my.opt$options$step), 
      sep = ", "
    )
  )
}

########################################################
# Command line parsing
########################################################

my.option.list <- list( 
  make_option(
    c("", "--column"),
    type = "character",
    default = NA, 
    help = "Data column name"
  ),
  make_option(
    c("f", "--filter"),
    type = "character",
    default = NA, 
    help = "Filtering expression"
  ),
  make_option(
    c("", "--min"),
    type = "double",
    default = 0, 
    help = "Distribution min value"
  ),
  make_option(
    c("", "--max"),
    type = "double",
    default = NA, 
    help = "Distribution max value"
  ),
  make_option(
    c("", "--step"),
    type = "double",
    default = 1, 
    help = "Distribution step"
  ),
  make_option(
    c("", "--suffix"),
    type = "character",
    default = NULL, 
    help = "Destination file name suffix"
  ),
  make_option(
    c("-n", "--number"),
    type = "integer",
    default = NA, 
    help = "Max number of events"
  )
)

my.opt <- parse_args(
  OptionParser(
    option_list = my.option.list,
    usage = "%prog [options] <src file dir> <txt file name>"
  ), 
  positional_arguments = c(2, 2)
)

if (is.na(my.opt$options$column)) {
  stop("Column is not specified")
}
if (is.na(my.opt$options$max)) {
  stop("Min distribution value is not specified")
}
if (is.na(my.opt$options$min)) {
  stop("Max distribution value is not specified")
}

########################################################
# Processing
########################################################

gneis.daq::peaks.dist.1d(
  srcDir = my.opt$args[1],
  destFn = my.opt$args[2],
  column = my.opt$options$column,
  breaks = seq(from = my.opt$options$min, to = my.opt$options$max, by = my.opt$options$step),
  filter = my.filter,
  nevents = my.opt$options$number,
  file.comment = my.file.comment()
)
