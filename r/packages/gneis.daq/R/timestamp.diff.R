timestamp.diff <- function(a, b, width = 31) {
  m <- 2^width
  if (!(a < m)) {
    a <- a - m
  }
  if (!(b < m)) {
    b <- b - m
  }
  res <- a - b
  if (res < 0) {
    res <- res + m
  }
  return(res)
}
