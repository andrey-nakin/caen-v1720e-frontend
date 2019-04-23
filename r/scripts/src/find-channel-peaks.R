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
    "PAE",
    "FI",
    "TI",
    "\n"
  )
}

my.print.channel <- function(my.info, wf, triggers) {
  my.pos <- which.min(wf)
  my.front.pos <- my.pos - my.opt$options$front
  my.tail.pos <- my.pos + my.opt$options$last + 1
  my.sum <- 0
  my.sum2 <- 0
  my.n <- 0
  my.front.sum <- 0
  my.tail.sum <- 0
  my.len <- length(wf)
  
  if (my.front.pos > 0) {
    my.sum <- sum(wf[1 : my.front.pos])
    my.sum2 <- sum(wf[1 : my.front.pos]^2)
    my.n <- my.front.pos
    
    my.front.sum <- sum(wf[(my.front.pos + 1) : my.pos])
  }
  
  if (my.tail.pos <= my.len) {
    my.sum <- my.sum + sum(wf[my.tail.pos : my.len])
    my.sum2 <- my.sum2 + sum(wf[my.tail.pos : my.len]^2)
    my.n <- my.n + my.len - my.tail.pos + 1
    
    my.tail.sum <- sum(wf[(my.pos + 1) : (my.tail.pos - 1)])
  }
  
  my.mean = my.sum / my.n
  my.var <- (my.n * my.sum2 - my.sum^2) / (my.n * (my.n - 1))
  my.std <- sqrt(my.var)
  
  my.amp = wf[my.pos] - my.mean
  my.amp.err = 0.5 + my.std / sqrt(my.n)
  my.front.sum <- my.front.sum - my.mean * my.opt$options$front
  my.tail.sum <- my.tail.sum - my.mean * my.opt$options$last
  
  cat(
    file = my.dest, 
    sep = "\t",
    my.info$TimeStamp,
    my.info$Run,
    my.info$EventCounter,
    my.info$DeviceTimeStamp,
    my.pos - 1,
    format(abs(my.amp), digits = 2),
    format(my.amp.err, digits = 2),
    format(abs(my.front.sum), digits = 2),
    format(abs(my.tail.sum), digits = 2),
    "\n"
  )
}

my.event.collector <- function(a, e) {
  if (a == 0) {
    my.print.init.info(e)
  } else if (a %% 10000 == 0) {
    cat(e$eventInfo$Run, e$eventInfo$EventCounter, "\n")
  }
  
  my.trg <- e$triggers

  if (!is.na(my.master.trigger.col) && my.trg[[my.master.trigger.col]][1] > 0) {
    my.last.master.run <<- my.info$Run
    my.last.master.eventCounter <<- my.info$EventCounter
    my.last.master.timeStamp <<- my.info$DeviceTimeStamp
    my.last.master.trigger.position <<-my.trg[[my.master.trigger.col]][4]
  }

  my.wf <- e$waveforms[[my.channel.col]]
  if (!is.null(my.wf)) {
    my.print.channel(e$eventInfo, my.wf, my.trg)
  }

  return(a + 1)  
}

my.make.filename <- function(opt) {
  res <- paste(opt$args[1], "/peaks.ch", opt$options$channel, sep = "")
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }
  if (!is.na(opt$options$master)) {
    res <- paste(res, ".mst", opt$options$master, sep = "")
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
    c("-f", "--front"),
    type = "integer",
    default = 10, 
    help = "Number of samples before the peak (default %default)"
  ),
  make_option(
    c("-l", "--last"),
    type = "integer",
    default = 10, 
    help = "Number of samples after the peak (default %default)"
  ),
  make_option(
    c("-m", "--master"),
    type = "integer",
    default = NA, 
    help = "Master trigger channel #"
  ),
  make_option(
    c("-n", "--number"),
    type = "integer",
    default = NA, 
    help = "Max number of waveforms to process"
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

my.midas.files <- tail(my.opt$args, n = -1)
my.channel.col <- my.make.column.name(my.opt$options$channel)
my.trigger.col <- my.make.column.name(my.opt$options$trigger)
my.master.trigger.col <- my.make.column.name(my.opt$options$master)

########################################################
# Processing
########################################################

my.dest <- file(my.make.filename(my.opt))
open(my.dest, "w")

my.event.filter <- NULL
if (!is.na(my.trigger.col)) {
  my.event.filter <- function(e) {
    return(e$triggers[[my.trigger.col]][1] > 0)
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
