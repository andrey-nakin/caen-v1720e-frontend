#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Functions
########################################################

my.process.waveform <- function(wf) {
  my.pulse <- gneis.daq::get.pulse(
    wf,
    front.len = my.opt$options$front + my.opt$options$padding,
    tail.len = my.opt$options$tail + my.opt$options$padding,
    n.skip = my.opt$options$skip,
    need.pulse = TRUE
  )
  if (is.null(my.pulse)) {
    return(0)
  }
  
  
  my.len <- length(wf)
  my.pos <- which.min(wf)
  my.front.pos <- my.pos - my.opt$options$front
  my.tail.pos <- my.pos + my.opt$options$last
  
  if (my.front.pos > 1 && my.tail.pos < my.len) {
    my.sum <- sum(wf[1 : (my.front.pos - 1)]) + sum(wf[(my.tail.pos + 1) : my.len])
    my.n <- my.len - (my.opt$options$front + my.opt$options$last + 1)
    my.mean <- my.sum / my.n
    my.amp = wf[my.pos] - my.mean
    
    if (abs(my.amp) > my.opt$options$minamp && (my.opt$options$maxamp < 0 || abs(my.amp) <= my.opt$options$maxamp)) {
      my.peak <- wf[my.front.pos : my.tail.pos] - my.mean
      
      my.wf.sum <<- my.wf.sum + my.peak
      my.wf.sum2 <<- my.wf.sum2 + my.peak^2
      
      my.wf.count <<- my.wf.count + 1
      
      return(1)
    }
  }
}

my.event.collector <- function(a, e) {
  my.res <- my.process.waveform(e$waveforms[[my.channel.column]])
  if (!is.null(my.res)) {
    return(a + my.res)
  } else {
    return(a)
  }
}

my.stop.func <- function(a) {
  return (!is.na(my.opt$options$number) && my.opt$options$number > 0 && !(a < my.opt$options$number))
}

my.make.filename <- function(opt) {
  res <- paste(opt$args[1], "/avg-peak-form.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, "trg", opt$options$trigger, sep = ".")
  }
  
  res <- paste(res, paste("amp", opt$options$minamp, "-", opt$options$maxamp, sep=""), sep = ".")
  
  if (length(opt$args) == 2) {
    runname <- strsplit(basename(opt$args[2]), "\\.")[[1]][1]
    res <- paste(res, runname, sep = ".")
  }
  
  res <- paste(res, "txt", sep = ".")
  
  return (res)
}

########################################################
# Command line parsing
########################################################

my.option.list <- list( 
  make_option(
    c("-a", "--minamp"),
    type = "double",
    default = 1000, 
    help = "Min value of peak amplitude (default %default)"
  ),
  make_option(
    c("-b", "--maxamp"),
    type = "double",
    default = 100000, 
    help = "Max value of peak amplitude (default %default)"
  ),
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
    c("", "--skip"),
    type = "integer",
    default = 50, 
    help = "Number of samples after the tail to exclude from mean level calcs"
  ),
  make_option(
    c("-n", "--number"),
    type = "integer",
    default = NA, 
    help = "Max number of waveforms to process"
  ),
  make_option(
    c("-p", "--padding"),
    type = "integer",
    default = 5, 
    help = "Pulse padding (default %default)"
  ),
  make_option(
    c("-r", "--resolution"),
    type = "double",
    default = 0.1, 
    help = "Pulse form resolution (default %default)"
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
    usage = "%prog [options] <txt file directory> <MID file 1> [<MID file 2> ...]"
  ), 
  positional_arguments = c(2, Inf)
)

if (is.na(my.opt$options$channel)) {
  stop("ERROR: channel is not specified")
}

my.channel.column <- paste("CH", my.opt$options$channel, sep = "")
if (!is.na(my.opt$options$trigger)) {
  my.trigger.column <- paste("CH", my.opt$options$trigger, sep = "")
} else {
  my.trigger.column <- NA
}
my.midas.files <- tail(my.opt$args, n = -1)

########################################################
# Processing
########################################################

my.wf.len <- round((my.opt$options$front + my.opt$options$tail + 1 + 2 * my.opt$options$padding) / my.opt$options$resolution)
my.wf.sum <- array(rep(0, times = my.wf.len))
my.wf.sum2 <- array(rep(0, times = my.wf.len))
my.wf.count <- array(rep(0, times = my.wf.len))

my.event.filter <- NULL
if (!is.na(my.trigger.column)) {
  my.event.filter <- function(e) {
    return(e$triggers[[my.trigger.column]][1] > 0)
  }
}

my.dest <- file(my.make.filename(my.opt))
open(my.dest, "w")

my.res <- read.events.from.gdconvert(
  file.names = my.midas.files,
  filter.func = my.event.filter,
  merging.func = my.event.collector,
  init.value = 0,
  stop.func = my.stop.func
)

close(my.dest)

#warnings()