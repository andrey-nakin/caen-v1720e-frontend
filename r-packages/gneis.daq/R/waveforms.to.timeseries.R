waveforms.to.timeseries <- 
  function(eventInfo, waveforms, dcOffsets, time.units = "nus", voltage.units = "mV") {
    s <- tolower(time.units)
    if (s == "s") {
      time.coef <- 1.0
    } else if (s == "ms") {
      time.coef <- 1.0e3;
    } else if (s == "nus") {
      time.coef <- 1.0e6;
    } else if (s == "ns") {
      time.coef <- 1.0e9;
    } else if (s == "ps") {
      time.coef <- 1.0e12;
    } else {
      stop(paste("Unknown time units:", time.units, sep = " "))
    }

    s <- tolower(voltage.units)
    if (s == "v") {
      v.coef <- 1.0
    } else if (s == "mv") {
      v.coef <- 1.0e3;
    } else if (s == "nuv") {
      v.coef <- 1.0e6;
    } else {
      stop(paste("Unknown voltage units:", voltage.units, sep = " "))
    }
    
    result <- data.frame(
      TIME = seq(from = 1, to = nrow(waveforms), by = 1) * 1.0e-6 * time.coef / eventInfo$SamplesPerNuSecond
    )
    for (col in colnames(waveforms)) {
      result[[col]] <- waveforms[[col]] * eventInfo$DcMultiplier + dcOffsets[[col]][2]
    }
    return (result)
  }
  