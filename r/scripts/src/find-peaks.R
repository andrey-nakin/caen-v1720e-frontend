#!/usr/bin/Rscript
# Arguments: [<options>] <destination file> <MID file1 [...]> 

library(gneis.daq)

my.cmd.args <- commandArgs(trailingOnly=TRUE)

my.dest <- file(my.cmd.args[1])
open(my.dest, "w")

my.front.lengths <- rep(4, times = 8)
my.tail.lengths <- rep(10, times = 8)

my.print.init.info <- function(e) {
  cat(
    file = my.dest, 
    sep = "\t",
    "# InitialTimeStamp:",
    e$eventInfo$TimeStamp,
    "\n"
  )
  cat(
    file = my.dest, 
    sep = "\t",
    "# Device:",
    e$eventInfo$Device,
    "\n"
  )
  cat(
    file = my.dest, 
    sep = "\t",
    "# FrontendIndex:",
    e$eventInfo$FrontendIndex,
    "\n"
  )
  cat(
    file = my.dest, 
    sep = "\t",
    "# DeviceTimeStampModule:",
    e$eventInfo$DeviceTimeStampModule,
    "\n"
  )
  cat(
    file = my.dest, 
    sep = "\t",
    "# TicksPerSample:",
    e$eventInfo$TicksPerSample,
    "\n"
  )
  cat(
    file = my.dest, 
    sep = "\t",
    "# SamplesPerNuSecond:",
    e$eventInfo$SamplesPerNuSecond,
    "\n"
  )
  cat(
    file = my.dest, 
    sep = "\t",
    "# BitsPerSample:",
    e$eventInfo$BitsPerSample,
    "\n"
  )

  cat(
    file = my.dest, 
    sep = "\t",
    "RUN",
    "EC",
    "TS",
    "CH",
    "TRG",
    "PP",
    "PA",
    "FI",
    "TI",
    "\n"
  )
}

my.print.channel <- function(my.info, ch, wf, trg) {
  my.pos <- which.min(wf)
  my.front.pos <- my.pos - my.front.lengths[ch]
  my.tail.pos <- my.pos + my.tail.lengths[ch] + 1
  my.sum <- 0
  my.n <- 0
  my.front.sum <- 0
  my.tail.sum <- 0
  my.len <- length(wf)
  
  if (my.front.pos > 0) {
    my.sum <- sum(wf[1 : my.front.pos])
    my.n <- my.front.pos
    
    my.front.sum <- sum(wf[(my.front.pos + 1) : my.pos])
  }
  
  if (my.tail.pos <= my.len) {
    my.sum <- my.sum + sum(wf[my.tail.pos : my.len])
    my.n <- my.n + my.len - my.tail.pos + 1
    
    my.tail.sum <- sum(wf[(my.pos + 1) : (my.tail.pos - 1)])
  }
  
  my.mean = my.sum / my.n
  my.amp = round(abs(my.mean - wf[my.pos]))
  my.front.sum <- round(abs(my.front.sum - my.mean * my.front.lengths[ch]))
  my.tail.sum <- round(abs(my.tail.sum - my.mean * my.tail.lengths[ch]))
  
  cat(
    file = my.dest, 
    sep = "\t",
    my.info$Run,
    my.info$EventCounter,
    my.info$DeviceTimeStamp,
    ch - 1,
    trg,
    my.pos - 1,
    my.amp,
    my.front.sum,
    my.tail.sum,
    "\n"
  )
}

my.event.collector <- function(a, e) {
  if (a == 0) {
    my.print.init.info(e)
  }
  
  my.info <- e$eventInfo
  my.wfs <- e$waveforms
  my.trg <- e$triggers
  my.ch <- 1
  
  for (col in colnames(my.wfs)) {
    my.print.channel(e$eventInfo, my.ch, my.wfs[[col]], my.trg[[col]][1])
    my.ch <- my.ch + 1
  }

  return(a + 1)  
}

my.res <- read.events.from.gdconvert(
  file.names = tail(my.cmd.args, n = -1),
  merging.func = my.event.collector,
  init.value = 0
)

close(my.dest)
