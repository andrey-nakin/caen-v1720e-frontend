#!/usr/bin/Rscript

library(optparse)
library(gneis.daq)

########################################################
# Global variables
########################################################

########################################################
# Functions
########################################################

my.print.header <- function(e) {
  cat(
    file = my.dest, 
    sep = "\t",
    "DATETIME",
    "RUN",
    "EC",
    "TS",
    "TRG"
  )

  for (ch in my.channel.indices) {
    my.n <- ch - 1
    
    for (t in my.trigger.indices) {
      cat(
        file = my.dest, 
        sep = "\t",
        "",
        paste("CH", my.n, "_PP_M", t - 1, sep = ""),
        paste("CH", my.n, "_EPP_M", t - 1, sep = "")
      )
    }
    
    cat(
      file = my.dest, 
      sep = "\t",
      "",
      paste("CH", my.n, "_PA", sep = ""),
      paste("CH", my.n, "_EPA", sep = ""),
      paste("CH", my.n, "_FI", sep = ""),
      paste("CH", my.n, "_TI", sep = "")
    )
  }
  
  cat(
    file = my.dest, 
    sep = "\t",
    "",
    "\n"
  )
}

my.print.channel <- function(ch, my.info, trg.ch, trg.pos) {
  my.pulse <- my.pulses[[ch]]
  if (is.null(my.pulse)) {
    return()
  }

  for (t in my.trigger.indices) {
    if (!is.na(my.last.master.run[t]) && my.last.master.run[t] == my.info$Run && !is.null(my.pulses[[t]])) {
      my.pos <- (my.pulse$x - (trg.pos + 1)) -
        (my.pulses[[t]]$x - (my.last.master.trigger.position[t] + 1))  +
        timestamp.diff(my.info$DeviceTimeStamp, my.last.master.timeStamp[t]) * my.info$TicksPerSample
      my.pos.err <- my.pulse$x.err
      if (my.last.master.eventCounter[t] != my.info$EventCounter) {
        my.pos.err <- my.pos.err + 1
      }
    } else {
      my.pos <- my.pulse$x - (trg.pos + 1)
      my.pos.err <- my.pulse$x.err
    }

    if (my.pos > 9999) {
      my.pos <- round(my.pos)
    } else {
      my.pos <- format(my.pos, digits = my.opt$options$precision)
    }
    
    cat(
      file = my.dest, 
      sep = "\t",
      "",
      my.pos,
      my.pos.err
    )
  }
  
  cat(
    file = my.dest, 
    sep = "\t",
    "",
    format(my.pulse$amp, digits = my.opt$options$precision),
    format(my.pulse$amp.err, digits = my.opt$options$precision),
    format(my.pulse$front.int, digits = my.opt$options$precision),
    format(my.pulse$tail.int, digits = my.opt$options$precision)
  )
  
  return(TRUE)
}

my.event.collector <- function(a, e) {
  my.info <- e$eventInfo
  my.trg <- e$triggers

  my.trg.ch <- 0
  my.trg.pos <- 0
  for (col in colnames(my.trg)) {
    if (my.trg[[col]][1]) {
      my.trg.ch <- my.trg.ch + 1
      my.trg.pos <- my.trg[[col]][4]
      break;
    }
    my.trg.ch <- my.trg.ch + 1
  }
  if (my.trg.ch == 0) {
    return(a)
  }

  if (a == 0) {
    my.print.header()
  }
    
  cat(
    file = my.dest, 
    sep = "\t",
    my.info$TimeStamp,
    my.info$Run,
    my.info$EventCounter,
    my.info$DeviceTimeStamp,
    my.trg.ch - 1
  )

  for (ch in my.channel.indices) {
    my.channel.col <- paste("CH", ch - 1, sep = "")
    my.pulse <- NULL
    my.wf <- e$waveforms[[my.channel.col]]
    
    if (!is.null(my.wf)) {
      my.pulse <- get.pulse(
        my.wf,
        front.len = my.channels$FRONT[ch],
        tail.len = my.channels$TAIL[ch],
        n.skip = my.channels$SKIP[ch],
        need.integration = TRUE
      )
    }

    my.pulses[[ch]] <<- my.pulse
  }
  
  my.last.master.run[my.trg.ch] <<- my.info$Run
  my.last.master.eventCounter[my.trg.ch] <<- my.info$EventCounter
  my.last.master.timeStamp[my.trg.ch] <<- my.info$DeviceTimeStamp
  my.last.master.trigger.position[my.trg.ch] <<- my.trg.pos

  for (ch in my.channel.indices) {
    my.res <- my.print.channel(ch, my.info, my.trg.ch, my.trg.pos)
    if (is.null(my.res)) {
      for (t in my.trigger.indices) {
        cat(file = my.dest, sep = "\t", "", NA, NA)
      }
      cat(file = my.dest, sep = "\t", "", NA, NA, NA, NA)
    }
  }

  cat(
    file = my.dest, 
    "\n"
  )
  
  if (a %% 1000 == 0 && my.opt$options$verbose) {
    cat(e$eventInfo$Run, e$eventInfo$EventCounter, "\n")
  }
  
  return(a + 1)
}

my.make.filename <- function(opt) {
  res <- paste(opt$args[1], "/peaks", sep = "")
  
  if (length(my.opt$args) == 2) {
    runname <- strsplit(basename(my.opt$args[2]), "\\.")[[1]][1]
    res <- paste(res, runname, sep = ".")
  }
  
  res <- paste(res, "txt", sep = ".")
  
  return (res)
}

my.make.column.name <- function(channel) {
  if (is.na(channel)) {
    return (NA)
  } else {
    return (paste("CH", channel, sep = ""))
  }
}

my.stop.func <- function(a) {
  return(!is.na(my.opt$options$number) && my.opt$options$number > 0 && !(a < my.opt$options$number))
}

########################################################
# Command line parsing
########################################################

my.option.list <- list( 
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
    c("-v", "--verbose"),
    action="store_true",
    default = FALSE, 
    help = "Verbose mode"
  )
)

my.opt <- parse_args(
  OptionParser(
    option_list = my.option.list,
    usage = "%prog [options] <destination directory> <MID file 1> [<MID file 2> ...]"
  ), 
  positional_arguments = c(2, Inf)
)

my.midas.files <- tail(my.opt$args, n = -1)
#my.channel.col <- my.make.column.name(my.opt$options$channel)
#my.master.trigger.col <- my.make.column.name(my.opt$options$master)
#my.trigger.col <- my.make.column.name(my.opt$options$trigger)

########################################################
# Processing
########################################################

my.channels <- read.table("channels.txt", header = T, sep = "")
my.channel.indices <- which(!is.na(my.channels$ON) & my.channels$ON > 0)
my.trigger.indices <- which(!is.na(my.channels$ON) & my.channels$ON > 0 & !is.na(my.channels$TRG) & my.channels$TRG > 0)
my.pulses <- list(rep(NULL, times = nrow(my.channels)))
my.last.master.run <- rep(NA, times = nrow(my.channels))
my.last.master.eventCounter <- rep(NA, times = nrow(my.channels))
my.last.master.timeStamp <- rep(NA, times = nrow(my.channels))
my.last.master.trigger.position <- rep(NA, times = nrow(my.channels))

my.dest <- file(my.make.filename(my.opt))
open(my.dest, "w")
my.write.header <- TRUE

my.res <- read.events.from.gdconvert(
  file.names = my.midas.files,
  merging.func = my.event.collector,
  stop.func = my.stop.func,
  init.value = 0
)

close(my.dest)
