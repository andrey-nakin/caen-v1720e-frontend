#!/usr/bin/Rscript

library(optparse)

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

cat("my.plot.file=", my.plot.file, "\n")
cat("my.midas.files=", my.midas.files, "\n")
cat("amplitude=", my.opt$options$amplitude, "\n")
