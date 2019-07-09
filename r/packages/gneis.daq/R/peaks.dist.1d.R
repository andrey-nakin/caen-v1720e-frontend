peaks.dist.1d <- function(
  column, srcDir, destFn, breaks, 
  nevents = NA, filter = NULL, file.suffix = NULL, file.comment = NULL
) {

  my.make.src.filename.mask <- function() {
    res <- paste("peaks", "run[0-9]+", "txt", sep = ".")
    return (res)
  }

  my.process.file <- function(fn, accum) {
    my.df <- read.table(fn, header = T, sep = "")
    my.y <- my.df[[column]]

    if (!is.null(filter)) {
      my.idx <- filter(my.df)
      if (!is.null(my.idx)) {
        my.y <- my.y[my.idx]
      }
    }

    my.min <- breaks[1]
    my.max <- breaks[length(breaks)]
    my.y <- my.y[which(my.y >= my.min & my.y < my.max)]
    
    my.h.y <- hist(my.y, breaks = breaks, plot = F)

    if (is.null(accum)) {
      return(my.h.y$counts)
    } else {
      return(accum + my.h.y$counts)
    }
  }

  my.write.result <- function() {
    my.file.conn <- file(destFn)
    if (!is.null(file.comment)) {
      writeLines(paste("#", file.comment), my.file.conn)
    }
    close(my.file.conn)
    
    write.table(
      my.df,
      file = destFn,
      append = T,
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
    if (!is.na(nevents) && sum(my.accum) >= nevents) {
      break
    }
  }

  my.df <- data.frame(
    TIME = tail(time.breaks, n = -1),
    COUNT = my.accum
  )
  
  my.write.result()

}
