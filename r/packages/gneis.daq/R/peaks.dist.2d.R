peaks.dist.2d <- function(
  xColumn, yColumn, srcDir, destFn, xBreaks, yBreaks, xAbsolute = FALSE, yAbsolute = FALSE,
  nevents = NA, filter = NULL, verbose = FALSE
) {

  my.make.src.filename.mask <- function() {
    res <- paste("peaks", "run[0-9]+", "txt", sep = ".")
    return (res)
  }
  
  my.process.file <- function(fn, accum) {
    my.df <- read.table(fn, header = T, sep = "")
    my.x <- my.df[[xColumn]]
    my.y <- my.df[[yColumn]]
    
    if (!is.null(filter)) {
      my.idx <- filter(my.df)
      if (!is.null(my.idx)) {
        my.x <- my.x[my.idx]
        my.y <- my.y[my.idx]
      }
    }
    
    if (xAbsolute) {
      my.x <- abs(my.x)
    }
    if (yAbsolute) {
      my.y <- abs(my.y)
    }
    
    my.min.x <- xBreaks[1]
    my.max.x <- xBreaks[length(xBreaks)]
    my.step.x <- xBreaks[2] - xBreaks[1]
    my.min.y <- yBreaks[1]
    my.max.y <- yBreaks[length(yBreaks)]
    
    my.idx <- which(my.x >= my.min.x & 
                      my.x < my.max.x & 
                      my.y >= my.min.y & 
                      my.y < my.max.y)
    my.x <- my.x[my.idx]
    my.y <- my.y[my.idx]
    
    my.m <- apply(my.x.breaks, 1, function(limits) {
      my.h.y <- hist(
        my.y[which(my.x >= limits[1] & my.x < limits[2])], 
        breaks = yBreaks, 
        plot = F, 
        right = F
      )
      return(my.h.y$counts) 
    })

    if (is.null(accum)) {
      return(my.m)
    } else {
      return(accum + my.m)
    }
  }
  
  my.write.result <- function(my.df) {
    write.table(
      my.accum, 
      file = destFn,
      sep = "\t",
      row.names = F,
      col.names = F
    )
  }
  
  ######################################################
  # Entry point
  ######################################################
  
  if (verbose) {
    cat("Processing directory", srcDir, "\n")
  }

  my.x.breaks <- array(dim = c(length(xBreaks) - 1, 2))
  my.x.breaks[,1] <- head(xBreaks, n = -1)
  my.x.breaks[,2] <- tail(xBreaks, n = -1)
  
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
  
  my.write.result(my.accum)
  
  if (verbose) {
    cat("Finished processing", "\n")
  }
  
}
