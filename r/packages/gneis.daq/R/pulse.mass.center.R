pulse.mass.center <- function(wf, front.len, tail.len, n.skip = 0, is.positive = FALSE) {
  if (is.positive) {
    my.pos <- which.max(wf)
  } else {
    my.pos <- which.min(wf)
  }
  
  my.front.pos <- my.pos - front.len
  my.tail.pos <- my.pos + tail.len
  my.skip.pos <- my.tail.pos + n.skip + 1
  my.len <- length(wf)
  
  if (my.front.pos < 1 || my.tail.pos > my.len) {
    return(NULL)
  }
  
  my.s <- 0
  my.n <- 0
  
  if (my.front.pos > 1) {
    my.n <- my.front.pos - 1
    my.s <- sum(wf[1 : my.n])
  }
  
  if (!(my.skip.pos > my.len)) {
    my.s <- my.s + sum(wf[my.skip.pos : my.len])
    my.n <- my.n + (my.len - my.skip.pos + 1)
  }
  
  if (my.n < 1) {
    return(NULL)
  }
  
  my.zero <- my.s / my.n
  my.pulse <- wf[my.front.pos : my.tail.pos] - my.zero
  my.positions <- (my.pos - front.len) : (my.pos + tail.len)
  my.mass.center <- sum(my.positions * my.pulse / sum(my.pulse))

  return(
    list(
      x = my.mass.center,
      error = 0.5
    )
  )
}
