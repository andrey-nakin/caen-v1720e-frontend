#!/usr/bin/Rscript
# Arguments: <master trigger> <trigger> <destination file> <MID file1 [...]> 

library(gneis.daq)

########################################################
# Global variables
########################################################

my.front.lengths <- rep(4, times = 8)
my.tail.lengths <- rep(10, times = 8)

my.last.master.run <- 0
my.last.master.eventCounter <- 0
my.last.master.timeStamp <- 0
my.last.master.trigger.position <- 0
my.last.master.peak.position <- 0

########################################################
# Functions
########################################################

my.print.init.info <- function(e) {
  cat(
    file = my.dest, 
    sep = "\t",
    "RUN",
    "EC",
    "TS",
    "CH",
    "TRG",
    "TRGP",
    "PP",
    "PA",
    "FI",
    "TI",
    "\n"
  )
}

my.print.channel <- function(my.info, ch, wf, trg, trg.pos) {
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
    trg.pos,
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
  
  if (my.trg[[my.master.trigger.col]][1] > 0) {
    my.last.master.run <<- my.info$Run
    my.last.master.eventCounter <<- my.info$EventCounter
    my.last.master.timeStamp <<- my.info$DeviceTimeStamp
    my.last.master.trigger.position <<-my.trg[[my.master.trigger.col]][4]
    # TODO my.last.master.peak.position <- as.numeric(my.s[7])
  }
  
  for (col in colnames(my.wfs)) {
    my.print.channel(e$eventInfo, my.ch, my.wfs[[col]], my.trg[[col]][1], my.trg[[col]][4])
    my.ch <- my.ch + 1
  }
  
  return(a + 1)  
}

########################################################
# Command line parsing
########################################################

my.cmd.args <- commandArgs(trailingOnly=TRUE)
my.master.trigger.channel <- as.numeric(my.cmd.args[1])
my.master.trigger.col <- paste("CH", my.master.trigger.channel)
my.trigger.channel<- as.numeric(my.cmd.args[2])
my.trigger.col <- paste("CH", my.trigger.channel)
my.filename.dest <- as.numeric(my.cmd.args[3])
my.filename.src <- tail(my.cmd.args, n = -3)

########################################################
# Processing
########################################################

my.dest <- file(my.filename.dest)
open(my.dest, "w")

my.res <- read.events.from.gdconvert(
  file.names = my.filename.src,
  merging.func = my.event.collector,
  init.value = 0
)

close(my.dest)
