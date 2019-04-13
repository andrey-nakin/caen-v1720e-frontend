#!/usr/bin/Rscript
# Arguments: <path-to-package> <MID file wildcard> <destination file> [<chunk size>]

my.cmd.args <- commandArgs(trailingOnly=TRUE)

install.packages(paste(my.cmd.args[1], "gneis.daq.tar.gz", sep = "/"))
library(gneis.daq)

if (length(my.cmd.args) < 4) {
  CHUNK <- 60 * 15
} else {
  CHUNK <- as.numeric(my.cmd.args[4])
}

my.dest <- file(my.cmd.args[3])
open(my.dest, "w")
cat(file = my.dest, "TS0\tTS1\tNE\tNS\tCH0\tSD0\tTRG0\tCH1\tSD1\tTRG1\tCH2\tSD2\tTRG2\tCH3\tSD3\tTRG3\tCH4\tSD4\tTRG4\tCH5\tSD5\tTRG5\tCH6\tSD6\tTRG6\tCH7\tSD7\tTRG7", "\n")

my.process <- function(nevents, filter.func = NULL, converter.func = NULL, merging.func = NULL, init.value = NULL) {
  res <- read.events.from.gdconvert(
    file.names = my.cmd.args[2],
    filter.func = filter.func, 
    converter.func = converter.func, 
    merging.func = merging.func,
    init.value = init.value,
    need.timeseries = TRUE,
    voltage.units = "v"
  );
  return(res)
}

my.print <- function(accum) {
  cat(file = my.dest, accum[[1]]$ts0, accum[[1]]$ts1 + 1, accum[[1]]$ne, accum[[1]]$n, sep = "\t")
  for (i in seq(1, length(accum))) {
    l <- accum[[i]]
    my.mean <- l$s / l$n
    my.sd <- sqrt((l$n * l$s2 - l$s * l$s) / (l$n * (l$n - 1)))
    cat(file = my.dest, "\t")
    cat(file = my.dest, my.mean, my.sd, l$nt, sep = "\t")
  }
  cat(file = my.dest, "\n")
}

my.res <- my.process(
  merging.func = function(accum, event) {
    my.s <- lapply(tail(colnames(event$timeseries), n = -1), function(col) {
      return(list(
        ts0 = event$eventInfo$TimeStamp,
        ts1 = event$eventInfo$TimeStamp,
        s = sum(event$timeseries[[col]]), 
        s2 = sum(event$timeseries[[col]]^2), 
        n = nrow(event$timeseries),
        ne = 1,
        nt = event$triggers[[col]][1]
      ))
    })

    if (is.null(accum)) {
      return(my.s)
    }

    if (floor(accum[[1]]$ts1 / CHUNK) != floor(event$eventInfo$TimeStamp / CHUNK)) {
      my.print(accum)
      return(my.s)
    }

    my.res <- list()
    for (i in seq(1, length(my.s))) {
      my.res[[i]] <- list()
      my.res[[i]]$ts0 <- accum[[i]]$ts0
      my.res[[i]]$ts1 <- my.s[[i]]$ts1
      my.res[[i]]$s <- accum[[i]]$s + my.s[[i]]$s
      my.res[[i]]$s2 <- accum[[i]]$s2 + my.s[[i]]$s2
      my.res[[i]]$n <- accum[[i]]$n + my.s[[i]]$n
      my.res[[i]]$ne <- accum[[i]]$ne + my.s[[i]]$ne
      my.res[[i]]$nt <- accum[[i]]$nt + my.s[[i]]$nt
    }

    return(my.res)
  }
)

my.print(my.res)

close(my.dest)
