#!/usr/bin/Rscript

library(optparse)
library(ggplot2)
library(gneis.daq)

########################################################
# Functions
########################################################

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
    
    if (abs(my.amp) > my.opt$options$minamp && (my.opt$options$maxamp < 0 || abs(my.amp) <= my.opt$options$maxamp)) {
      my.peak <- wf[my.front.pos : my.tail.pos] - my.mean
      
      my.wf.sum <<- my.wf.sum + my.peak
      my.wf.sum2 <<- my.wf.sum2 + my.peak^2
      
      # if (my.wf.count > 0) {
      #   my.wf.min <<- pmin(my.wf.min, my.peak)
      #   my.wf.max <<- pmax(my.wf.max, my.peak)
      # } else {
      #   my.wf.min <<- my.peak
      #   my.wf.max <<- my.peak
      # }
      
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
  return (my.opt$options$number > 0 && a >= my.opt$options$number)
}

my.plot <- function() {
  my.df <- data.frame(
    x = rep(my.x, 3),
    y = append(append(my.wf.avg, my.wf.avg + qnorm(0.10) * my.wf.std), my.wf.avg + qnorm(0.90) * my.wf.std),
    c = append(append(rep("a) Mean", length(my.wf.avg)), rep("b) 10th Percentile", length(my.wf.avg))), rep("c) 90th Percentile", length(my.wf.avg))),
    err = append(my.wf.avg.err * qnorm(0.975), rep(NA, 2 * length(my.wf.avg.err)))
  )

  pdf(my.plot.file)
  
  my.p <- ggplot(data = my.df, aes(x = x, y = y, group = c, color = c)) +
    ggtitle(
      label = paste(
        "Averaged Pulse", 
        paste("Channel #", my.opt$options$channel, sep = ""), 
        paste("Trigger #", my.opt$options$trigger, sep = ""), 
        paste("Amplitudes", paste(my.opt$options$minamp, my.opt$options$maxamp, sep = "-"), sep = ": "),
        sep = ", "
      ),
      subtitle = paste(
        paste("Peak Pos", my.opt$options$front, sep = ": "),
        paste("Peak Amp", format(my.wf.avg[my.opt$options$front + 1], digits = 2), sep = ": "),
        paste("Num of WFs", my.wf.count, sep = ": "),
        paste("10/90 Front Length", format(my.root2 - my.root1, digits = 2), sep = ": "),
        sep = ", "
      )
    ) +
    scale_x_continuous(name = "Time, channels") +
    scale_y_continuous(name = "Amplitude, channels") +
    theme_light(base_size = 10) +
    geom_errorbar(aes(ymin=y - err, ymax = y + err), width = .3, position = position_dodge(0.05)) +
    geom_line() +
    geom_line(aes(x = x, y = rep(my.level1, length(x))), size = 0.25, color = 'magenta') +
    geom_line(aes(x = x, y = rep(my.level2, length(x))), size = 0.25, color = 'magenta') +
    geom_point(aes(shape = c)) + scale_shape_manual(values=c(20, NA, NA)) +
    warnings()
  
  plot(my.p)  
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
    default = 0, 
    help = "Channel # (default %default)"
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
    c("-n", "--number"),
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

CONF_LEVEL <- 2

my.wf.sum <- array(rep(0, times = my.opt$options$front + my.opt$options$last + 1))
my.wf.sum2 <- array(rep(0, times = my.opt$options$front + my.opt$options$last + 1))
my.wf.min <- array(rep(0, times = my.opt$options$front + my.opt$options$last + 1))
my.wf.max <- array(rep(0, times = my.opt$options$front + my.opt$options$last + 1))
my.wf.count <- 0

my.channel.column <- paste("CH", my.opt$options$channel, sep = "")

my.event.filter <- NULL
if (!(my.opt$options$trigger < 0)) {
  my.trigger.column <- paste("CH", my.opt$options$trigger, sep = "")
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

my.wf.avg <- my.wf.sum / my.wf.count
my.wf.var <- (my.wf.count * my.wf.sum2 - my.wf.sum^2) / (my.wf.count * (my.wf.count - 1))
my.wf.std <- sqrt(my.wf.var)
my.wf.avg.err <- my.wf.std / sqrt(my.wf.count)
my.x <- seq(0, length(my.wf.avg) - 1)

my.func <- approxfun(my.x, my.wf.avg)
my.level1 <- 0.1 * (my.wf.avg[my.opt$options$front + 1] - my.wf.avg[1]) + my.wf.avg[1]
my.level2 <- 0.9 * (my.wf.avg[my.opt$options$front + 1] - my.wf.avg[1]) + my.wf.avg[1]
if (my.level2 < my.level1) {
  cat(my.wf.avg[my.opt$options$front + 1], my.wf.avg[1], my.level1, my.level2, "\n")
  my.root1 <- uniroot(f = function(x) my.func(x) - my.level1, interval = c(0, my.opt$options$front))$root
  my.root2 <- uniroot(f = function(x) my.func(x) - my.level2, interval = c(0, my.opt$options$front))$root
} else {
  my.root1 <- 0
  my.root2 <- 0
}

my.plot()

#warnings()