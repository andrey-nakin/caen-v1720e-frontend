read.events.from.pipe <- 
  function(
    file.names, 
    executable="gd-convert", 
    filter.func = NULL, 
    converter.func = NULL, 
    merging.func = NULL, 
    init.value = NULL, 
    nevents = -1, 
    time.units = "nus", 
    voltage.units = "mV",
    need.timeseries = F
  ) {
    my.cmd <- paste(executable, "-fsimple", "-Fstdout", paste(file.names, collapse = " "), sep = " ")
    my.con <- pipe(my.cmd)
    open(my.con)
    my.result <- read.events.from.con(
      my.con, 
      filter.func = filter.func, 
      converter.func = converter.func, 
      merging.func = merging.func, 
      init.value = init.value, 
      nevents = nevents, 
      time.units = time.units, 
      voltage.units = voltage.units,
      need.timeseries = need.timeseries
    )
    close(my.con)
  }
