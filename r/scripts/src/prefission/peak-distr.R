#!/usr/bin/Rscript

library(optparse)
library(ggplot2)
library(stats)
library(plotly)
library(tidyverse)
library(htmlwidgets)

########################################################
# Functions
########################################################

my.make.txt.filename.mask <- function(opt, notrigger = FALSE) {
  res <- paste("peaks.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$master)) {
    res <- paste(res, ".mst", opt$options$master, sep = "")
  }
  
  if (!is.na(opt$options$trigger) && !notrigger) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }
  
  res <- paste(res, "run[0-9]+", "txt", sep = ".")
  return (res)
}

my.make.dest.filename <- function(opt, dir, ext, suffix = NULL) {
  res <- paste(dir, "/peaks.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$master)) {
    res <- paste(res, ".mst", opt$options$master, sep = "")
  }
  
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }
  
  res <- paste(res, "checked", paste("amp", opt$options$step, sep = ""), "dist", sep = ".")
  if (!is.null(suffix)) {
    res <- paste(res, suffix, sep = ".")
  }
  res <- paste(res, ext, sep = ".")
  
  return (res)
}

my.make.plot.filename <- function(opt, suffix = NULL) {
  return(
    my.make.dest.filename(opt, opt$args[3], "pdf", suffix = suffix)
  )
}

my.make.html.filename <- function(opt, suffix = NULL) {
  return(
    my.make.dest.filename(opt, opt$args[4], "html", suffix = suffix)
  )
}

my.make.result.filename <- function(opt, suffix = NULL) {
  return(
    my.make.dest.filename(opt, opt$args[2], "txt", suffix = suffix)
  )
}

my.extract.run <- function(fn) {
  return(sub(".*\\.(run[0-9]+)\\..*", "\\1", basename(fn), perl=TRUE))
}

my.make.channel.fn <- function(fn, channel) {
  my.res <- sub("\\.ch([0-9])\\.", paste(".ch", channel, ".", sep = ""), fn, perl = T)
  return(my.res)
}

my.process.file <- function(fn, accum, breaks) {
  
  my.df1 <- read.table(my.make.channel.fn(fn, 1), header = T)
  my.df2 <- read.table(my.make.channel.fn(fn, 2), header = T)
  my.df3 <- read.table(my.make.channel.fn(fn, 3), header = T)
  my.df4 <- read.table(my.make.channel.fn(fn, 4), header = T)
  my.df5 <- read.table(my.make.channel.fn(fn, 5), header = T)

  my.ec1 <- my.df1$EC[which(abs(my.df1$PA) >= 75)]
  my.ec2 <- my.df2$EC[which(abs(my.df2$PA) >= 65)]
  my.ec3 <- my.df3$EC[which(abs(my.df3$PA) >= 35)]
  my.ec4 <- my.df4$EC[which(abs(my.df4$PA) >= 35)]
  my.ec <- unique(append(my.ec1, append(my.ec2, append(my.ec3, my.ec4))))
  my.ec <- sort(my.ec)
  cat("filtering...", length(my.df5$EC), length(my.ec), "\n")
  my.x <- abs(my.df5$PA[which(my.df5$EC %in% my.ec)])
  cat("filtering done", length(my.x), "\n")
  my.x <- my.x[which(my.x >= my.opt$options$min & my.x < my.opt$options$max)]
  
  my.total <<- my.total + length(my.x)
  my.h <- hist(my.x, breaks = breaks, plot = F)
  
  if (is.null(accum)) {
    return(my.h$counts)
  }
  
  return(accum + my.h$counts)
  
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

my.plot <- function() {
  my.data <- data.frame(
    x = my.x,
    y = my.accum
  )
  my.ci <- stats::binom.test(x = max(my.accum), n = sum(my.accum), p = 1/length(my.accum), conf.level = 0.95)$conf.int
  my.err.min <- (my.ci[2] - my.ci[1]) / 2
  
  pdf(my.make.plot.filename(my.opt))
  
  my.p <- ggplot(
    data = my.data, 
    aes(x = x, y = y)
  ) + 
    geom_line() +
    ggtitle(
      label = my.plot.title("Amplitude Distribution"),
      subtitle = paste(
        paste("Total", format(my.total, digits = 2), sep = ": "),
        #      paste("MinE", format(my.err.min, digits = 2), sep = ": "),
        sep = ", "
      )
    ) +
    scale_x_continuous(name = "Absolute Amplitude, channels") + 
    scale_y_continuous(name = "Count") +
    theme_light()
  
  plot(my.p)
  
  htmlwidgets::saveWidget(ggplotly(my.p), my.make.html.filename(my.opt))
  
}

my.plot.derivative <- function() {
  my.data <- data.frame(
    x = my.der.x,
    y = my.der.y
  )
  
  pdf(my.make.plot.filename(my.opt, suffix = "der"))
  
  my.p <- ggplot(
    data = my.data, 
    aes(x = x, y = y)
  ) + 
    geom_line() +
    ggtitle(
      label = my.plot.title("Amplitude Distribution Derivative")
    ) +
    scale_x_continuous(name = "Absolute Amplitude, channels") + 
    scale_y_continuous(name = "Derivative") +
    theme_light()
  
  plot(my.p)
  
  htmlwidgets::saveWidget(ggplotly(my.p), my.make.html.filename(my.opt, suffix = "der"))
  
}

my.write.result <- function() {
  my.df <- data.frame(
    AMP = my.x,
    COUNT = my.accum
  )
  write.table(
    my.df, 
    file = my.make.result.filename(my.opt),
    sep = "\t",
    row.names = F
  )
  
  my.df <- data.frame(
    AMP = my.der.x,
    DERIVATIVE = my.der.y
  )
  write.table(
    my.df, 
    file = my.make.result.filename(my.opt, suffix = "der"),
    sep = "\t",
    row.names = F
  )
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
    c("", "--min"),
    type = "double",
    default = 0, 
    help = "Min value"
  ),
  make_option(
    c("", "--max"),
    type = "double",
    default = NA, 
    help = "Max value"
  ),
  make_option(
    c("", "--step"),
    type = "double",
    default = 1, 
    help = "Distribution step"
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
    usage = "%prog [options] <src file dir> <txt file dir> <plot file dir> <html file dir>"
  ), 
  positional_arguments = c(4, 4)
)

########################################################
# Processing
########################################################

my.total <- 0
my.accum <- NULL
my.breaks <- seq(from = my.opt$options$min, to = my.opt$options$max, by = my.opt$options$step)
my.x <- tail(my.breaks, n = -1) - my.opt$options$step / 2

my.files <- list.files(path = my.opt$args[1], pattern = my.make.txt.filename.mask(my.opt), full.names = T)
if (length(my.files) == 0) {
  my.files <- list.files(path = my.opt$args[1], pattern = my.make.txt.filename.mask(my.opt, notrigger = T), full.names = T)
}
for (my.fn in my.files) {
  my.accum <- my.process.file(my.fn, my.accum, my.breaks)
  if (!is.na(my.opt$options$number) && my.total >= my.opt$options$number) {
    break
  }
}

my.der.x <- tail(my.x, n = -1) - my.opt$options$step / 2
my.der.y <- diff(my.accum) / my.opt$options$step

my.write.result()

my.plot()
my.plot.derivative()
