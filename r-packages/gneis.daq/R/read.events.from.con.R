# Read one or several MID files, parse and return the events
read.events.from.con <- 
  function(con, filter.func = NULL, converter.func = NULL, merging.func = NULL, init.value = NULL, nevents = -1, time.units = "nus", voltage.units = "mV") {
    state <- 0  # idle
    my.ecount <- 0
    if (missing(merging.func) || is.null(merging.func)) {
      my.result <- list()
    } else {
      my.result <- init.value
    }
    
    while (length(line <- readLines(con, n=1)) > 0) {
      if (state == 0) {
        if (line == "#EventInfo") {
          state <- 1  # readig event info
          eventInfo <- list()
        } else if (line == "#Waveforms") {
          waveforms <- read.csv(con, header = T, sep = "\t", nrows = eventInfo$WaveformLength)
        } else if (line == "#DcOffsets") {
          dcOffsets <- read.csv(con, header = T, sep = "\t", nrows = 2)
        } else if (line == "#Triggers") {
          triggers <- read.csv(con, header = T, sep = "\t", nrows = 4)
        } else if (line == "#EndOfEvent") {
          my.event <- list(
            eventInfo = eventInfo,
            waveforms = waveforms,
            timeseries = waveforms.to.timeseries(
              eventInfo, waveforms, dcOffsets, time.units = time.units, voltage.units = voltage.units
            ),
            dcOffsets = dcOffsets,
            triggers = triggers
          )
          
          if (missing(filter.func) || is.null(filter.func) || filter.func(my.event) == TRUE) {
            my.ecount <- my.ecount + 1
            
            if (!missing(converter.func) && !is.null(converter.func)) {
              my.event <- converter.func(my.event)
            }
            
            if (missing(merging.func) || is.null(merging.func)) {
              my.result[[my.ecount]] <- my.event
            } else {
              my.result <- merging.func(my.result, my.event)
            }
            
            if (nevents >= 0 && nevents <= my.ecount) {
              break
            }
          }
        }
      } else if (state == 1) {
        if (line == "") {
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
    
    return(my.result)
  }

