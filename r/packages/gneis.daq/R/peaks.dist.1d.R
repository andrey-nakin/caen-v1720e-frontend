peaks.dist.1d <- function(
  column, srcDir, destFn, breaks, absolute = FALSE,
  nevents = NA, filter = NULL, verbose = FALSE
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

    if (absolute) {
      my.y <- abs(my.y)
    }
    
    my.min <- breaks[1]
    my.max <- breaks[length(breaks)]
    my.y <- my.y[which(my.y >= my.min & my.y < my.max)]
    
    my.h.y <- hist(my.y, breaks = breaks, plot = F, right = F)

    if (is.null(accum)) {
      return(my.h.y$counts)
    } else {
      return(accum + my.h.y$counts)
    }
  }

  my.write.result <- function(my.df) {
    write.table(
      my.df,
      file = destFn,
      sep = "\t",
      row.names = F,
      col.names = T
    )
  }
  
  ######################################################
  # Entry point
  ######################################################

  if (verbose) {
    cat("Processing directory", srcDir, "\n")
  }
  
  my.accum <- NULL
  my.files <- list.files(path = srcDir, pattern = my.make.src.filename.mask(), full.names = T)
  for (my.fn in my.files) {
    if (verbose) {
      cat("Processing file", my.fn, "\n")
    }
    
    my.accum <- my.process.file(my.fn, my.accum)

    if (verbose) {
      if (is.null(my.accum)) {
        cat("No events in statistics", "\n")
      } else {
        cat("Number of events in statistics:", sum(my.accum), "\n")
      }
    }
    
    if (!is.na(nevents) && sum(my.accum) >= nevents) {
      break
    }
  }

  if (is.null(my.accum)) {
    stop("No data")
  }
  
  my.df <- data.frame(
    VALUE = tail(breaks, n = -1),
    COUNT = my.accum
  )
  colnames(my.df) <- c(column, "COUNT")
  
  my.write.result(my.df)

  if (verbose) {
    cat("Finished processing", "\n")
  }
  
}
