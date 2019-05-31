#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Functions
########################################################

my.filter <- function(df) {
  return(TRUE)
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
    c("", "--timemin"),
    type = "double",
    default = 0, 
    help = "Time min value"
  ),
  make_option(
    c("", "--timemax"),
    type = "double",
    default = NA, 
    help = "Time max value"
  ),
  make_option(
    c("", "--timestep"),
    type = "double",
    default = 1, 
    help = "Time distribution step"
  ),
  make_option(
    c("", "--ampmin"),
    type = "double",
    default = 0, 
    help = "Amplitude min value"
  ),
  make_option(
    c("", "--ampmax"),
    type = "double",
    default = NA, 
    help = "Amplitude max value"
  ),
  make_option(
    c("", "--ampstep"),
    type = "double",
    default = 1, 
    help = "Amplitude distribution step"
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

gneis.daq::peaks.amp.time.distr(
  srcDir = my.opt$args[1],
  destDir = my.opt$args[2],
  channel = my.opt$options$channel,
  master = my.opt$options$master,
  trigger = my.opt$options$trigger,
  amp.breaks = seq(from = my.opt$options$ampmin, to = my.opt$options$ampmax, by = my.opt$options$ampstep),
  time.breaks = seq(from = my.opt$options$timemin, to = my.opt$options$timemax, by = my.opt$options$timestep),
  filter = my.filter,
  nevents = my.opt$options$number
)
