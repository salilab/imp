import math

# various parameters for restraint are specified here
kappa_=1000.0
max_score_=0.01

# packing restraint
packing_nsig_=3
packing_ncl_=13

# scoring function
score_name_="dope_score.lib"

# other restraints
diameter_=35.0
z_range_=[-3.5,3.5]
tilt_range_=[0,math.radians(45)]
