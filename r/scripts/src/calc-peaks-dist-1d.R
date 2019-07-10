#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Functions
########################################################

my.filter.func <- function(df) {
  my.indices <- eval(parse(text = my.filter.expr))
  return(my.indices)
}

########################################################
# Command line parsing
########################################################

my.option.list <- list( 
  make_option(
    c("-c", "--column"),
    type = "character",
    default = NA, 
    help = "Data column name"
  ),
  make_option(
    c("-f", "--filter"),
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
    default = 1000, 
    help = "Distribution max value"
  ),
  make_option(
    c("", "--step"),
    type = "double",
    default = 1, 
    help = "Distribution step"
  ),
  make_option(
    c("-a", "--absolute"),
    type = "logical",
    default = FALSE, 
    action = "store_true",
    help = "Use absolute values"
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

########################################################
# Processing
########################################################

if (is.null(my.opt$options$filter)) {
  my.filter <- NULL
} else {
  my.filter.expr <- paste(
    "which(",
    gsub(pattern = "@", replacement = "df$", x = my.opt$options$filter),
    ")"
  )
  my.filter <- my.filter.func
}

gneis.daq::peaks.dist.1d(
  srcDir = my.opt$args[1],
  destFn = my.opt$args[2],
  column = my.opt$options$column,
  breaks = seq(from = my.opt$options$min, to = my.opt$options$max, by = my.opt$options$step),
  absolute = my.opt$options$absolute,
  filter = my.filter,
  nevents = my.opt$options$number
)
