#!/usr/bin/Rscript

library(optparse)
library(ggplot2)

########################################################
# Functions
########################################################

my.make.txt.filename.mask <- function(opt) {
  res <- paste("peaks.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$master)) {
    res <- paste(res, ".mst", opt$options$master, sep = "")
  }
  
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }

  res <- paste(res, "run[0-9]+", "txt", sep = ".")
  return (res)
}

my.make.dest.filename <- function(opt, dir, ext) {
  res <- paste(dir, "/peaks.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$master)) {
    res <- paste(res, ".mst", opt$options$master, sep = "")
  }
  
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }

  res <- paste(res, ext, sep = ".")
  
  return (res)
}

my.make.plot.filename <- function(opt) {
  return(
    my.make.dest.filename(opt, opt$args[2], "pdf"
    )
  )
}

my.extract.run.no <- function(fn) {
  return(as.numeric(sub(".*\\.run([0-9]+)\\..*", "\\1", basename(fn), perl=TRUE)))
}

my.process.file <- function(fn, accum) {

  my.df <- read.table(fn, header = T, sep = "", strip.white = T)
  
  my.res <- list(
    run.no = my.extract.run.no(fn),
    count = length(my.df$PP),
    pp.mean = mean(my.df$PP),
    pp.sd = sd(my.df$PP),
    epp.mean = mean(my.df$EPP),
    pa.mean = mean(my.df$PA),
    pa.sd = sd(my.df$PA),
    epa.mean = mean(my.df$EPA[which(!is.na(my.df$EPA))])
  )

  if (is.null(accum)) {
    return(my.res)
  }
  
  accum <- list(
    run.no = append(accum$run.no, my.res$run.no),
    count = append(accum$count, my.res$count),
    pp.mean = append(accum$pp.mean, my.res$pp.mean),
    pp.sd = append(accum$pp.sd, my.res$pp.sd),
    epp.mean = append(accum$epp.mean, my.res$epp.mean),
    pa.mean = append(accum$pa.mean, my.res$pa.mean),
    pa.sd = append(accum$pa.sd, my.res$pa.sd),
    epa.mean = append(accum$epa.mean, my.res$epa.mean)
  )

  return(accum)
  
}

my.plot.title <- function(main) {
  my.res <- paste(main, paste("Channel #", my.opt$options$channel, sep = ""), sep = ", ")
  if (!is.na(my.opt$options$master)) {
    my.res <- paste(my.res, paste("Master #", my.opt$options$master, sep = ""), sep = ", ")
  }
  if (!is.na(my.opt$options$trigger)) {
    my.res <- paste(my.res, paste("Trigger #", my.opt$options$trigger, sep = ""), sep = ", ")
  }
  return(my.res)
}

my.plot.stat <- function(x, y, main, ylab, units) {
  my.data <- data.frame(x = x, y = y)
  my.lm <- lm(y ~ x, data = my.data)
  my.coef <- coefficients(my.lm)
  my.slope <- my.coef[2]
  my.slope.ci <- confint(my.lm, 'x', level = 0.95)
  my.slope.err = max(abs(my.slope - my.slope.ci[1]), abs(my.slope - my.slope.ci[2]))
  
  p <- ggplot(
    data = my.data, 
    aes(x = x, y = y)
  ) + 
    geom_line() +
    ggtitle(
      label = my.plot.title(main),
      subtitle = paste("Approximation Line Slope: ", sprintf("%0.2g +/- %0.1g", my.slope, my.slope.err), " ", units, "/run", sep = "")
    ) +
    scale_x_continuous(name = "Run #") + 
    scale_y_continuous(name = paste(ylab, ", ", units, sep = "")) +
    theme_light() +
    geom_abline(intercept = my.coef[1], slope = my.coef[2], color="#00a000", linetype="dotted")
  
  plot(p)
  
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
  )
)

my.opt <- parse_args(
  OptionParser(
    option_list = my.option.list,
    usage = "%prog [options] <txt file directory> <plot file directory>"
  ), 
  positional_arguments = c(2, 2)
)

########################################################
# Processing
########################################################

my.accum <- NULL
for (my.fn in list.files(path = my.opt$args[1], pattern = my.make.txt.filename.mask(my.opt), full.names = T)) {
  my.accum <- my.process.file(my.fn, my.accum)
}

pdf(my.make.plot.filename(my.opt))

my.plot.stat(my.accum$run.no, my.accum$count, "Trigger Count", "Count", "")

my.plot.stat(my.accum$run.no, my.accum$pp.mean, "Mean Peak Position", "Peak Position", "channels")
my.plot.stat(my.accum$run.no, my.accum$pp.sd, "St. Dev. of Peak Position", "Standard Deviation", "channels")
my.plot.stat(my.accum$run.no, my.accum$epp.mean, "Mean Peak Position Error", "Peak Position Error", "channels")

my.plot.stat(my.accum$run.no, my.accum$pa.mean, "Mean Peak Amplitude", "Peak Amplitude", "channels")
my.plot.stat(my.accum$run.no, my.accum$pa.sd, "St. Dev. of Peak Amplitude", "Standard Deviation", "channels")
my.plot.stat(my.accum$run.no, my.accum$epa.mean, "Mean Peak Amplitude Error", "Peak Amplitude Error", "channels")
