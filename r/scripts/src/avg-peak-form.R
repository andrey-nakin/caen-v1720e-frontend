#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Command line parsing
########################################################

my.option.list <- list( 
  make_option(
    c("-a", "--amplitude"),
    type = "double",
    default = 1000, 
    help = "Min value of peak amplitude (default %default)"
  ),
  make_option(
    c("-f", "--front"),
    type = "integer",
    default = 4, 
    help = "Number of samples before the peak (default %default)"
  ),
  make_option(
    c("-l", "--lasting"),
    type = "integer",
    default = 4, 
    help = "Number of samples after the peak (default %default)"
  ),
  make_option(
    c("-m", "--max"),
    type = "integer",
    default = -1, 
    help = "Max number of waveforms to process (default %default)"
  ),
  make_option(
    c("-t", "--trigger"),
    type = "integer",
    default = -1, 
    help = "Trigger channel # (default %default)"
  )
)

my.opt <- parse_args(
  OptionParser(
    option_list = my.option.list,
    usage = "%prog [options] <plot file name> <MID file 1> [<MID file 2> ...]"
  ), 
  positional_arguments = c(2, Inf)
)

my.plot.file <- my.opt$args[1]
my.midas.files <- tail(my.opt$args, n = -1)

########################################################
# Processing
########################################################

my.event.filter <- NULL
if (!(my.opt$options$trigger < 0)) {
  my.trigger.column <- paste("CH", my.opt$options$trigger, sep = "")
  my.event.filter <- function(e) {
    return(e$triggers[[my.trigger.column]][1] > 0)
  }
}

my.event.collector <- function(a, e) {
  cat(e$eventInfo$Run, e$eventInfo$EventCounter, "\n")
}

my.res <- read.events.from.gdconvert(
  file.names = my.midas.files,
  filter.func = my.event.filter,
  merging.func = my.event.collector,
  init.value = 0,
  nevents = 10
)
