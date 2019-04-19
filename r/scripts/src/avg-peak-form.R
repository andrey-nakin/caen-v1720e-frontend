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
    c("-c", "--channel"),
    type = "integer",
    default = 0, 
    help = "Channel # (default %default)"
  ),
  make_option(
    c("-f", "--front"),
    type = "integer",
    default = 4, 
    help = "Number of samples before the peak (default %default)"
  ),
  make_option(
    c("-l", "--last"),
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

my.wf.sum <- array(rep(0, times = my.opt$options$front + my.opt$options$last + 1))
my.wf.sum2 <- array(rep(0, times = my.opt$options$front + my.opt$options$last + 1))
my.wf.count <- 0

my.channel.column <- paste("CH", my.opt$options$channel, sep = "")

my.event.filter <- NULL
if (!(my.opt$options$trigger < 0)) {
  my.trigger.column <- paste("CH", my.opt$options$trigger, sep = "")
  my.event.filter <- function(e) {
    return(e$triggers[[my.trigger.column]][1] > 0)
  }
}

my.process.waveform <- function(wf) {
  my.len <- length(wf)
  my.pos <- which.min(wf)
  my.front.pos <- my.pos - my.opt$options$front
  my.tail.pos <- my.pos + my.opt$options$last
  
  if (my.front.pos > 1 && my.tail.pos < my.len) {
    my.sum <- sum(wf[1 : (my.front.pos - 1)]) + sum(wf[(my.tail.pos + 1) : my.len])
    my.n <- my.len - (my.opt$options$front + my.opt$options$last + 1)
    my.mean <- my.sum / my.n
    my.amp = wf[my.pos] - my.mean
    
    if (abs(my.amp) > my.opt$options$amplitude) {
      my.peak <- wf[my.front.pos : my.tail.pos] - my.mean

      my.wf.sum <<- my.wf.sum + my.peak
      my.wf.sum2 <<- my.wf.sum2 + my.peak^2
      my.wf.count <<- my.wf.count + 1
      
      return(1)
    }
  }
}

my.event.collector <- function(a, e) {
  my.process.waveform(e$waveforms[[my.channel.column]])
}

my.res <- read.events.from.gdconvert(
  file.names = my.midas.files,
  filter.func = my.event.filter,
  merging.func = my.event.collector,
  init.value = 0,
  nevents = 1000
)

my.wf.avg <- my.wf.sum / my.wf.count
my.wf.var <- (my.wf.count * my.wf.sum2 - my.wf.sum^2) / (my.wf.count * (my.wf.count - 1))
my.wf.std <- sqrt(my.wf.var)
my.wf.avg.err <- my.wf.std / sqrt(my.wf.count)

cat(my.wf.avg, "\n")
cat(my.wf.avg.err, "\n")

pdf(my.plot.file)
plot(my.wf.avg, type='b', pch=19)
