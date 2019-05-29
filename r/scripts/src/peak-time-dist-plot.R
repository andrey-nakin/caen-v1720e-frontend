#!/usr/bin/Rscript

library(optparse)
library(ggplot2)
library(plotly)
library(tidyverse)
library(htmlwidgets)

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

my.make.dest.filename <- function(opt, dir, ext, suffix = NULL) {
  res <- paste(dir, "/peaks.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$master)) {
    res <- paste(res, ".mst", opt$options$master, sep = "")
  }
  
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }

  res <- paste(res, paste("time", opt$options$step, sep = ""), "dist", sep = ".")
  if (!is.null(suffix)) {
    res <- paste(res, suffix, sep = ".")
  }
  res <- paste(res, ext, sep = ".")
  
  return (res)
}

my.make.plot.filename <- function(opt, suffix = NULL) {
  return(
    my.make.dest.filename(opt, opt$args[2], "pdf", suffix = suffix)
  )
}

my.make.html.filename <- function(opt, suffix = NULL) {
  return(
    my.make.dest.filename(opt, opt$args[3], "html", suffix = suffix)
  )
}

my.make.result.filename <- function(opt, suffix = NULL) {
  return(
    my.make.dest.filename(opt, opt$args[1], "txt", suffix = suffix)
  )
}

my.extract.run.no <- function(fn) {
  return(as.numeric(sub(".*\\.run([0-9]+)\\..*", "\\1", basename(fn), perl=TRUE)))
}

my.process.file <- function(fn, accum, breaks) {

  my.df <- read.table(fn, header = T, sep = "", strip.white = T)
  my.min <- my.opt$options$min
  my.max <- my.opt$options$max
  my.x <- my.df$PP
  my.total <<- my.total + length(my.x)
  my.x <- my.x[which(my.x >= my.min & my.x < my.max)]
  my.count <<- my.count + length(my.x)
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

  pdf(my.make.plot.filename(my.opt))
  
  my.p <- ggplot(
    data = my.data, 
    aes(x = x, y = y)
  ) + 
    geom_line() +
    ggtitle(
      label = my.plot.title("Time Distribution"),
      subtitle = paste(
        paste("Total", format(my.total, digits = 2), sep = ": "),
        paste("Counted", format(my.count, digits = 2), sep = ": "),
        sep = ", "
      )
    ) +
    scale_x_continuous(name = "Time, channels") + 
    scale_y_continuous(name = "Count") +
    theme_light()

  plot(my.p)
  
  htmlwidgets::saveWidget(ggplotly(my.p), my.make.html.filename(my.opt))
  
}

my.write.result <- function() {
  my.df <- data.frame(
    TIME = my.x,
    COUNT = my.accum
  )
  write.table(
    my.df, 
    file = my.make.result.filename(my.opt),
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
    help = "Step"
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
    usage = "%prog [options] <txt file directory> <plot file directory> <html file directory>"
  ), 
  positional_arguments = c(3, 3)
)

########################################################
# Processing
########################################################

my.total <- 0
my.count <- 0
my.accum <- NULL
my.breaks <- seq(from = my.opt$options$min, to = my.opt$options$max, by = my.opt$options$step)
my.x <- tail(my.breaks, n = -1) - my.opt$options$step / 2

for (my.fn in list.files(path = my.opt$args[1], pattern = my.make.txt.filename.mask(my.opt), full.names = T)) {
  my.accum <- my.process.file(my.fn, my.accum, my.breaks)
  if (!is.na(my.opt$options$number) && my.count >= my.opt$options$number) {
    break
  }
}

my.der.x <- tail(my.x, n = -1) - my.opt$options$step / 2
my.der.y <- diff(my.accum) / my.opt$options$step

my.write.result()

my.plot()
