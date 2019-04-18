#!/usr/bin/Rscript
# Arguments: [<options>] <peaks file> <dest file>

library(gneis.daq)

my.master.channel <- 0  # number of master trigger channel
my.dist.channel <- 5  # number of channel of interest
my.timestamp.module <- 2^31   # module of time stamp values

#####################################################
# Command line parsing
#####################################################

my.cmd.args <- commandArgs(trailingOnly=TRUE)
my.file.peaks <- my.cmd.args[1]
my.file.dest <- my.cmd.args[2]

#####################################################
# File processing
#####################################################

my.peaks <- file(my.file.peaks)
open(my.peaks, "r")

my.dest <- file(my.file.dest)
open(my.dest, "w")

my.last.master.run <- 0
my.last.master.eventCounter <- 0
my.last.master.timeStamp <- 0
my.last.master.peakPosition <- 0

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
      my.last.master.peakPosition <- as.numeric(my.s[6])
    }
  }
  
  if (my.channel == my.dist.channel) {
    my.run <- as.numeric(my.s[1])
    my.eventCounter <- as.numeric(my.s[2])
    my.is.trigger <- as.numeric(my.s[5])
    
    if (my.run == my.last.master.run && my.eventCounter > my.last.master.eventCounter && my.is.trigger > 0) {
      my.timeStamp <- as.numeric(my.s[3])
      my.peakPosition <- as.numeric(my.s[6])
      my.timeStamp.diff <- (my.timeStamp - my.last.master.timeStamp) %% my.timestamp.module
      my.time <- my.peakPosition - my.last.master.peakPosition + my.timeStamp.diff
      
      cat(
        file = my.dest, 
        sep = "\t",
        my.run, 
        my.eventCounter, 
        my.dist.channel,
        my.time, 
        my.s[7], 
        my.s[8], 
        my.s[9], 
        "\n")
    }
  }
  
}

close(my.peaks)
close(my.dest)
