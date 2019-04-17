#!/usr/bin/Rscript
# Arguments: <source file> <destination file>

my.pkgTest <- function(x) {
  if (!require(x, character.only = TRUE)) {
    install.packages(x, dep=TRUE, repos = "https://cloud.r-project.org")
    if(!require(x, character.only = TRUE)) {
      stop("Package not found")
    }
  }
}

my.pkgTest("ggplot2")
my.pkgTest("scales")

library(ggplot2)
library(scales)

my.cmd.args <- commandArgs(trailingOnly=TRUE)

my.raw <- read.table(my.cmd.args[1], header = T)
pdf(my.cmd.args[2]) 

my.plot <- function(my.time, y, main, ylab, ch, units) {
  my.time <- my.time + 3 * 3600 # time zone shift
  my.data <- data.frame(x = my.time, y = y)
  my.lm <- lm(y ~ x, data = my.data)
  my.coef <- coefficients(my.lm)
  my.slope <- my.coef[2] * 3600
  my.slope.ci <- confint(my.lm, 'x', level = 0.95) * 3600
  my.slope.err = max(abs(my.slope - my.slope.ci[1]), abs(my.slope - my.slope.ci[2]))
  
  p <- ggplot(
    data = my.data, 
    aes(x = as.POSIXct(x, origin = "1970-01-01", tz = "UTC"), y = y)
  ) + 
  geom_line() +
  ggtitle(
    label = paste(main, ", Channel #", ch, sep = ""),
    subtitle = paste("Approximation Line Slope: ", sprintf("%0.2g +/- %0.1g", my.slope, my.slope.err), " ", units, "/h", sep = "")
  ) +
  scale_x_datetime(labels = date_format("%d/%m %H:%M"), name = "Time", date_breaks = "2 hours") + 
  scale_y_continuous(name = paste(ylab, ", ", units, sep = "")) +
  theme_light(base_size = 10) +
  theme(axis.text.x = element_text(angle = 45, hjust = 1)) +
  geom_abline(intercept = my.coef[1], slope = my.coef[2], color="#00a000", linetype="dotted")
  
  plot(p)

}

for (ch in seq(0, 7)) {
  my.n <- length(my.raw$TS0)
  my.indices <- 1 : (my.n)
  my.time0 <- my.raw$TS0[my.indices]
  my.time <- my.raw$TS1[my.indices]
  my.mean <- my.raw[[paste("CH", ch, sep = "")]][my.indices]
  my.sd <- my.raw[[paste("SD", ch, sep = "")]][my.indices]
  my.nt <- my.raw[[paste("TRG", ch, sep = "")]][my.indices]
  
  my.plot(
    my.time = my.time,
    y = my.mean * 1e3,
    main = "Mean Voltage",
    ylab = "Mean Voltage",
    units = "mV",
    ch = ch
  )  

  my.plot(
    my.time = my.time,
    y = my.sd * 1e3,
    main = "Voltage Standard Deviation",
    ylab = "Std. Deviation",
    units = "mV",
    ch = ch
  )  

  if (sum(my.nt) > 0) {
    my.plot(
      my.time = my.time,
      y = my.nt / (my.time - my.time0),
      main = "Trigger Rate",
      ylab = "Trigger Rate",
      units = "Hz",
      ch = ch
    )  
  }
  
}

#warnings()