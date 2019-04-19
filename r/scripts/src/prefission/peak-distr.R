#!/usr/bin/Rscript
# Arguments: [<options>] <peaks file> <dest file> <channel>

library(gneis.daq)

my.master.channel <- 0  # number of master trigger channel
my.dist.channel <- 0  # number of channel of interest
my.timestamp.module <- 2^31   # module of time stamp values

#####################################################
# Command line parsing
#####################################################

my.cmd.args <- commandArgs(trailingOnly=TRUE)
my.file.peaks <- my.cmd.args[1]
my.file.dest <- my.cmd.args[2]
if (length(my.cmd.args) >= 3) {
  my.dist.channel <- as.numeric(my.cmd.args[3])
}

#####################################################
# File processing
#####################################################

my.peaks <- file(my.file.peaks)
open(my.peaks, "r")

my.dest <- file(my.file.dest)
open(my.dest, "w")

cat(
  file = my.dest, 
  sep = "\t",
  "RUN",
  "EC",
  "CH",
  "TRG",
  "PP",
  "PA",
  "FI",
  "TI",
  "\n"
)

my.last.master.run <- 0
my.last.master.eventCounter <- 0
my.last.master.timeStamp <- 0
my.last.master.trigger.position <- 0
my.last.master.peak.position <- 0

while (length(line <- readLines(my.peaks, n=1)) > 0) {
  my.s <- strsplit(line, split = "\t")[[1]]
  
  my.channel <- as.numeric(my.s[4])
  if (is.na(my.channel)) {
    next
  }

  if (my.channel == my.master.channel) {
    my.is.trigger <- as.numeric(my.s[5])
    if (my.is.trigger > 0) {
      my.last.master.run <- as.numeric(my.s[1])
      my.last.master.eventCounter <- as.numeric(my.s[2])
      my.last.master.timeStamp <- as.numeric(my.s[3])
      my.last.master.trigger.position <-as.numeric(my.s[6]) 
      my.last.master.peak.position <- as.numeric(my.s[7])
    }
  }
  
  if (my.channel == my.dist.channel) {
    my.run <- as.numeric(my.s[1])
    my.eventCounter <- as.numeric(my.s[2])
    my.is.trigger <- as.numeric(my.s[5])
    
    if (my.run == my.last.master.run && my.eventCounter >= my.last.master.eventCounter && my.is.trigger > 0) {
      my.timeStamp <- as.numeric(my.s[3])
      my.trigger.position <-as.numeric(my.s[6])
      my.peak.position <- as.numeric(my.s[7])
      my.timeStamp.diff <- (my.timeStamp - my.last.master.timeStamp) %% my.timestamp.module
      my.time <- my.peak.position - my.trigger.position - 
        (my.last.master.peak.position - my.last.master.trigger.position) + 
        my.timeStamp.diff
      
      cat(
        file = my.dest, 
        sep = "\t",
        my.run, 
        my.eventCounter, 
        my.dist.channel,
        my.is.trigger,
        my.time, 
        my.s[8], 
        my.s[9], 
        my.s[10], 
        "\n")
    }
  }
  
}

close(my.peaks)
close(my.dest)
