# Read one or several MID files, parse and return the events
read.events.from.pipe <- 
  function(files, handler, executable = "gd-convert") {
    cmd <- paste(executable, "-fsimple", files, sep = " ")
    cat("cmd=", cmd, "\n")
    f <- pipe(cmd)
    cat("f=", f, "\n")
    line <- readLines(f,n=1)
    cat("line=", line, "\n")
    close(f)
  }

