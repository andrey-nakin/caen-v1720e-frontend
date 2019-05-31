#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Functions
########################################################

my.filter <- function(df) {
  my.amp.from <- my.channels$AMPFROM[my.opt$options$channel + 1]
  if (!is.na(my.amp.from)) {
    my.col <- paste("CH", my.opt$options$channel, "_PA", sep = "")
    return(which(abs(df[[my.col]]) >= my.amp.from))
  }

  return()
}

########################################################
# Command line parsing
########################################################

my.option.list <- list( 
  make_option(
    c("-c", "--channel"),
    type = "integer",
    default = NA, 
    help = "Channel #"
  ),
  make_option(
    c("-m", "--master"),
    type = "integer",
    default = NA, 
    help = "Trigger channel # used for time calcs."
  ),
  make_option(
    c("-t", "--trigger"),
    type = "integer",
    default = NA, 
    help = "Trigger channel #"
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
    help = "Max # of samples"
  )
)

my.opt <- parse_args(
  OptionParser(
    option_list = my.option.list,
    usage = "%prog [options] <src file dir> <txt file dir>"
  ), 
  positional_arguments = c(2, 2)
)

########################################################
# Processing
########################################################

my.channels <- read.table("channels.txt", header = T, sep = "")

gneis.daq::peaks.time.quantiles(
  srcDir = my.opt$args[1],
  destDir = my.opt$args[2],
  channel = my.opt$options$channel,
  master = my.opt$options$master,
  trigger = my.opt$options$trigger,
  filter = my.filter,
  nevents = my.opt$options$number,
  file.suffix = my.opt$options$suffix
)
