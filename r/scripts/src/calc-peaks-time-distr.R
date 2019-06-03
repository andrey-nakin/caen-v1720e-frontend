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
  my.time.range <- paste("time range >=", my.opt$options$timemin, "and <", my.opt$options$timemax);
  my.time.step  <- paste("time step =", my.opt$options$timestep)
  
  if (!my.opt$options$nocamtimefilter) {
    if (!is.na(my.cam.time.from) && !is.na(my.cam.time.to)) {
      my.cam.time <- paste("camera time >=", my.cam.time.from, "and <", my.cam.time.to)
    } else if (!is.na(my.cam.time.from)) {
      my.cam.time <- paste("camera time >=", my.cam.time.from)
    } else if (!is.na(my.cam.time.to)) {
      my.cam.time <- paste("camera time <", my.cam.time.to)
    }
  } else {
    my.cam.time <- "all camera times"
  }
  
  if (!my.opt$options$noampfilter) {
    if (!is.na(my.amp.from)) {
      my.amp <- paste("amplitude >=", my.amp.from)
    }
  } else {
    my.amp <- "all amplitudes"
  }

  return(paste(my.time.range, my.time.step, my.amp, my.cam.time, sep = ", "))
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
    c("", "--suffix"),
    type = "character",
    default = NULL, 
    help = "Destination file name suffix"
  ),
  make_option(
    c("", "--noampfilter"),
    type = "logical",
    default = FALSE, 
    action = "store_true",
    help = "Use filtration by amplitude"
  ),
  make_option(
    c("", "--nocamtimefilter"),
    type = "logical",
    default = FALSE, 
    action = "store_true",
    help = "Use filtration by camera time"
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
    usage = "%prog [options] <src file dir> <txt file dir> [<pdf file dir> [<html file dir>]]"
  ), 
  positional_arguments = c(2, 4)
)

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

gneis.daq::peaks.time.distr(
  srcDir = my.opt$args[1],
  txtDir = my.opt$args[2],
  pdfDir = my.opt$args[3],
  htmlDir = my.opt$args[4],
  channel = my.opt$options$channel,
  master = my.opt$options$master,
  trigger = my.opt$options$trigger,
  time.breaks = seq(from = my.opt$options$timemin, to = my.opt$options$timemax, by = my.opt$options$timestep),
  filter = my.filter,
  nevents = my.opt$options$number,
  file.suffix = my.opt$options$suffix,
  file.comment = my.file.comment()
)
