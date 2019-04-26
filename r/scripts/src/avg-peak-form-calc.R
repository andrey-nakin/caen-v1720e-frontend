#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Functions
########################################################

xx <- array(dim = c(0))
yy <- array(dim = c(0))

my.process.waveform <- function(wf) {
  my.pulse <- gneis.daq::get.pulse(
    wf,
    front.len = my.opt$options$front + my.opt$options$padding,
    tail.len = my.opt$options$tail + my.opt$options$padding,
    n.skip = my.opt$options$skip,
    need.pulse = TRUE
  )
  
  if (is.null(my.pulse)) {
    return()
  }
  
  if (abs(my.pulse$amp) < my.opt$options$minamp || abs(my.pulse$amp) > my.opt$options$maxamp) {
    return()
  }

  xx <<- append(xx, my.pulse$positions - my.pulse$x)
  yy <<- append(yy, my.pulse$pulse)

  return(1)
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
  
  res <- paste(res, sprintf("amp%d-%d", opt$options$minamp, opt$options$maxamp), sep = ".")
  
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

my.event.filter <- NULL
if (!is.na(my.trigger.column)) {
  my.event.filter <- function(e) {
    return(e$triggers[[my.trigger.column]][1] > 0)
  }
}

my.res <- read.events.from.gdconvert(
  file.names = my.midas.files,
  filter.func = my.event.filter,
  merging.func = my.event.collector,
  init.value = 0,
  stop.func = my.stop.func
)

my.dest <- file(my.make.filename(my.opt))
open(my.dest, "w")

cat(
  file = my.dest, 
  "N", 
  "MX", 
  "MX2",
  "MY",
  "MY2",
  "\n", 
  sep = "\t"
)

for (a in seq(from = -my.opt$options$front, to = my.opt$options$tail, by = my.opt$options$resolution)) {

  i <- which(xx >= a & xx < a + my.opt$options$resolution)
  x <- xx[i]
  y <- yy[i]
  n <- length(y)

  cat(
    file = my.dest, 
    n, 
    mean(x),
    sum(x^2) / n,
    mean(y), 
    sum(y^2) / n,
    "\n", 
    sep = "\t"
  )

}

close(my.dest)
