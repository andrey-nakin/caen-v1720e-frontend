# Read one or several MID files, parse and return the events
read.events.from.file <- 
  function(con, handler) {
    state <- 0  # idle
    
    cat("****************************************\n")
    i <- 0
    while (length(line <- readLines(f,n=1)) > 0) {
      i <- i + 1
      if (state == 0) {
        if (line == "EventInfo") {
          cat("switch to state 1\n")
          state <- 1  # readig event info
          eventInfo <- list()
        } else if (line == "Waveforms") {
          waveforms <- read.csv(con, header = T, sep = "\t", nrows = eventInfo$RecordLength)
        } else if (line == ".") {
          res <- handler(list(
            eventInfo = eventInfo,
            waveforms = waveforms
          ))
          if (!is.null(res) && res == FALSE) {
            break
          }
        }
      } else if (state == 1) {
        if (line == "") {
          cat("switch to state 0\n")
          state <- 0
        } else {
          s <- strsplit(line, "\\s")[[1]]
          name <- s[1]
          if (name == "Device") {
            value <- s[2]
          } else {
            value <- as.numeric(s[2])
          }
          eventInfo[[name]] <- value
        }
      }
    }    
  }

