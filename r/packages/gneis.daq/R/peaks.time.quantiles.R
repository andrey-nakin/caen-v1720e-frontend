peaks.time.quantiles <- function(
  channel, master, srcDir, destDir, trigger = NA,
  nevents = NA, filter = NULL, file.suffix = NULL
) {
  
  my.make.src.filename.mask <- function() {
    res <- paste("peaks", "run[0-9]+", "txt", sep = ".")
    return (res)
  }
  
  my.process.file <- function(fn, accum) {
    my.df <- read.table(fn, header = T, sep = "")
    
    my.y <- my.df[[paste("CH", channel, "_PP_M", master, sep = "")]]
    my.trg <- my.df$TRG
    
    if (!is.null(filter)) {
      my.idx <- filter(my.df)
      if (!is.null(my.idx)) {
        my.y <- my.y[my.idx]
        my.trg <- my.trg[my.idx]
      }
    }
    
    if (!is.na(trigger)) {
      my.idx <- which(my.trg == trigger)
      my.y <- my.y[my.idx]
    }
    
    my.y <- my.y[which(!is.na(my.y))]

    if (is.null(accum)) {
      return(my.y)
    }
    
    return(append(accum, my.y))
  }
  
  my.make.dest.filename <- function(dir, ext) {
    res <- paste(dir, "/peaks.ch", channel, sep = "")
    
    res <- paste(res, ".mst", master, sep = "")
    
    if (!is.na(trigger)) {
      res <- paste(res, ".trg", trigger, sep = "")
    }
    
    if (!is.null(file.suffix)) {
      res <- paste(res, file.suffix, sep = ".")
    }
    
    res <- paste(res, "quantile", ext, sep = ".")
    
    return (res)
  }
  
  my.make.result.filename <- function() {
    return(
      my.make.dest.filename(destDir, "txt")
    )
  }
  
  my.write.result <- function(df) {
    write.table(
      df,
      file = my.make.result.filename(),
      sep = "\t",
      row.names = F,
      col.names = T
    )
  }
  
  ######################################################
  # Entry point
  ######################################################
  
  my.accum <- NULL
  my.files <- list.files(path = srcDir, pattern = my.make.src.filename.mask(), full.names = T)
  for (my.fn in my.files) {
    my.accum <- my.process.file(my.fn, my.accum)
    if (!is.na(nevents) && length(my.accum) >= nevents) {
      break
    }
  }

  my.a <- c(0.001 / 4, 0.001 / 2, 0.01 / 4, 0.01 / 2, 0.1 / 4, 0.1 / 2, 1 / 10, 1 / 8, 1 / 4)
  my.v <- append(my.a, append(0.5, rev(1 - my.a)))
  my.q <- quantile(my.accum, my.v)
  my.write.result(data.frame(
    QUANTILE = my.v,
    VALUE = my.q
  ))
  
}
