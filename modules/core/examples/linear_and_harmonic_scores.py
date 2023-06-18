## \example core/linear_and_harmonic_scores.py
# An example for setting a linear or harmonic score between two particles
# using either a point distance (between particle centers) or a sphere
# distance (between particle surfaces).
#
# Note: this example relies on matplotlib for plotting the scores,
#       but it can be easily modified to just print the scores.
#
# Author: Barak Raveh, 2022/12/13

import IMP
import IMP.algebra
import IMP.core
try:
    import matplotlib.pyplot
except ImportError:
    matplotlib = None
import numpy
import sys


IMP.setup_from_argv(sys.argv, "linear or harmonic score example")


DEFAULT_RADIUS = 2.0 # radius of particles
LINEAR_OFFSET = 4.0 # distance at which the function is zero (note this is not the Y-axis intercept!)
LINEAR_SLOPE = 3.0   # slope of linear score (= force in kcal/mol/A)
HARMONIC_MEAN = 3    # distance at which the function is minimal/maximal
HARMONIC_K = 2.0     # quadratic coefficient

def create_particle(m, radius = DEFAULT_RADIUS):
    p = IMP.Particle(m)
    s = IMP.algebra.Sphere3D([0,0,0], radius)
    IMP.core.XYZR.setup_particle(p, s)
    return p

def create_linear_point_pair_score():
    ''' slope*x + intercept for point distance '''
    linear_functor = IMP.core.Linear(LINEAR_OFFSET, LINEAR_SLOPE)
    return IMP.core.DistancePairScore(linear_functor)

def create_harmonic_point_pair_score():
    ''' 0.5*k*(x-mean)^2 for point distance '''
    harmonic_functor = IMP.core.Harmonic(HARMONIC_MEAN, HARMONIC_K)
    return IMP.core.DistancePairScore(harmonic_functor)

def create_linear_sphere_pair_score():
    ''' slope*x + intercept for sphere distance '''
    linear_functor = IMP.core.Linear(LINEAR_OFFSET, LINEAR_SLOPE)
    return IMP.core.SphereDistancePairScore(linear_functor)

def create_harmonic_sphere_pair_score():
    ''' 0.5*k*(x-mean)^2 for sphere distance '''
    harmonic_functor = IMP.core.Harmonic(HARMONIC_MEAN, HARMONIC_K)
    return IMP.core.SphereDistancePairScore(harmonic_functor)

def create_model(pair_score):
    m = IMP.Model()
    particles = [create_particle(m) for x in range(2)]
    restraint = IMP.core.PairRestraint(m, pair_score, particles)
    xyzrs = [IMP.core.XYZR(p) for p in particles]
    return m, xyzrs, restraint

def plot_score(pair_score, caption,
               xmin = -15.0, xmax = 15.0, xstep = 0.01):
    '''
    Plots a pair_score between two particles, one particle
    being at [0,0,0] and the other particle being at [x,0,0]
    for x in the closed interval [xmin:xstep:xmax]
    '''
    m, xyzrs, restraint = create_model(pair_score)
    xyzrs[0].set_coordinates([0,0,0])
    X = numpy.arange(xmin, xmax+0.1*xstep, xstep)
    Y = 0.0*X
    for i,x in enumerate(X):
        xyzrs[0].set_coordinates([x,0,0])
        Y[i] = restraint.get_score()
    if not matplotlib:
        print("Not showing plot; matplotlib is not installed "
              "or could not be imported")
    elif IMP.get_is_quick_test():
        print("Not showing plot, as we are running test cases")
    else:
        matplotlib.pyplot.plot(X,Y,'-')
        matplotlib.pyplot.title(caption)
        matplotlib.pyplot.xlabel(r"$X_2$ [$\AA$]")
        matplotlib.pyplot.ylabel("Energy [$kcal \cdot mol^{-1}$]")
        matplotlib.pyplot.gca().spines['bottom'].set_position(('data', 0))
        matplotlib.pyplot.show()

if __name__ == "__main__":
    # NOTE: sphere distance is not distance!
    linear_str = "{:.1f}*(dist-{:.1f})".format(LINEAR_SLOPE, LINEAR_OFFSET)
    harmonic_str = "{:.1f}*(dist-{:.1f})^2".format(HARMONIC_K, HARMONIC_MEAN)
    plot_score(create_linear_point_pair_score(),
               caption="Linear point distance\n{}"
               .format(linear_str))
    plot_score(create_linear_sphere_pair_score(),
               caption="Linear sphere distance (R={:.1f} A)\n{}"
               .format(DEFAULT_RADIUS, linear_str))
    plot_score(create_harmonic_point_pair_score(),
               caption="Harmonic point distance\n{}"
               .format(harmonic_str))
    plot_score(create_harmonic_sphere_pair_score(),
               caption="Harmonic sphere distance (R={:.1f} A)\n{}"
               .format(DEFAULT_RADIUS, harmonic_str))
