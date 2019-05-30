get.pulse <- function(wf, front.len, tail.len, n.skip = 0, is.positive = FALSE, need.integration = FALSE, need.pulse = FALSE) {
  
  if (is.positive) {
    my.pos <- which.max(wf)
  } else {
    my.pos <- which.min(wf)
  }

  if (n.skip < 0) {
    n.skip <- length(wf)
  }
    
  my.front.pos <- my.pos - front.len
  my.tail.pos <- my.pos + tail.len
  my.skip.pos <- my.tail.pos + n.skip + 1
  my.len <- length(wf)
  
  if (my.front.pos < 1 || my.tail.pos > my.len) {
    return(NULL)
  }
  
  my.s <- 0
  my.s2 <- 0
  my.n <- 0
  
  if (my.front.pos > 1) {
    my.n <- my.front.pos - 1
    my.s <- sum(wf[1 : my.n])
    my.s2 <- sum(wf[1 : my.n]^2)
  }
  
  if (!(my.skip.pos > my.len)) {
    my.s <- my.s + sum(wf[my.skip.pos : my.len])
    my.s2 <- my.s2 + sum(wf[my.skip.pos : my.len]^2)
    my.n <- my.n + (my.len - my.skip.pos + 1)
  }
  
  if (my.n < 1) {
    return(NULL)
  }
  
  my.zero <- my.s / my.n
  my.var <- (my.n * my.s2 - my.s^2) / (my.n * (my.n - 1))
  my.std <- sqrt(my.var)
  
  my.pulse <- wf[my.front.pos : my.tail.pos] - my.zero
  my.pulse.abs <- abs(my.pulse)
  my.positions <- (my.pos - front.len) : (my.pos + tail.len)
  my.mass.center <- sum(my.positions * (my.pulse.abs / sum(my.pulse.abs)))

  if (need.integration) {
    my.front.int <- sum(my.pulse[1 : (front.len + 1)])
    my.tail.int <- sum(my.pulse) - my.front.int
  } else {
    my.front.int <- NA
    my.tail.int <- NA
  }

  if (!need.pulse) {
    my.pulse = NA
    my.positions = NA
  }
    
  return(
    list(
      x = my.mass.center,
      x.err = 0.0,
      amp = wf[my.pos] - my.zero,
      amp.err = 0.5 + my.std / sqrt(my.n),
      front.int = my.front.int,
      tail.int = my.tail.int,
      pulse = my.pulse,
      positions = my.positions
    )
  )
  
}
