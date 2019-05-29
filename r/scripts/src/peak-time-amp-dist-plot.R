#!/usr/bin/Rscript

library(optparse)

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

  res <- paste(
    res, 
    paste("amp", my.opt$options$ampstep, sep = ""), 
    paste("time", my.opt$options$timestep, sep = ""), 
    "dist", 
    sep = "."
  )
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
  my.x <- abs(my.df$PA)
  my.y <- my.df$PP
  
  if (!is.na(my.opt$options$trigger)) {
    my.idx <- which(my.df$TRG == my.opt$options$trigger)
    my.x <- my.x[my.idx]
    my.y <- my.y[my.idx]
  }

  my.idx <- which(my.x >= my.opt$options$ampmin & 
                    my.x < my.opt$options$ampmax & 
                    my.y >= my.opt$options$timemin & 
                    my.y < my.opt$options$timemax)
  my.x <- my.x[my.idx]
  my.y <- my.y[my.idx]
  my.value.count <<- my.value.count + length(my.x)
  
  l <- lapply(tail(my.x.breaks, n = -1), function(xmax) {
    xmin <- xmax - my.opt$options$ampstep
    my.h.y <- hist(my.y[which(my.x >= xmin & my.x < xmax)], breaks = my.y.breaks, plot = F)
    return(my.h.y$counts) 
  })
  my.m <- do.call(cbind, l)

  if (is.null(accum)) {
    return(my.m)
  }
  
  return(accum + my.m)
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

my.write.result <- function() {
  cat("writing to ", my.make.result.filename(my.opt), "\n")
  write.table(
    my.accum, 
    file = my.make.result.filename(my.opt),
    sep = "\t",
    row.names = F,
    col.names = F
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
    c("", "--timemin"),
    type = "double",
    default = 0, 
    help = "Time min value"
  ),
  make_option(
    c("", "--timemax"),
    type = "double",
    default = NA, 
    help = "Time max value"
  ),
  make_option(
    c("", "--timestep"),
    type = "double",
    default = 1, 
    help = "Time distribution step"
  ),
  make_option(
    c("", "--ampmin"),
    type = "double",
    default = 0, 
    help = "Amplitude min value"
  ),
  make_option(
    c("", "--ampmax"),
    type = "double",
    default = NA, 
    help = "Amplitude max value"
  ),
  make_option(
    c("", "--ampstep"),
    type = "double",
    default = 1, 
    help = "Amplitude distribution step"
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
    usage = "%prog [options] <txt file directory>"
  ), 
  positional_arguments = c(1, 1)
)

########################################################
# Processing
########################################################

my.value.count <- 0
my.accum <- NULL
my.x.breaks <- seq(from = my.opt$options$ampmin, to = my.opt$options$ampmax, by = my.opt$options$ampstep)
my.y.breaks <- seq(from = my.opt$options$timemin, to = my.opt$options$timemax, by = my.opt$options$timestep)

my.files <- list.files(path = my.opt$args[1], pattern = my.make.txt.filename.mask(my.opt), full.names = T)
if (length(my.files) == 0) {
  my.files <- list.files(path = my.opt$args[1], pattern = my.make.txt.filename.mask(my.opt, notrigger = T), full.names = T)
}

for (my.fn in my.files) {
  my.accum <- my.process.file(my.fn, my.accum)
  if (!is.na(my.opt$options$number) && my.value.count >= my.opt$options$number) {
    break
  }
}

my.write.result()
