#!/usr/bin/Rscript

library(optparse)
library(ggplot2)
library(stringr)

########################################################
# Functions
########################################################

my.plot <- function() {
  my.df <- data.frame(
    x = rep(my.x, 3),
    y = append(append(my.wf.avg, my.wf.avg + qnorm(0.10) * my.wf.std), my.wf.avg + qnorm(0.90) * my.wf.std),
    c = append(append(rep("a) Mean", length(my.wf.avg)), rep("b) 10th Percentile", length(my.wf.avg))), rep("c) 90th Percentile", length(my.wf.avg))),
    err = append(my.wf.avg.err * qnorm(0.975), rep(NA, 2 * length(my.wf.avg.err)))
  )

  pdf(my.plot.file)
  
  my.p <- ggplot(data = my.df, aes(x = x, y = y, group = c, color = c)) +
    ggtitle(
      label = paste(
        "Averaged Pulse", 
        paste("Channel #", my.opt$options$channel, sep = ""), 
        paste("Trigger #", my.opt$options$trigger, sep = ""), 
        paste("Amplitudes", paste(my.opt$options$minamp, my.opt$options$maxamp, sep = "-"), sep = ": "),
        sep = ", "
      ),
      subtitle = paste(
        paste("Peak Pos", my.opt$options$front, sep = ": "),
        paste("Peak Amp", format(my.wf.avg[my.opt$options$front + 1], digits = 2), sep = ": "),
        paste("Num of WFs", my.wf.count, sep = ": "),
        paste("10/90 Front Length", format(my.root2 - my.root1, digits = 2), sep = ": "),
        sep = ", "
      )
    ) +
    scale_x_continuous(name = "Time, channels") +
    scale_y_continuous(name = "Amplitude, channels") +
    theme_light(base_size = 10) +
    geom_errorbar(aes(ymin=y - err, ymax = y + err), width = .3, position = position_dodge(0.05)) +
    geom_line() +
    geom_line(aes(x = x, y = rep(my.level1, length(x))), size = 0.25, color = 'magenta') +
    geom_line(aes(x = x, y = rep(my.level2, length(x))), size = 0.25, color = 'magenta') +
    geom_point(aes(shape = c)) + scale_shape_manual(values=c(20, NA, NA)) +
    warnings()
  
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
  my.level1 <- my.low.level * (df$MY[my.opt$options$front] - df$MY[1]) + df$MY[1]
  my.level2 <- my.high.level * (df$MY[my.opt$options$front] - df$MY[1]) + df$MY[1]
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
      front.len = my.root2 - my.root1
    )
  )
    
}

my.extract.run.no <- function(fn) {
  return(as.numeric(sub(".*\\.run([0-9]+)\\..*", "\\1", basename(fn), perl=TRUE)))
}

my.process.file <- function(fn) {
  
  cat("FILE", fn, "\n")
  my.df <- read.table(fn, header = T, sep = "\t")
  my.info <- my.analyze(my.df)
  
  my.stat.run <<- append(my.stat.run, my.extract.run.no(fn))
  my.stat.peak.pos <<- append(my.stat.peak.pos, my.info$peak.pos)
  my.stat.peak.amp <<- append(my.stat.peak.amp, my.info$peak.amp)
  my.stat.front.len <<- append(my.stat.front.len, my.info$front.len)
  
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
    theme_light(base_size = 10) +
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

for (my.fn in list.files(path = my.opt$args[1], pattern = my.make.txt.filename.mask(my.opt), full.names = T)) {
  my.process.file(my.fn)
}

pdf(my.make.plot.filename(my.opt))

my.plot.stat(my.stat.peak.pos, "Peak Positions", "Peak Position", "channels")
my.plot.stat(my.stat.peak.amp, "Peak Amplitudes", "Peak Amplitude", "channels")
my.plot.stat(my.stat.front.len, "Front Lengths", "Front Length", "channels")

# my.wf.avg <- my.wf.sum / my.wf.count
# my.wf.var <- (my.wf.count * my.wf.sum2 - my.wf.sum^2) / (my.wf.count * (my.wf.count - 1))
# my.wf.std <- sqrt(my.wf.var)
# my.wf.avg.err <- my.wf.std / sqrt(my.wf.count)
# my.x <- seq(0, length(my.wf.avg) - 1)
# 
# my.func <- approxfun(my.x, my.wf.avg)
# my.level1 <- 0.1 * (my.wf.avg[my.opt$options$front + 1] - my.wf.avg[1]) + my.wf.avg[1]
# my.level2 <- 0.9 * (my.wf.avg[my.opt$options$front + 1] - my.wf.avg[1]) + my.wf.avg[1]
# if (my.level2 < my.level1) {
#   my.root1 <- uniroot(f = function(x) my.func(x) - my.level1, interval = c(0, my.opt$options$front))$root
#   my.root2 <- uniroot(f = function(x) my.func(x) - my.level2, interval = c(0, my.opt$options$front))$root
# } else {
#   my.root1 <- 0
#   my.root2 <- 0
# }

# my.plot()

#warnings()