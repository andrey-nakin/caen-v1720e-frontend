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
    "TM",
    "RUN",
    "EC",
    "TS",
    "PP",
    "PA",
    "EPA",
    "FI",
    "TI",
    "\n"
  )
}

my.print.channel <- function(my.info, wf, triggers) {
  my.pos <- which.min(wf)
  my.front.pos <- my.pos - my.opt$options$front
  my.tail.pos <- my.pos + my.opt$options$tail
  my.skip.pos <- my.tail.pos + my.opt$options$skip + 1
  my.sum <- 0
  my.sum2 <- 0
  my.n <- 0
  my.front.sum <- 0
  my.tail.sum <- 0
  my.len <- length(wf)

  if (!(my.front.pos > 0) || my.tail.pos > my.len) {
    return(0)
  }

  my.front <- wf[my.front.pos : my.pos]
  my.tail <- wf[(my.pos + 1) : my.tail.pos]
  my.pulse <- append(my.front, my.tail)
  
  my.front.sum <- sum(my.front)
  my.tail.sum <- sum(my.tail)
  my.sum <- my.front.sum + my.tail.sum
  my.positions <- seq(0, length(my.opt$options$front) + length(my.opt$options$tail) + 1)
  my.mass.center <- sum(my.positions * my.pulse / my.sum)

  if (my.front.pos > 1) {
    my.sum <- sum(wf[1 : (my.front.pos - 1)])
    my.sum2 <- sum(wf[1 : (my.front.pos - 1)]^2)
    my.n <- my.front.pos - 1
  }
  
  if (my.skip.pos <= my.len) {
    my.sum <- my.sum + sum(wf[my.skip.pos : my.len])
    my.sum2 <- my.sum2 + sum(wf[my.skip.pos : my.len]^2)
    my.n <- my.n + my.len - my.skip.pos + 1
  }
  
  if (!(my.n > 1)) {
    return(0)
  }
  
  my.mean = my.sum / my.n
  my.var <- (my.n * my.sum2 - my.sum^2) / (my.n * (my.n - 1))
  my.std <- sqrt(my.var)
  
  my.amp = wf[my.pos] - my.mean
  my.amp.err = 0.5 + my.std / sqrt(my.n)
  my.front.sum <- my.front.sum - my.mean * (my.opt$options$front + 1)
  my.tail.sum <- my.tail.sum - my.mean * my.opt$options$tail
  
  cat(
    file = my.dest, 
    sep = "\t",
    my.info$TimeStamp,
    my.info$Run,
    my.info$EventCounter,
    my.info$DeviceTimeStamp,
    format(my.mass.center, digits = my.opt$options$precision),
    format(my.amp, digits = my.opt$options$precision),
    format(my.amp.err, digits = my.opt$options$precision),
    format(my.front.sum, digits = my.opt$options$precision),
    format(my.tail.sum, digits = my.opt$options$precision),
    "\n"
  )
  
  return(1)
}

my.find.master.peak.position <- function(wf) {
  my.pos <- which.min(wf)
  my.front.pos <- my.pos - my.opt$options$masterfront
  my.tail.pos <- my.pos + my.opt$options$mastertail
  my.len <- length(wf)
  
  if (!(my.front.pos > 0) || my.tail.pos > my.len) {
    return(NA)
  }
  
  my.front <- wf[my.front.pos : my.pos]
  my.tail <- wf[(my.pos + 1) : my.tail.pos]
  my.pulse <- append(my.front, my.tail)
  
  my.front.sum <- sum(my.front)
  my.tail.sum <- sum(my.tail)
  my.sum <- my.front.sum + my.tail.sum
  my.positions <- seq(my.pos - my.opt$options$masterfront, my.pos + my.opt$options$mastertail)
  my.mass.center <- sum(my.positions * my.pulse / my.sum)
  
  cat("MC", my.pos, my.mass.center, "\n")

  return(my.mass.center)
}

my.event.collector <- function(a, e) {
  if (a == 0) {
    my.print.init.info(e)
  } else if (a %% 10000 == 0) {
    cat(e$eventInfo$Run, e$eventInfo$EventCounter, "\n")
  }
  
  my.info <- e$eventInfo
  my.trg <- e$triggers

  if (!is.na(my.master.trigger.col) && my.trg[[my.master.trigger.col]][1] > 0) {
    my.last.master.run <<- my.info$Run
    my.last.master.eventCounter <<- my.info$EventCounter
    my.last.master.timeStamp <<- my.info$DeviceTimeStamp
    my.last.master.trigger.position <<- my.trg[[my.master.trigger.col]][4]
    my.last.master.peak.position <<- my.find.master.peak.position(e$waveforms[[my.master.trigger.col]])
  }

  my.wf <- e$waveforms[[my.channel.col]]
  if (!is.null(my.wf)) {
    my.print.channel(my.info, my.wf, my.trg)
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
  
  res <- paste(res, ".txt", sep = "")
  
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

my.event.filter <- NULL
if (!is.na(my.trigger.col)) {
  my.event.filter <- function(e) {
    if (e$triggers[[my.trigger.col]][1] > 0) {
      return(TRUE)
    }
    if (!is.na(my.master.trigger.col) && e$triggers[[my.master.trigger.col]][1] > 0) {
      return(TRUE)
    }
    return(FALSE)
  }
}

my.res <- read.events.from.gdconvert(
  file.names = my.midas.files,
  filter.func = my.event.filter,
  merging.func = my.event.collector,
  init.value = 0,
  nevents = my.opt$options$number
)

close(my.dest)
