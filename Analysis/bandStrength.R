#!/usr/bin/env Rscript
# vim: foldmethod=marker: 
# This script is used to calculate the length of surgical tubing for the recovery system.
# As of this comment (2021-09-19), the recommended length is 39 inches, which allows a little extra length for tuning.

# measurements {{{
F_desired_N <- 100 # N; ideal clamping force from all bands (This was actually measured and is almost exactly 100 N.)
n_bands <- 18 # number of desired bands
g_earth <- 9.80665 # m/s^2; Earth surface gravity
length_cut_extra_in <- 3 # inches; extra length to add to the cut sections
dist_interHole_in <- 0.72 # inches; distance between holes on the spaghetti ring
origLength_in <- 23+7/8 # un-loaded length of test section
OD_in <- 0.2 # nominal outer diamter
ID_in <- 0.12 # nominal inner diameter
length_in <- c( # measured length of test section
            50.5,
            44+1/8,
            38+7/16,
            36+1/16,
            31+5/16,
            29+11/16,
            25+1/2,
            26+1/2,
            24+5/8,
            24+1/4,
            24
)

load_g <- c( # measured load on test section
            780,
            655,
            555,
            455,
            355,
            255,
            105,
            155,
            50,
            25,
            10
)
#plot(length_in, load_g, xlim=c(0, max(length_in)), ylim=c(0,max(load_g)))
# }}}

# conversions {{{
F_desired_lbf <- 0.22480894*F_desired_N # lbf; desired clamping force from all bands
L_des_stretch_in <- 29.25 # in; design length when stretched
load_lbf <- 0.0022046226*load_g # conversion from grams-force to pounds force
A_in2 <- pi*(OD_in/2)^2 - pi*(ID_in/2)^2 # nominal cross-sectional area
stress_eng_psi <- load_lbf/A_in2 # engineering stress... very different from true stress in this case!
strain <- (length_in-origLength_in)/origLength_in
dat <- data.frame(strain, stress=stress_eng_psi)
# }}}

m_stressStrain_psi <- lm(stress~0+strain+I(strain^2)+I(strain^3), dat) # s = 0 + e + e^2 + e^3
#plot(dat$strain, dat$stress)
#grid()
predf_stressStrain_psi <- function(x) predict(m_stressStrain_psi, data.frame(strain=x))
#curve(predf_stressStrain_psi, add=T)
#curve(function(x) predict(m, data.frame(strain=x)), add=T) # not sure why this doesn't work
summary(m_stressStrain_psi)

stress_desired_psi <- F_desired_lbf/A_in2/n_bands

rootf <- function(x) { predf_stressStrain_psi(x)-stress_desired_psi }
#plot(rootf)
#abline(h=0)
strain_desired <- uniroot(rootf, range(strain))$root

L_des_0 <- L_des_stretch_in/(1+strain_desired)
cat("This is the length we want to have each individual strand:\n")
cat("L_des_0:", L_des_0, " inches\n")
cat("\n")
cat("So, we should cut ", n_bands/2, " peices of tubing in lengths of ", ceiling(L_des_0*2+dist_interHole_in+length_cut_extra_in), " inches.\n") # 39 in

#plot(m)
