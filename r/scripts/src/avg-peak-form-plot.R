#!/usr/bin/Rscript

library(optparse)
library(ggplot2)
library(stringr)

########################################################
# Functions
########################################################

my.plot.title <- function(main) {
  my.res <- paste(main, paste("Channel #", my.opt$options$channel, sep = ""), sep = ", ")
  if (!is.na(my.opt$options$trigger)) {
    my.res <- paste(my.res, paste("Trigger #", my.opt$options$trigger, sep = ""), sep = ", ")
  }
  my.res <- paste(my.res, paste("Amplitudes", paste(my.opt$options$minamp, my.opt$options$maxamp, sep = "-"), sep = ": "), sep = ", ")
  return(my.res)
}

my.plot <- function(my.df) {
  my.info <- my.analyze(my.df)
  
  my.sum = my.df$MY * my.df$N
  my.sum2 = my.df$MY2 * my.df$N
  my.var = (my.df$N * my.sum2 - my.sum^2) / (my.df$N * (my.df$N - 1))
  my.std = sqrt(my.var)

  my.df <- data.frame(
    x = my.df$MX,
    y = my.df$MY,
    y.err = my.std / sqrt(my.df$N)
  )

  my.p <- ggplot(data = my.df, aes(x = x, y = y)) +
    ggtitle(
      label = my.plot.title("Averaged Pulse"),
      subtitle = paste(
        paste("Peak Pos", format(my.info$peak.pos, digits = 2), sep = ": "),
        paste("Peak Amp", format(my.info$peak.amp, digits = 2), sep = ": "),
        paste(sprintf("%d/%d Front Length", my.low.level * 100, my.high.level * 100) , format(my.info$front.len, digits = 2), sep = ": "),
        sep = ", "
      )
    ) +
    scale_x_continuous(name = "Time, channels") +
    scale_y_continuous(name = "Amplitude, channels") +
    theme_light() +
    geom_line() +
    geom_line(aes(x = x, y = rep(my.info$low.level, length(x))), size = 0.25, color = 'magenta') +
    geom_line(aes(x = x, y = rep(my.info$high.level, length(x))), size = 0.25, color = 'magenta') +
    warnings()
  
  if (max(my.df$y.err) > abs(my.info$peak.amp) / 500) {
    my.p = my.p + geom_errorbar(aes(ymin=y - y.err, ymax = y + y.err), width = .3, position = position_dodge(0.05))
  }
  
  plot(my.p)  
}

my.plot.diff <- function(my.df) {

  my.dx = diff(my.df$MX)
  my.dy = diff(my.df$MY)
  my.f <- rep(1/10, 10)

  my.df <- data.frame(
    x = tail(my.df$MX, n = -1),
    y = filter(my.dy / my.dx, my.f, method = "convolution")
  )
  
  my.p <- ggplot(data = my.df, aes(x = x, y = y)) +
    ggtitle(
      label = my.plot.title("Averaged Pulse Diff")
      # subtitle = paste(
      #   paste("Peak Pos", format(my.info$peak.pos, digits = 2), sep = ": "),
      #   paste("Peak Amp", format(my.info$peak.amp, digits = 2), sep = ": "),
      #   paste(sprintf("%d/%d Front Length", my.low.level * 100, my.high.level * 100) , format(my.info$front.len, digits = 2), sep = ": "),
      #   sep = ", "
      # )
    ) +
    scale_x_continuous(name = "Time, channels") +
    scale_y_continuous(name = "Amplitude Diff") +
    theme_light() +
    geom_line()

  plot(my.p)  
}

my.make.txt.filename.mask <- function(opt) {
  res <- paste("avg-peak-form.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }
  
  res <- paste(res, sprintf("amp%d-%d", opt$options$minamp, opt$options$maxamp), sep = ".")
  
  res <- paste(res, "*", "txt", sep = ".")
  
  return (res)
}

my.make.plot.filename <- function(opt) {
  res <- paste(opt$args[2], "/avg-peak-form.ch", opt$options$channel, sep = "")
  
  if (!is.na(opt$options$trigger)) {
    res <- paste(res, ".trg", opt$options$trigger, sep = "")
  }
  
  res <- paste(res, sprintf("amp%d-%d", opt$options$minamp, opt$options$maxamp), sep = ".")
  
  res <- paste(res, "pdf", sep = ".")
  
  return (res)
}

my.analyze <- function(df) {

  my.pos <- which.min(df$MY)

  my.func <- approxfun(df$MX, df$MY)
  my.level1 <- my.low.level * (df$MY[my.pos] - df$MY[1]) + df$MY[1]
  my.level2 <- my.high.level * (df$MY[my.pos] - df$MY[1]) + df$MY[1]
  if (my.level2 < my.level1) {
    my.root1 <- uniroot(f = function(x) my.func(x) - my.level1, interval = c(df$MX[1], df$MX[my.pos]))$root
    my.root2 <- uniroot(f = function(x) my.func(x) - my.level2, interval = c(df$MX[1], df$MX[my.pos]))$root
  } else {
    my.root1 <- 0
    my.root2 <- 0
  }
  
  return(
    list(
      peak.pos = df$MX[my.pos],
      peak.amp = df$MY[my.pos],
      front.len = my.root2 - my.root1,
      low.level = my.level1,
      high.level = my.level2
    )
  )
    
}

my.extract.run.no <- function(fn) {
  return(as.numeric(sub(".*\\.run([0-9]+)\\..*", "\\1", basename(fn), perl=TRUE)))
}

my.process.file <- function(fn, accum) {
  
  my.df <- read.table(fn, header = T, sep = "\t")
  my.info <- my.analyze(my.df)
  
  my.stat.run <<- append(my.stat.run, my.extract.run.no(fn))
  my.stat.peak.pos <<- append(my.stat.peak.pos, my.info$peak.pos)
  my.stat.peak.amp <<- append(my.stat.peak.amp, my.info$peak.amp)
  my.stat.front.len <<- append(my.stat.front.len, my.info$front.len)

  if (is.null(accum)) {
    return(my.df)
  }
  
  accum$N <- accum$N + my.df$N
  accum$MX <- (accum$MX * accum$N + my.df$MX * my.df$N) / (accum$N + my.df$N)
  accum$MX2 <- (accum$MX2 * accum$N + my.df$MX2 * my.df$N) / (accum$N + my.df$N)
  accum$MY <- (accum$MY * accum$N + my.df$MY * my.df$N) / (accum$N + my.df$N)
  accum$MY2 <- (accum$MY2 * accum$N + my.df$MY2 * my.df$N) / (accum$N + my.df$N)
  
  return(accum)
  
}

my.plot.stat <- function(y, main, ylab, units) {
  my.data <- data.frame(x = my.stat.run, y = y)
  my.lm <- lm(y ~ x, data = my.data)
  my.coef <- coefficients(my.lm)
  my.slope <- my.coef[2]
  my.slope.ci <- confint(my.lm, 'x', level = 0.95)
  my.slope.err = max(abs(my.slope - my.slope.ci[1]), abs(my.slope - my.slope.ci[2]))
  
  p <- ggplot(
    data = my.data, 
    aes(x = x, y = y)
  ) + 
    geom_line() +
    ggtitle(
      label = paste(main, ", Channel #", my.opt$options$channel, sep = ""),
      subtitle = paste("Approximation Line Slope: ", sprintf("%0.2g +/- %0.1g", my.slope, my.slope.err), " ", units, "/run", sep = "")
    ) +
    scale_x_continuous(name = "Run #") + 
    scale_y_continuous(name = paste(ylab, ", ", units, sep = "")) +
    theme_light() +
    theme(axis.text.x = element_text(angle = 45, hjust = 1)) +
    geom_abline(intercept = my.coef[1], slope = my.coef[2], color="#00a000", linetype="dotted")
  
  plot(p)
  
}

########################################################
# Command line parsing
########################################################

my.option.list <- list( 
  make_option(
    c("-a", "--minamp"),
    type = "double",
    default = 1000, 
    help = "Min value of peak amplitude (default %default)"
  ),
  make_option(
    c("-b", "--maxamp"),
    type = "double",
    default = 100000, 
    help = "Max value of peak amplitude (default %default)"
  ),
  make_option(
    c("-c", "--channel"),
    type = "integer",
    default = NA, 
    help = "Channel #"
  ),
  make_option(
    c("", "--front"),
    type = "integer",
    default = 10, 
    help = "Number of samples before the peak (default %default)"
  ),
  make_option(
    c("", "--tail"),
    type = "integer",
    default = 10, 
    help = "Number of samples before the peak (default %default)"
  ),
  make_option(
    c("-r", "--resolution"),
    type = "double",
    default = 0.1, 
    help = "Pulse form resolution (default %default)"
  ),
  make_option(
    c("-t", "--trigger"),
    type = "integer",
    default = NA, 
    help = "Trigger channel #"
  )
)

my.opt <- parse_args(
  OptionParser(
    option_list = my.option.list,
    usage = "%prog [options] <txt file directory> <plot file directory>"
  ), 
  positional_arguments = c(2, 2)
)

########################################################
# Processing
########################################################

my.low.level <- 0.05
my.high.level <- 0.95

my.stat.run <- array(dim = c(0))
my.stat.peak.pos <- array(dim = c(0))
my.stat.peak.amp <- array(dim = c(0))
my.stat.front.len <- array(dim = c(0))
my.accum <- NULL

for (my.fn in list.files(path = my.opt$args[1], pattern = my.make.txt.filename.mask(my.opt), full.names = T)) {
  my.accum <- my.process.file(my.fn, my.accum)
}

pdf(my.make.plot.filename(my.opt))

my.plot(my.accum)
my.plot.diff(my.accum)

my.plot.stat(my.stat.peak.pos, "Peak Positions vs. Run", "Peak Position", "channels")
my.plot.stat(my.stat.peak.amp, "Peak Amplitudes vs. Run", "Peak Amplitude", "channels")
my.plot.stat(my.stat.front.len, "Front Lengths vs. Run", "Front Length", "channels")
