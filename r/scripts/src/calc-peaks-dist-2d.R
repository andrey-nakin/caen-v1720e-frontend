#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Functions
########################################################

my.filter.func <- function(df) {
  my.indices <- eval(my.filter.expr)
  return(my.indices)
}

########################################################
# Command line parsing
########################################################

my.option.list <- list( 
  make_option(
    c("-x", "--xcolumn"),
    type = "character",
    default = NA, 
    help = "Data column name for X axis"
  ),
  make_option(
    c("-y", "--ycolumn"),
    type = "character",
    default = NA, 
    help = "Data column name for Y axis"
  ),
  make_option(
    c("-f", "--filter"),
    type = "character",
    default = NULL, 
    help = "Filtering expression"
  ),
  make_option(
    c("", "--xmin"),
    type = "double",
    default = 0, 
    help = "Distribution min value for X axis"
  ),
  make_option(
    c("", "--xmax"),
    type = "double",
    default = 1000, 
    help = "Distribution max value for X axis"
  ),
  make_option(
    c("", "--xstep"),
    type = "double",
    default = 1, 
    help = "Distribution step for X axis"
  ),
  make_option(
    c("", "--ymin"),
    type = "double",
    default = 0, 
    help = "Distribution min value for Y axis"
  ),
  make_option(
    c("", "--ymax"),
    type = "double",
    default = 1000, 
    help = "Distribution max value for Y axis"
  ),
  make_option(
    c("", "--ystep"),
    type = "double",
    default = 1, 
    help = "Distribution step for Y axis"
  ),
  make_option(
    c("", "--xabsolute"),
    type = "logical",
    default = FALSE, 
    action = "store_true",
    help = "Use absolute values for values along X axis"
  ),
  make_option(
    c("", "--yabsolute"),
    type = "logical",
    default = FALSE, 
    action = "store_true",
    help = "Use absolute values for values along Y axis"
  ),
  make_option(
    c("-v", "--verbose"),
    type = "logical",
    default = FALSE, 
    action = "store_true",
    help = "Print diagnostic messages"
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

if (is.na(my.opt$options$xcolumn)) {
  stop("Column for X axis is not specified")
}
if (is.na(my.opt$options$ycolumn)) {
  stop("Column for Y axis is not specified")
}

########################################################
# Processing
########################################################

if (is.null(my.opt$options$filter)) {
  my.filter <- NULL
} else {
  if (my.opt$options$verbose) {
    cat("Filter expression: ", my.opt$options$filter, "\n")
  }
  my.filter.expr <- parse(
    text = paste(
      "which(",
      gsub(pattern = "@", replacement = "df$", x = my.opt$options$filter),
      ")"
    )
  )
  my.filter <- my.filter.func
}

gneis.daq::peaks.dist.2d(
  srcDir = my.opt$args[1],
  destFn = my.opt$args[2],
  xColumn = my.opt$options$xcolumn,
  yColumn = my.opt$options$ycolumn,
  xBreaks = seq(
    from = my.opt$options$xmin, 
    to = my.opt$options$xmax, 
    by = my.opt$options$xstep
  ),
  yBreaks = seq(
    from = my.opt$options$ymin, 
    to = my.opt$options$ymax, 
    by = my.opt$options$ystep
  ),
  xabsolute = my.opt$options$xabsolute,
  yabsolute = my.opt$options$yabsolute,
  filter = my.filter,
  nevents = my.opt$options$number,
  verbose = my.opt$options$verbose
)
