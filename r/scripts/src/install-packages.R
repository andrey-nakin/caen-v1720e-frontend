#!/usr/bin/Rscript
# Arguments: <package> [ <package> ...]

my.pkgTest <- function(x) {
  if (!require(x, character.only = TRUE)) {
    install.packages(x, dep=TRUE, repos = "https://cloud.r-project.org")
    if(!require(x, character.only = TRUE)) {
      stop("Package not found")
    }
  }
}

my.cmd.args <- commandArgs(trailingOnly=TRUE)
for (pn in my.cmd.args) {
  my.pkgTest(pn)
}

warnings()
