#!/usr/bin/Rscript
# Arguments: <master trigger> <trigger> <destination file> <MID file1 [...]> 

library(optparse)
library(gneis.daq)

########################################################
# Global variables
########################################################

my.front.lengths <- rep(4, times = 8)
my.tail.lengths <- rep(10, times = 8)

my.last.master.run <- 0
my.last.master.eventCounter <- 0
my.last.master.timeStamp <- 0
my.last.master.trigger.position <- 0
my.last.master.peak.position <- 0

########################################################
# Functions
########################################################

my.print.init.info <- function(e) {
  cat(
    file = my.dest, 
    sep = "\t",
    "DATETIME",
    "RUN",
    "EC",
    "TS",
    "TRG",
    "PP",
    "EPP",
    "PA",
    "EPA",
    "FI",
    "TI",
    "\n"
  )
}

my.print.channel <- function(my.info, wf, trg.ch, trg.pos) {
  my.pulse <- get.pulse(
    wf,
    front.len = my.opt$options$front,
    tail.len = my.opt$options$tail,
    n.skip = my.opt$options$skip,
    need.integration = TRUE
  )
  
  if (is.null(my.pulse)) {
    return(0)
  }

  if (my.last.master.run == my.info$Run && !is.na(my.last.master.peak.position)) {
    my.pos <- (my.pulse$x - my.last.master.peak.position) + 
      (trg.pos - my.last.master.trigger.position) + 
      ((my.info$DeviceTimeStamp - my.last.master.timeStamp) %% my.info$DeviceTimeStampModule) * my.info$TicksPerSample
    my.pos.err <- my.pulse$x.err + 1
  } else {
    my.pos <- my.pulse$x - trg.pos
    my.pos.err <- my.pulse$x.err
  }
  
  if (my.pos > 9999) {
    my.pos <- round(my.pos)
  } else {
    my.pos <- format(my.pos, digits = my.opt$options$precision)
  }
  
  cat(
    file = my.dest, 
    sep = "\t",
    my.info$TimeStamp,
    my.info$Run,
    my.info$EventCounter,
    my.info$DeviceTimeStamp,
    trg.ch,
    my.pos,
    format(my.pos.err, digits = my.opt$options$precision),
    format(my.pulse$amp, digits = my.opt$options$precision),
    format(my.pulse$amp.err, digits = my.opt$options$precision),
    format(my.pulse$front.int, digits = my.opt$options$precision),
    format(my.pulse$tail.int, digits = my.opt$options$precision),
    "\n"
  )
  
  return(1)
}

my.event.collector <- function(a, e) {
  if (a == 0) {
    my.print.init.info(e)
  } else if (a %% 1000 == 0 && my.opt$options$verbose) {
    cat(e$eventInfo$Run, e$eventInfo$EventCounter, "\n")
  }
  
  my.info <- e$eventInfo
  my.trg <- e$triggers

  if (!is.na(my.master.trigger.col) && my.trg[[my.master.trigger.col]][1] > 0) {
    my.last.master.run <<- my.info$Run
    my.last.master.eventCounter <<- my.info$EventCounter
    my.last.master.timeStamp <<- my.info$DeviceTimeStamp
    my.last.master.trigger.position <<- my.trg[[my.master.trigger.col]][4]
    my.mc <- get.pulse(
      e$waveforms[[my.master.trigger.col]],
      front.len = my.opt$options$masterfront,
      tail.len = my.opt$options$mastertail,
      n.skip = my.opt$options$masterskip
    )
    if (!is.null(my.mc)) {
      my.last.master.peak.position <<- my.mc$x
    } else {
      my.last.master.peak.position <<- NA
    }
  }

  if (is.na(my.trigger.col) || my.trg[[my.trigger.col]][1] > 0) {
    my.wf <- e$waveforms[[my.channel.col]]
    if (!is.null(my.wf)) {
      my.trg.ch <- 0
      my.trg.pos <- 0
      for (col in colnames(my.trg)) {
        if (my.trg[[col]][1]) {
          my.trg.pos <- my.trg[[col]][4]
          break;
        }
        my.trg.ch <- my.trg.ch + 1  
      }
      
      my.print.channel(my.info, my.wf, my.trg.ch, my.trg.pos)
    }
  }

  return(a + 1)  
}

my.make.filename <- function(opt) {
  res <- paste(opt$args[1], "/peaks.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$master)) {
    res <- paste(res, ".mst", opt$options$master, sep = "")
  }
  
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }
  
  if (length(my.opt$args) == 2) {
    runname <- strsplit(basename(my.opt$args[2]), "\\.")[[1]][1]
    res <- paste(res, runname, sep = ".")
  }
  
  res <- paste(res, "txt", sep = ".")
  
  return (res)
}

my.make.column.name <- function(channel) {
  if (is.na(channel)) {
    return (NA)
  } else {
    return (paste("CH", channel, sep = ""))
  }
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
    c("", "--front"),
    type = "integer",
    default = 10, 
    help = "Number of samples before the peak (default %default)"
  ),
  make_option(
    c("", "--tail"),
    type = "integer",
    default = 10, 
    help = "Number of samples before the peak (default %default)"
  ),
  make_option(
    c("-m", "--master"),
    type = "integer",
    default = NA, 
    help = "Trigger channel # used for time calcs."
  ),
  make_option(
    c("", "--masterfront"),
    type = "integer",
    default = 10, 
    help = "Number of samples before the peak in master channel (default %default)"
  ),
  make_option(
    c("", "--mastertail"),
    type = "integer",
    default = 10, 
    help = "Number of samples before the peak in master channel (default %default)"
  ),
  make_option(
    c("", "--masterskip"),
    type = "integer",
    default = 50, 
    help = "Number of samples after the peak to skip in master channel (default %default)"
  ),
  make_option(
    c("-n", "--number"),
    type = "integer",
    default = NA, 
    help = "Max number of waveforms to process"
  ),
  make_option(
    c("-p", "--precision"),
    type = "integer",
    default = 4, 
    help = "Number precision"
  ),
  make_option(
    c("", "--skip"),
    type = "integer",
    default = 50, 
    help = "Number of samples after the tail to exclude from mean level calcs"
  ),
  make_option(
    c("-t", "--trigger"),
    type = "integer",
    default = NA, 
    help = "Trigger channel #"
  ),
  make_option(
    c("-v", "--verbose"),
    action="store_true",
    default = FALSE, 
    help = "Verbose mode"
  )
)

my.opt <- parse_args(
  OptionParser(
    option_list = my.option.list,
    usage = "%prog [options] <destination directory> <MID file 1> [<MID file 2> ...]"
  ), 
  positional_arguments = c(2, Inf)
)

if (is.na(my.opt$options$channel)) {
  stop("ERROR: channel is not specified")
}

if (is.na(my.opt$options$master)) {
  stop("ERROR: master trigger is not specified")
}

my.midas.files <- tail(my.opt$args, n = -1)
my.channel.col <- my.make.column.name(my.opt$options$channel)
my.master.trigger.col <- my.make.column.name(my.opt$options$master)
my.trigger.col <- my.make.column.name(my.opt$options$trigger)

########################################################
# Processing
########################################################

my.dest <- file(my.make.filename(my.opt))
open(my.dest, "w")

my.res <- read.events.from.gdconvert(
  file.names = my.midas.files,
  merging.func = my.event.collector,
  init.value = 0,
  nevents = my.opt$options$number
)

close(my.dest)
