#!/usr/bin/env python

__doc__ = """
This module provides a few methods to improve the efficiency of a replica-exchange simulation
by tuning its parameters.
Author: Yannick Spill
"""

import sys
#from mpmath import erf, erfinv
#from math import sqrt
#from random import randint
from numpy import *
from numpy.random import randint
import rpy2.robjects as robjects

kB = 1.3806503 * 6.0221415 / 4184.0  # Boltzmann constant in kcal/mol/K
# here for float comparison. Floats are equal if their difference
EPSILON = 1e-8
                # is smaller than EPSILON
debug = False


def prdb(arg):
    if debug:
        print arg

# R compatibility functions

r = robjects.r
robjects.globalenv["kB"] = kB
_rinverf = r('invErf <-  function(x) {qnorm((1 + x) /2) / sqrt(2)}')
_rerf = r('erf <- function(x) {2 * pnorm(x * sqrt(2)) - 1}')
_rinvF = r('qf')
_rinterp = None


def erfinv(x):
    return _rinverf(x)[0]


def erf(x):
    return _rerf(x)[0]


def Finv(x, d1, d2):
    return _rinvF(x, d1, d2)[0]


def spline(xy, mean, method=None):
    """spline interpolation of (x,y) coordinates. If interpolation goes
    negative, replace by mean value.
    """
    x, y = zip(*xy)
    robjects.globalenv["x"] = robjects.FloatVector(x)
    robjects.globalenv["y"] = robjects.FloatVector(y)
    global _rinterp
    #_rinterp = r.splinefun(x,y)
    if method is None:
        r('cvsplinenonbounded <- splinefun(x,y)')
    else:
        r('cvsplinenonbounded <- splinefun(x,y,method="%s")' % method)
    _rinterp = r(
        'cvspline <- function(x) { tmp = cvsplinenonbounded(x); if (tmp>0) {tmp} else {%f}}' %
        mean)

    def interpolated(x):
        global _rinterp
        return _rinterp(x)[0]
    return interpolated


def linear_interpolation(xy, mean):
    """linear interpolation of (x,y) coordinates. No extrapolation possible.
    """
    x, y = zip(*xy)
    robjects.globalenv["x"] = robjects.FloatVector(x)
    robjects.globalenv["y"] = robjects.FloatVector(y)
    global _rinterp
    #_rinterp = r.splinefun(x,y)
    _rinterp = r('cvspline <- approxfun(x,y)')

    def interpolated(x):
        global _rinterp
        return _rinterp(x)[0]
    return interpolated


# R testing functions

def anova(*args):
    """perform anova using R and return statistic, p-value, between and within variance"""
    ngroups = len(args)  # number of groups
    # nreps = len(args[0]) #number of repetitions
    #group = r.gl(ngroups,nreps)
    reps = r.rep(0, len(args[0]))
    weight = robjects.FloatVector(args[0])
    for i in xrange(1, len(args)):
        reps += r.rep(i, len(args[i]))
        weight += robjects.FloatVector(args[i])
    group = r.factor(reps)
    robjects.globalenv["weight"] = weight
    robjects.globalenv["group"] = group
    lm = r.lm("weight ~ group")
    aov = r.anova(lm)
    prdb(aov)
    # F statistic, p-value, between and within variance
    anova_result = {'fstat': aov[3][0],
                    'pval': aov[4][0],
                    'between': aov[2][0],
                    'within': aov[2][1],
                    'nsteps': [len(i) for i in args],
                    'nreps': ngroups,
                    'test': 'anova'}  # nreps: number of replicas
    return anova_result


def kruskal(*args):
    """perform kruskal-wallis rank test"""
    ngroups = len(args)
    #nreps = len(args[0])
    #group = r.gl(ngroups,nreps)
    reps = r.rep(0, len(args[0]))
    weight = robjects.FloatVector(args[0])
    for i in xrange(1, len(args)):
        reps += r.rep(i, len(args[i]))
        weight += robjects.FloatVector(args[i])
    group = r.factor(reps)
    aov = r('kruskal.test')(group, weight)
    prdb(aov)
    kruskal_result = {'fstat': aov[0][0],
                      'pval': aov[2][0],
                      'nsteps': [len(i) for i in args],
                      'nreps': ngroups,
                      'test': 'kruskal'}
    return kruskal_result  # F statistic and p-value


def ttest(obs, target):
    """perform a one-sample two-sided t-test on obs against target mean"""
    test = r('t.test')(robjects.IntVector(obs), mu=target)
    return test[0][0], test[2][0]  # stat and p-value


def binom(obs, target):
    """perform an exact binomial test on the mean of obs against target"""
    success = sum(obs)
    trials = len(obs)
    test = r('binom.test')(success, trials, p=target)
    return test[0][0], test[2][0]  # stat and p-value


def bartlett(*args):
    """perform bartlett's test on the equality of variances of the observations"""
    ngroups = len(args)
    nreps = len(args[0])
    group = r.gl(ngroups, nreps)
    weight = robjects.IntVector(args[0])
    for i in args[1:]:
        weight += robjects.IntVector(i)
    robjects.globalenv["weight"] = weight
    robjects.globalenv["group"] = group
    var = r('bartlett.test')(weight, group)
    return var[0][0], var[2][0]  # statistic and p-value


def fligner(*args):
    """perform Fligner-Killeen non-parametric test of the variance equality"""
    ngroups = len(args)
    nreps = len(args[0])
    group = r.gl(ngroups, nreps)
    weight = robjects.IntVector(args[0])
    for i in args[1:]:
        weight += robjects.IntVector(i)
    robjects.globalenv["weight"] = weight
    robjects.globalenv["group"] = group
    var = r('fligner.test')(weight, group)
    return var[0][0], var[2][0]  # statistic and p-value


def power_test(ar, power=0.8, alpha=0.05):
    """perform an anova power test and return
    - the power of the test with this input data
    - the number of points that would be needed to achieve a default power of 0.8
    ar: the ouput of anova()
    """
    result = r('power.anova.test')(groups=ar['nreps'], n=min(ar['nsteps']),
                                   between=ar['between'], within=ar['within'], sig=alpha)
    prdb('the power of this anova was: %.3f' % result[5][0])
    result = r('power.anova.test')(groups=ar['nreps'],
                                   between=ar['between'], within=ar['within'], sig=alpha, pow=power)
    prdb(
        'To have a power of %.3f, there should be at least %d exchange attemps.' %
        (power, result[1][0]))
    return


def minimum_n(ar, alpha=0.05):
    """This routine tries to return an estimate of the additional number of exchange trials that
    could lead to a positive result of the anova (e.g. the average ARs are not the same). It is
    still very crude. It also assumes that a one-way anova was made.
    ar: the output of anova()
    alpha: type I error
    """
    nreps = ar['nreps']
    nsteps = ar['nsteps']
    try:
        nsteps = min(nsteps)
    except:
        pass
    fstat = ar['fstat']
    return (
        nsteps *
        (sqrt(Finv(1 - alpha, nreps - 1, nreps * (nsteps - 1)) / fstat) - 1)
    )


# Heat capacity class
class CvEstimator:

    """When created, estimates the heat capacity from the energies or from the
    indicator functions using the specified method. Two methods are then
    available to guess the heat capacity at a given parameter value: get()
    returns a single point, and mean() returns the linear average between two
    points.
    """

    def __init__(self, params, energies=None, indicators=None,
                 method="constant", temps=None, write_cv=False):

        kB = 1.3806503 * 6.0221415 / 4184.0  # Boltzmann constant in kcal/mol/K
        self.__initialized = False
        self.__cv = []
        self.method = method

        if method == "interpolate":
            self.estimate_cv_interpolate(params, indicators)
            self.get = self.get_interp
            self.mean = self.mean_interp
        elif method == "constant":
            self.estimate_cv_constant(params, indicators)
            self.get = lambda p: self.__cv
            self.mean = lambda p1, p2: self.__cv
            self.__cvfun = self.get
            r('cvspline <- function(x) {%f}' % self.__cv)
        elif method == "mbar":
            self.estimate_cv_mbar(params, energies, temps)
            self.get = self.get_mbar
            self.mean = self.mean_mbar
        else:
            raise NotImplementedError(method)

        self.__initialized = True

        # write the heat capacity to a file
        if write_cv:
            fl = open('cv', 'w')
            fl.write("".join(["%f %f\n" % (x, self.get(x))
                     for x in linspace(params[0] / 2, 2 * params[-1])]))
            fl.close()

    def estimate_cv_interpolate(self, params, indicators):
        """interpolate using previous values, by reversing the approximate overlap
        function
        """
        if self.__initialized:
            return
        if len(indicators) != len(params) - 1:
            raise ValueError(
                "the length of indicators and params does not match!")
        if params != tuple(sorted(params)):
            raise NotImplementedError(
                "unable to work on parameters that do not change monotonically")

        prdb("storing params and means")
        self.__params = params
        self.__pmeans = [(params[i] + params[i + 1]) / 2.
                         for i in xrange(len(params) - 1)]
        prdb("computing __cv")
        for i, ind in enumerate(indicators):
            mean = sum(ind) / float(len(ind))
            Y2 = 2 * kB * erfinv(1 - mean) ** 2
            p1 = params[i]
            p2 = params[i + 1]
            self.__cv.append(
                (self.__pmeans[i], (p1 ** 2 + p2 ** 2) * float(Y2) / (p2 - p1) ** 2))
        prdb(self.__params)
        prdb(self.__cv)
        self.__cvmean = sum([i[1] for i in self.__cv]) / float(len(self.__cv))
        if self.__cvmean < 0:
            raise ValueError("Cv mean is negative!")
        self.__cvfun = spline(self.__cv, self.__cvmean)
        return

    def estimate_cv_constant(self, params, indicators):
        """try to guess which constant cv fits best"""
        if self.__initialized:
            return
        self.estimate_cv_interpolate(params, indicators)
        self.__cv = self.__cvmean
        return

    def needs_init(self):
        if not self.__initialized:
            raise RuntimeError("Class was not initialized correctly!")

    def estimate_cv_mbar(params, energies, temps):
        "use MBAR to get the heat capacity"
        raise NotImplementedError(method)
        if self.__initialized:
            return

    def _isinbounds(self, p, params):
        """returns True if p is within parms, else false. the params list must be sorted ascendingly."""
        if p < params[0] - EPSILON or p > params[-1] + EPSILON:
            #prdb("Warning: value %f is outside of bounds, extrapolating." % p)
            return False
        else:
            return True

    def _interpolate(self, xval, xlist):
        """return interpolation of Cv at point xval, and return the average instead
        if this value is negative.
        """
        self._isinbounds(xval, xlist)
        val = self.__cvfun(xval)
        if val > 0:
            return val
        else:
            return self.__cvmean

    def get_interp(self, param):
        """returns the point estimate of the first derivative of the energy with
        respect to the replica exchange parameter (usually T or q).
        This version assumes that the means of cv are given.
        """
        self.needs_init()
        return self._interpolate(param, self.__pmeans)

    def get_mbar(self, param):
        """returns the point estimate of the first derivative of the energy with
        respect to the replica exchange parameter (usually T or q).
        This version assumes that the values of cv are given.
        """
        self.needs_init()
        return self._interpolate(param, self.__params)

    def mean_interp(self, pa, pb):
        """estimate the mean of Cv between two points. Here the means were
        stored previously
        """
        self.needs_init()
        return self._interpolate((pa + pb) / 2., self.__pmeans)

    def mean_mbar(self, pa, pb):
        self.needs_init()
        return (self.get_mbar(pb) + self.get_mbar(pa)) / 2.

# Parameter updating methods


def update_good_dumb(newp, oldp, *args, **kwargs):
    """Here the old parameters are oldp[0] and oldp[1], and the starting point
    is newp[0]. We should modify newp[1] so that the AR in the following cycle
    is equal to the targetAR.
    In the "dumb" method, the Cv and targetAR keywords are ignored.
    Here the newp[1] parameter is modified because prior changes have set
    newp[0] to a different value than oldp[0]. Thus, we should move newp[1] by
    minimizing the effect on the AR since it is supposedly equal to targetAR. In
    this simple method, the parameter is just translated.
    """
    prdb(
        "newp[0] has moved (%.3f -> %.3f), adjusting the position of newp[1]" %
        (oldp[0], newp[0]))
    return oldp[1] - (oldp[0] - newp[0])


def update_bad_dumb(newp, oldp, ind, targetAR=0.4, scale=0.1, **kwargs):
    """Here the old parameters are oldp[0] and oldp[1], and the starting point
    is newp[0]. We should modify newp[1] so that the AR in the following cycle
    is equal to the targetAR.
    In the "dumb" method, the Cv keyword is ignored. Here the newp[1]
    parameter is modified to follow the possible translation of newp[0] by
    calling update_good_dumb, and then newp[1] is added or substracted scale% of
    (oldp[1] - oldp[0]) to adjust to targetAR.
    """

    if newp[0] != oldp[0]:
        newp[1] = update_good_dumb(newp, oldp)
    if targetAR > sum(ind) / float(len(ind)):
        prdb("""target AR is higher than expected, decreasing newp[1]""")
        newp[1] -= scale * (oldp[1] - oldp[0])
    else:
        prdb("""target AR is lower than expected, increasing newp[1]""")
        newp[1] += scale * (oldp[1] - oldp[0])
    return newp[1]


def update_any_cv_step(newp, oldp, ind, targetAR=0.4, Cv=None, **kwargs):
    """here we use the average AR formula of two gaussians to get newp[1] as a
    function of newp[1], knowing the targetAR and estimating the Cv. If targetAR
    is negative, consider that mean(ind) equals the target AR and skip any
    calculation in the case that oldp[0] equals newp[0].
    step: suppose the heat capacity is stepwise constant, i.e. use the heat
    capacity at position newp[0] as an estimate of the mean of the heat capacity
    between newp[0] and newp[1]. This does not require any self-consistent loop.
    """

    global kB

    if abs(oldp[0] - newp[0]) < EPSILON and targetAR < 0:
        return oldp[1]
    if targetAR < 0:
        targetAR = sum(ind) / float(len(ind))
    cv = Cv.get(newp[0])
    Y = sqrt(2 * kB) * float(erfinv(1 - targetAR))
    if Y ** 2 >= cv:
        raise ValueError("""targetAR too small for this approximate method, use
        the full self-consistent method instead.""")
    return newp[0] * (cv + Y * sqrt(2 * cv - Y ** 2)) / (cv - Y ** 2)


def update_any_cv_sc(newp, oldp, ind, targetAR=0.4, Cv=None,
                     tol=1e-6, maxiter=10000):
    """self-consistent solver version"""

    global kB

    if abs(oldp[0] - newp[0]) < EPSILON and targetAR < 0:
        return oldp[1]
    if targetAR < 0:
        targetAR = sum(ind) / float(len(ind))
    cv = Cv.get(newp[0])
    Y = sqrt(2 * kB) * float(erfinv(1 - targetAR))
    if Y ** 2 >= cv:
        raise ValueError("""targetAR too small for this approximate method, use
        the full self-consistent method instead.""")
    targetp = newp[0] * (cv + Y * sqrt(2 * cv - Y ** 2)) / (cv - Y ** 2)
    for i in xrange(maxiter):
        cv = Cv.mean(newp[0], targetp)
        (oldtargetp, targetp) = (
            targetp, newp[0] * (cv + Y * sqrt(2 * cv - Y ** 2)) / (cv - Y ** 2))
        if abs(targetp - oldtargetp) <= tol:
            break
        if isnan(targetp):
            if Y ** 2 >= cv:
                raise ValueError("""targetAR too small for this approximate method, use
                the full self-consistent method instead.""")
            else:
                raise ValueError("""something unexpected happened""")
    if i == maxiter - 1:
        prdb("""Warning: unable to converge the self-consistent after %d
        iterations and a tolerance of %f. Change the method or decrease the
        tolerance!""" % (maxiter, tol))
    prdb("converged after %d iterations and a tolerance of %f for x=%f" %
         (i, tol, oldp[1]))
    return targetp


def update_any_cv_scfull(newp, oldp, ind, targetAR=0.4, Cv=None,
                         tol=1e-6, maxiter=10000):
    """self-consistent solver version, on the exact average AR equation"""

    # create helper functions and overlap function
    #_ru21 = r('u21 <- function(t1,t2) { (t2-t1)*(cvspline(t2)-cvspline(t1))/2. }')
    _ru21 = r(
        'u21 <- function(t1,t2) { integrate(Vectorize(cvspline),t1,t2)$value }')
    _rb21 = r('b21 <- function(t1,t2) { 1./(kB*t2) - 1./(kB*t1) }')
    _rsigma2 = r('sigma2 <- function(t) {cvspline(t)*kB*t**2}')
    _rovboltz = r('ovboltz <- function(t1,t2) {\
            1/2*( 1-erf(\
                    u21(t1,t2)/sqrt(2*(sigma2(t1)+sigma2(t2))))\
                + exp(b21(t1,t2)*(u21(t1,t2)+b21(t1,t2)*(sigma2(t1)+sigma2(t2))/2))\
                    * (1+erf((u21(t1,t2)+b21(t1,t2)*(sigma2(t1)+sigma2(t2)))/(sqrt(2*(sigma2(t1)+sigma2(t2))))))\
                )}')
    _rrootfn = r(
        'rootfn <- function(t2) {ovboltz(%f,t2)-%f}' %
        (newp[0], targetAR))

    # find upper bound for estimation, raise an error if cv is negative
    if oldp[1] > oldp[0]:
        tmp = newp[0] * 1.05
    else:
        tmp = newp[0] * 0.95
    nloops = 0
    while _rrootfn(tmp)[0] >= 0:
        nloops += 1
        tmp += (oldp[1] - oldp[0])
        if Cv.get(tmp) < 0:
            raise RuntimeError("heat capacity goes negative")
        if nloops > maxiter:
            raise RuntimeError('could not find zero of function!')

    # find root
    _runiroot = r(
        'uniroot(rootfn,c(%f,%f),f.lower = %f, f.upper = %f, tol = %f, maxiter = %d)' % (newp[0], tmp,
                                                                                         1 - targetAR, -targetAR, tol, maxiter))
    prdb(
        "self-consistent solver converged after %s iterations and an estimated precision of %s " %
        (_runiroot[2][0], _runiroot[3][0]))
    prdb(
        ["root:",
         _runiroot[0][0],
         "overlap:",
         _rovboltz(newp[0],
                   _runiroot[0][0])[0]])
    return _runiroot[0][0]


def update_any_cv_nr(newp, oldp, ind, targetAR=0.4, Cv=None, **kwargs):
    """newton-raphson solver version"""

    # use nlm
    raise NotImplementedError

# Testing methods


def are_equal_to_targetAR(
    indicators,
    targetAR=0.4,
    alpha=0.05,
        method="binom"):
    """here, all indicators have same average, we want to know if it is equal to
    targetAR
    """

    # calculate sample mean deviation of each indicator function from targetAR
    deviations = sorted([(abs(sum(ind) / float(len(ind)) - targetAR), ind)
                        for pos, ind in enumerate(indicators)])
    deviant = deviations[-1]

    # perform t-test
    if method == "ttest":
        #from statlib.stats import ttest_1samp as ttest
        ttest = ttest
    elif method == "binom":
        ttest = binom
    else:
        raise NotImplementedError

    try:
        test, pval = ttest(deviant[1], targetAR)
    except:
        if abs(targetAR - sum(deviant[1]) / len(deviant[1])) > EPSILON:
            pval = 0
        else:
            pval = 1
    if pval < alpha:
        return False
    else:
        return True


def are_stationnary(indicators, alpha=0.05, method="anova"):
    """test on the stationarity of the observations (block analysis). Done so by
    launching an anova on the difference between the two halves of each observations.
    """

    if method == "kruskal":
        test = kruskal
    else:
        test = anova

    tmp = array(indicators)
    blocklen = len(indicators[0]) / 2
    block = tmp[:, :blocklen] - tmp[:, blocklen:2 * blocklen]
    if test(*block)['pval'] < alpha:
        return False
    else:
        return True


def are_equal(indicators, targetAR=0.4, alpha=0.05,
              method="anova", varMethod="skip", power=0.8):
    """Perform a one-way ANOVA or kruskal-wallis test on the indicators set,
    and return True if one cannot exclude with risk alpha that the indicators
    AR are different (i.e. True = all means are equal). Also performs a test
    on the variance (they must be equal).
    """

    if min(targetAR, 1 - targetAR) * len(indicators[0]) <= 5 and \
            (varMethod == "bartlett" or method == "anova"):
        prdb("Warning: normal approximation to the binomial does not hold!")

    # test the variances
    if varMethod == "skip":
        pass
    else:
        if varMethod == "bartlett":
            pval = bartlett(*indicators)[1]
        elif varMethod == "fligner":
            pval = killeen(*indicators)[1]
        else:
            raise NotImplementedError(
                "variance testing method unknown: %s" %
                varMethod)
        if pval < alpha:
            prdb("Warning: performing mean test with unequal variances.")

    if method == "kruskal":
        test = kruskal
    else:
        test = anova

    tr = test(*indicators)
    tr['alpha'] = alpha
    # p-value < alpha => H0 rejected => result == False
    tr['result'] = tr['pval'] >= alpha

    # if tr['test'] == 'anova':
        # try:
        #    power_test(tr, power=power, alpha=alpha)
        # except:
        #    prdb("power test failed")
        #    pass

    return tr


def find_good_ARs(indicators, targetAR=0.4, alpha=0.05, method="binom"):
    """perform one-sample t-tests on each of the data sets, and return
    a tuple of bool of size N-1, False if AR i is not equal to targetAR
    at risk alpha.
    """

    # calculate sample means of each indicator function
    means = sorted([(sum(ind) / float(len(ind)), pos, ind)
                   for pos, ind in enumerate(indicators)])

    # perform t-test
    if method == "ttest":
        #from statlib.stats import ttest_1samp as ttest
        ttest = ttest
    elif method == "binom":
        ttest = binom
    else:
        raise NotImplementedError

    isGoodTuple = []
    # start from the lowest means and stop when they are ok
    prdb("starting left")
    for (i, (mean, pos, ind)) in enumerate(means):
        prdb(
            "performing t-test on couple %d having average AR %f, position %d" %
            (pos, mean, i))
        try:
            test, pval = ttest(ind, targetAR)
        except:
            if abs(targetAR - mean) > EPSILON:
                pval = 0
            else:
                pval = 1
        if pval < alpha:
            # means are different
            isGoodTuple.append((pos, False))
        else:
            goodstart = i
            break
    # then start from the highest means
    prdb("starting right")
    for (i, (mean, pos, ind)) in enumerate(reversed(means)):
        prdb("performing t-test on couple %d having average AR %f, position %d"
             % (pos, mean, len(means) - 1 - i))
        if ttest(ind, targetAR)[1] < alpha:
            # means are different
            isGoodTuple.append((pos, False))
        else:
            goodstop = len(means) - 1 - i
            break

    # limiting cases: all different
    if len(isGoodTuple) > len(indicators):
        return tuple([False] * len(indicators))
    # all equal
    elif len(isGoodTuple) == 0:
        return tuple([True] * len(indicators))
    # intermediate
    else:
        isGoodTuple.extend([(means[i][1], True) for i in
                            xrange(goodstart, goodstop + 1)])
        isGoodTuple.sort()
        return tuple([tup[1] for tup in isGoodTuple])

# Trebst, Katzgraber, Nadler and Hansmann optimum flux stuff


def mean_first_passage_times(
    replicanums_ori,
    subs=1,
    start=0,
        use_avgAR=False):
    """compute mean first passage times as suggested in
    Nadler W, Meinke J, Hansmann UHE, Phys Rev E *78* 061905 (2008)

    use_avgAR : if a list of average ARs is given computes everything from
    average AR; if it is False, compute by direct counting of events.

    returns:
        If use_avgAR == False:
            tau0, tauN, chose_N, times0, timesN
        else:
            tau0, tauN, None, None, None
    tau0[i]: average time to go from replica 0 to replica i
    tauN[i]: average time to go from replica N to replica i
    times0 and timesN are the corresponding lists of single events.

    """

    from numpy import array, zeros
    from pprint import pprint, pformat
    replicanums = array(replicanums_ori)[:, start::subs]
    N = len(replicanums)
    tauN = [0] * N
    tau0 = [0] * N

    if use_avgAR:
        tau0[0] = 0.0
        tauN[-1] = 0.0
        for state in xrange(1, N):
            tau0[state] = tau0[state - 1] + \
                state / (float(use_avgAR[state - 1]))
        for state in reversed(xrange(1, N)):
            tauN[state - 1] = tauN[state] \
                + (N - (state - 1)) / (float(use_avgAR[state - 1]))

        return tau0, tauN, None, None, None

    else:
        #prdb('not using average AR')
        from itertools import izip
        # the algorithm looks for replicas that start at the lowest temp, and
        # records the farthest state it went to before returning to zero. Once
        # back it increments the counter of all concerned replicas. Similar
        # procedure if starting from N.
        store0 = zeros((N, N), dtype=bool)
        last0 = [0 for i in xrange(N)]
        already0 = [False for i in xrange(N)]
        times0 = [[] for i in xrange(N)]
        storeN = zeros((N, N), dtype=bool)
        lastN = [0 for i in xrange(N)]
        alreadyN = [False for i in xrange(N)]
        timesN = [[] for i in xrange(N)]

        #prdb('looping over replicanums')
        for time, frame in enumerate(izip(*replicanums)):
            # case of the replica in state 0
            if not already0[frame[0]]:
                last0[frame[0]] = time
                store0[frame[0], :] = True
                already0[frame[0]] = True
            # case of the replica in state N
            if not alreadyN[frame[-1]]:
                lastN[frame[-1]] = time
                storeN[frame[-1], :] = True
                alreadyN[frame[-1]] = True
            # set already flags to False when in state 1 or N-1
            already0[frame[1]] = False
            alreadyN[frame[-2]] = False
            # loop over all states
            for state, rep in enumerate(frame):
                if store0[rep, state]:
                    # reached a state for the first time since 0
                    store0[rep, state] = False
                    # store time since this replica left state 0
                    times0[state].append(time - last0[rep])
                if storeN[rep, state]:
                    # reached a state for the first time since N
                    storeN[rep, state] = False
                    # store time since this replica left state N
                    timesN[state].append(time - lastN[rep])
        #prdb([replicanums.shape, len(storeN), len(last0)])
        times = [[] for i in xrange(N)]
        chose_N = [len(timesN[state]) > len(times0[state]) for state in
                   xrange(N)]
        for state in xrange(N):
            tauN[state] = sum(timesN[state]) / float(len(timesN[state]))
            tau0[state] = sum(times0[state]) / float(len(times0[state]))
        # prdb(len(chose_N))

        return tau0, tauN, chose_N, times0, timesN


def compute_effective_fraction(tau0, tauN, chose_N):
    """input: tau0, tauN, chose_N
    output: effective fraction f(T) (P_up(n)) as introduced in
    Trebst S, Troyer M, Hansmann UHE, J Chem Phys *124* 174903 (2006).
    formalized in
    Nadler W, Hansmann UHE, Phys Rev E *75* 026109 (2007)
    and whose calculation is enhanced in
    Nadler W, Meinke J, Hansmann UHE, Phys Rev E *78* 061905 (2008)
    the ideal value of f(n) should be 1 - n/N
    """

    # nstar is the index of the last state where tau0 should be used.
    N = len(tau0)
    if chose_N is None:
        nstar = N / 2
    else:
        nstar = N - sum([int(a) for a in chose_N]) - 1

    prdb("n* = %d" % nstar)
    # compute helper functions h
    h0 = [0] * N
    h0[1] = tau0[1]
    for state in xrange(2, nstar + 1):
        h0[state] = h0[state - 1] + \
            (tau0[state] - tau0[state - 1]) / float(state)

    hN = [0] * N
    hN[-2] = tauN[-2]
    for state in reversed(xrange(nstar, N - 1)):
        hN[state] = hN[state + 1] + \
            (tauN[state] - tauN[state + 1]) / float(N - state)

    # compute flow probabilities
    pup = [0] * N
    pup[0] = 1
    for n in xrange(1, nstar + 1):
        pup[n] = 1 - h0[n] / (h0[nstar] + hN[nstar])
    for n in xrange(nstar + 1, N):
        pup[n] = hN[n] / (h0[nstar] + hN[nstar])

    return pup


def spline_diffusivity(pup, params):
    """spline interpolation of  diffusivity: D = 1/(df/dT * heta)
    """
    from numpy import linspace
    robjects.globalenv["hetay"] = \
        robjects.FloatVector(linspace(0, 1, num=len(params)).tolist())
    robjects.globalenv["hetax"] = robjects.FloatVector(params)
    robjects.globalenv["pupx"] = robjects.FloatVector(params)
    robjects.globalenv["pupy"] = robjects.FloatVector(pup)
    heta = r('heta <- splinefun(hetax,hetay,method="monoH.FC")')
    eff = r('eff <- splinefun(pupx,pupy,method="monoH.FC")')
    diff = r('diff <- function(x) {-1/(heta(x,deriv=1)*eff(x,deriv=1))}')
    return lambda x: diff(x)[0]


# Misc
def compute_indicators(replicanums, subs=1, start=0):
    """input: replicanums : a list of N lists of size M, where N is the number of
    states and M is the length of the simulation. Each element is an integer,
    and corresponds to the label of a replica.
    output: an indicator function of exchanges (size (N-1)x(M-1)), 1 if exchange and
    0 if not.
    """
    def exchange(n, m):
        if replicanums[n][m] == replicanums[n + 1][m + 1] \
           and replicanums[n][m + 1] == replicanums[n + 1][m]:
            return 1
        else:
            return 0

    indicators = []
    for n in xrange(len(replicanums) - 1):
        indicators.append([exchange(n, m)
                           for m in xrange(len(replicanums[n]) - 1)][start::subs])
    return indicators

# Main routines


def update_params_nonergodic(pup, params, write_g=False, num=False):

    from numpy import linspace
    #g = spline(zip(pup,params),0,method='monoH.FC')
    g = linear_interpolation(zip(pup, params), 0)
    if write_g:
        d = spline_diffusivity(pup, params)
        fl = open('g', 'w')
        fl.write("".join(["%f %f\n" % (x, g(x))
                 for x in linspace(0, 1, num=100)]))
        fl.close()
        fl = open('diffusivity', 'w')
        fl.write(''.join(["%f %f\n" % (x, d(x)) for x in
                          linspace(params[0], params[-1], num=100)]))
        fl.close()
        fl = open('pup', 'w')
        fl.write("".join(["%f %f\n" % (i, j) for (i, j) in zip(params, pup)]))
        fl.close()

    if num is False:
        newparams = [g(i) for i in reversed(linspace(0, 1, num=len(params)))]
    else:
        newparams = [g(i) for i in reversed(linspace(0, 1, num=num))]
    # for numerical issues
    newparams[0] = params[0]
    newparams[-1] = params[-1]

    return newparams


def update_params(
    indicators, params, isGood, targetAR=0.4, immobilePoint=1,
        Cv=None, badMethod="dumb", goodMethod="dumb", dumb_scale=0.1):
    """update the parameters according to the isGood tuple and using the
    specified methods"""

    newparams = list(params)  # make a copy

    if immobilePoint != 1:
        raise NotImplementedError

    if Cv is None and (badMethod != "dumb" or goodMethod != "dumb"):
        raise RuntimeError("""Cv needs to be estimated if using other methods
        than 'dumb' for updating!""")

    if goodMethod == "dumb":
        update_good = update_good_dumb
    elif goodMethod == "step":
        update_good = update_any_cv_step
    elif goodMethod == "sc":
        update_good = update_any_cv_sc
    elif goodMethod == "scfull":
        update_good = update_any_cv_scfull
    elif goodMethod == "nr":
        update_good = update_any_cv_nr
    else:
        raise NotImplementedError(goodMethod)
    if badMethod == "dumb":
        update_bad = update_bad_dumb
    elif badMethod == "step":
        update_bad = update_any_cv_step
    elif badMethod == "sc":
        update_bad = update_any_cv_sc
    elif badMethod == "scfull":
        update_bad = update_any_cv_scfull
    elif badMethod == "nr":
        update_bad = update_any_cv_nr
    else:
        raise NotImplementedError(badMethod)

    # scan each position starting from the immobilePoint
    for pos in xrange(len(params) - 1):
        if isGood[pos]:
            newparams[pos + 1] = update_good(
                newparams[pos:pos + 2], params[pos:pos + 2],
                indicators[pos], targetAR=targetAR, Cv=Cv)
        else:
            newparams[pos + 1] = update_bad(
                newparams[pos:pos + 2], params[pos:pos + 2],
                indicators[pos], targetAR=targetAR, Cv=Cv, scale=dumb_scale)

    return tuple(newparams)


def tune_params_flux(replicanums, params, subs=1, start=0, alpha=0.05,
                     testMethod='anova', meanMethod='binom', use_avgAR=False,
                     power=0.8, num=False):
    # num is here if you want to add some more temperatures. indicate total
    # number of replicas

    # TODO: do case where one estimates all based on target AR.
    if use_avgAR is not False:
        raise NotImplementedError

    prdb("computing mean first passage times")
    tau0, tauN, chose_N, times0, timesN = mean_first_passage_times(replicanums,
                                                                   subs=subs, start=start, use_avgAR=use_avgAR)

    prdb("average round trip time: %.2f (%d+%d events)" %
        (tau0[-1] + tauN[0], len(times0[-1]), len(timesN[0])))
    prdb("checking if the parameterset needs to be improved")
    N = len(replicanums)
    if chose_N is None:
        nstar = N / 2
    else:
        nstar = N - sum([int(a) for a in chose_N]) - 1

    reduced = []
    # no need to check for times?[0] or times?[N]
    for n in xrange(1, N - 1):
        if n > nstar:
            reduced.append([i * 2.0 / ((N - n) * (N - n + 1))
                           for i in timesN[n]])
        else:
            reduced.append([i * 2.0 / (n * (n + 1)) for i in times0[n]])

    anova_result = are_equal(reduced, alpha=alpha, method=testMethod,
                             power=power)
    if (anova_result['result']):  # TODO test if equal to targetAR
        prdb("flux is constant, nothing to do!")
        min_n = minimum_n(anova_result, alpha)
        prdb('Try to rerun this test with at least %d more samples.' %
             ceil(min_n))
        return (False, min_n)

    # the flux is not constant so the parameters need improvement.
    # calculate the estimate of the effective fraction
    prdb("parameterset not optimal, computing effective fraction")
    pup = compute_effective_fraction(tau0, tauN, chose_N)

    # improve parameterset
    prdb("returning new parameterset")
    params = update_params_nonergodic(pup, params, num=num)

    return (True, params)


def tune_params_ar(
    indicators, params, targetAR=0.4, alpha=0.05, immobilePoint=1, CvMethod="skip", badMethod="dumb", goodMethod="dumb",
    varMethod="skip", testMethod="anova", meanMethod="binom",
        energies=None, temps=None, power=0.8, dumb_scale=0.1):
    """Tune the replica-exchange parameters and return a new set.

    Arguments:
        indicators -- an (N-1)x(M-1) table, where entry at position (j,i)
            is True if replica j and j+1 performed an exchange between
            times i and i+1.
        params -- the current set of N parameters used in the simulation.

    Keyword arguments:
        targetAR -- the target AR which is wanted for the simulation
            (default: 0.4)
        alpha -- the type 1 error on the one-way ANOVA and subsequent
            t-tests (default: 5%)
        immobilePoint -- which replica should keep it's parameter fixed
            (default: 1st replica, e.g. 1)
        CvMethod -- the heat capacity estimation method (default:
            "skip", other options: "mbar", "spline", "constant")
        badMethod -- how to correct the (j+1)th parameter if the
            acceptance ratio between replicas j and j+1 is off the
            target value (default: "dumb", options: "step", "sc", "scfull", "nr")
        goodMethod -- how to update the value of the (j+1)th parameter
            in the case of a correctly exchanging couple, but if the jth
            parameter has been modified (default: "dumb",options: "step",
            "sc" self-consistent, "scfull" self-consistent using the exact equation,
            "nr" newton-raphson solver for the exact equation)
        dumb_scale -- (0.0-1.0) in the "dumb" method, scale wrong temperature
            intervals by this amount. (default: 0.1)
        testMethod -- how to test for the difference of the means,
            either "anova" for a one-way anova, or "kruskal" for a
            Kruskal-Wallis one-way non-parametric anova.
        meanMethod -- "ttest" for a two-sided one-sample t-test,
            "binom" for an exact binomial test of the probability.
        varMethod -- how to test for the equality of variances.
            "fligner" for the Fligner-Killeen non-parametric test,
            "bartlett" for Bartlett's test, "skip" to pass.
        energies -- if CvMethod is set to "mbar", the energies of each
            state as a function of time are used to estimate the heat capacity.
        temps -- the temperatures of the simulations, if estimating with "mbar".

    Return Value:
        returns a tuple: (bool, params). bool is True if params have
        changed, and params is the new set.

    """

    # perform ANOVA
    prdb("performing ANOVA")
    anova_result = are_equal(indicators, targetAR, alpha, method=testMethod,
                             varMethod=varMethod, power=power)
    if (anova_result['result'] and
            are_equal_to_targetAR(indicators, targetAR, alpha, method=meanMethod)):
        prdb("all means are equal to target AR, nothing to do!")
        min_n = minimum_n(anova_result, alpha)
        prdb(
            'Try to rerun this test with at least %d more samples.' %
            ceil(min_n))
        return (False, min_n)
    prdb("some means are different, performing t-tests")

    # perform two-by-two t-tests
    isGood = find_good_ARs(indicators, targetAR, alpha, method=meanMethod)
    if not (False in isGood):
        prdb("""Bad luck: ANOVA says means are not identical, but t-tests
        can't find the bad rates...""")
        return (False, params)
    prdb(isGood)

    # check if data is stationnary by doing a block analysis
    prdb("performing stationarity test")
    if not are_stationnary(indicators, alpha):
        prdb("Warning: Some simulations are not stationary!")

    # interpolate the heat capacity from the data
    # TODO: use all previous data, not just from this run.
    prdb("launching Cv estimation or skipping it")
    if CvMethod == "skip":
        Cv = None
    elif CvMethod == "interpolate" or CvMethod == "constant":
        Cv = CvEstimator(params, indicators=indicators, method=CvMethod)
    elif CvMethod == "mbar":
        Cv = CvEstimator(
            params,
            energies=energies,
            temps=temps,
            method=CvMethod)
    else:
        raise NotImplementedError(CvMethod)

    # update parameters
    prdb('updating params')
    # update the current parameter set to match the target AR
    params = update_params(indicators, params, isGood, targetAR=targetAR,
                           immobilePoint=immobilePoint, Cv=Cv,
                           badMethod=badMethod, goodMethod=goodMethod, dumb_scale=dumb_scale)

    prdb('Done')
    return (True, params)

if __name__ == '__main__':
    from numpy import *
    replicanums = []
    for i in xrange(1, 8):
        replicanums.append(tuple(fromfile('data/replica-indices/%d.rep' % i,
                                          dtype=int, sep='\n')))

    prdb("replicanums: %dx%d" % (len(replicanums), len(replicanums[0])))
    params = tuple(fromfile('data/temperatures', sep=' '))

    prdb(params)
    indicators = compute_indicators(replicanums, subs=1, start=0)
    prdb("indicators: %dx%d" % (len(indicators), len(indicators[0])))
    prdb("Exchange rate:")
    prdb(array([sum(ind) / float(len(ind)) for ind in indicators]))
    array([sum(ind) / float(len(ind)) for ind in
           indicators]).tofile('xchgs', sep='\n')
    changed, newparams = tune_params(indicators, params, targetAR=0.25,
                                     badMethod="dumb", goodMethod="dumb", CvMethod="skip", testMethod="anova", alpha=0.05)
    if not changed:
        print "Parameter set seems optimal."
    else:
        if not True in [abs(newparams[i + 1] - newparams[i]) < 1e-3 for i in xrange(len(newparams) - 1)]:
            array(newparams).tofile('data/temperatures', sep=' ')
        else:
            print "PROBLEM IN NEW PARAMETERSET -> not saved"
    print "params    :", params
    print "new params:", newparams
