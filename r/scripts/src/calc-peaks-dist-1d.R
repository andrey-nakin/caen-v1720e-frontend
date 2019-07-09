#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Functions
########################################################

my.filter <- function(df) {
  if (!my.opt$options$nocamtimefilter && !my.opt$options$noampfilter) {
    my.indices <- which(
      abs(df[[my.amp.col]]) >= my.amp.from &
        df[[my.cam.col]] >= my.cam.time.from &
        df[[my.cam.col]] < my.cam.time.to
    )
  } else if (!my.opt$options$nocamtimefilter) {
    my.indices <- which(
      df[[my.cam.col]] >= my.cam.time.from &
        df[[my.cam.col]] < my.cam.time.to
    )
  } else if (!my.opt$options$noampfilter) {
    my.indices <- which(
      abs(df[[my.amp.col]]) >= my.amp.from
    )
  } else {
    my.indices <- NULL
  }
      
  return(my.indices)
}

my.file.comment <- function() {
  my.time.range <- paste("distribution range >=", my.opt$options$min, "and <", my.opt$options$max);
  my.time.step  <- paste("distribution step =", my.opt$options$step)
  
  return(paste(my.time.range, my.time.step, sep = ", "))
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
    usage = "%prog [options] <src file dir> <txt file dir>"
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

my.channels <- read.table("channels.txt", header = T, sep = "")
my.cam.channel <- which.max(my.channels$CAM)

my.amp.col <- paste("CH", my.opt$options$channel, "_PA", sep = "")
my.amp.from <- my.channels$AMPFROM[my.opt$options$channel + 1]

my.cam.col <- paste("CH", my.opt$options$channel, "_PP_M", my.cam.channel - 1, sep = "")
my.cam.time.from <- my.channels$CTFROM[my.opt$options$channel + 1]
my.cam.time.to <- my.channels$CTTO[my.opt$options$channel + 1]

gneis.daq::peaks.dist.1d(
  srcDir = my.opt$args[1],
  txtDir = my.opt$args[2],
  column = my.opt$options$column,
  breaks = seq(from = my.opt$options$min, to = my.opt$options$max, by = my.opt$options$step),
  filter = my.filter,
  nevents = my.opt$options$number,
  file.suffix = my.opt$options$suffix,
  file.comment = my.file.comment()
)
