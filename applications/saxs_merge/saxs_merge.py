#!/usr/bin/env python

import sys,os
import optparse
from numpy import *
import copy
from scipy.stats import t as student_t

import IMP
import IMP.isd
import IMP.gsl

class SAXSProfile:

    def __init__(self):
        self.gamma = 1.0
        self.Nreps = 10
        self.flag_names=["q","I","err"]
        self.flag_types=[float,float,float]
        self.flag_dict={"q":0,"I":1,"err":2}
        self.nflags=len(self.flag_names)
        self.data = []
        self.gp = None
        self.particles = {}
        self.intervals = {}
        self.filename = None

    def add_data(self, input, offset=0):
        """add experimental data to saxs profile.
        offset=i means discard first i columns
        """
        if isinstance(input, str):
            #read all lines
            stuff = [i.split() for i in open(input).readlines()]
            #keep the ones that have data
            stuff = filter(lambda a: (len(a)>=self.nflags)
                                      and not a[0].startswith('#'), stuff)
            stuff = map(lambda a:a[:self.nflags+offset], stuff)
        elif isinstance(input, list) or isinstance(input, tuple):
            for i in input:
                if len(i) != self.nflags + offset:
                    raise TypeError, "length of each entry should be %d" % \
                                                    self.nflags
            stuff = input
        else:
            raise ValueError, "Unknown data type"
        data = []
        for a in stuff:
            entry=[]
            for f,z in zip(self.flag_types,a[offset:]):
                if z is None:
                    entry.append(None)
                else:
                    entry.append(f(z))
            data.append(entry)
        self.data += copy.deepcopy(data)
        self.data.sort(key=lambda a:a[0])

    def get_raw_data(self, colwise=False):
        """return original q,I,err values, sorted"""
        if colwise:
            data = self.get_raw_data(colwise=False)
            retval = {}
            retval[self.flag_names[0]] = [d[0] for d in data]
            retval[self.flag_names[1]] = [d[1] for d in data]
            retval[self.flag_names[2]] = [d[2] for d in data]
            return retval
        return [i[:3] for i in self.data]

    def get_data(self, *args, **kwargs):
        """get data, rescaled by self.gamma.
        Returns a list of tuples, each of which is organized as:
            Column 1 : unique id (increasing)
            Column 2 : q (increasing)
            Column 3 : I
            Column 4 : err
            Columns 5+ : flags, as indicated by self.get_flag_names()
        Arguments / Options:
            qmin : specify the starting q value
            qmax : specify the ending q value
                    if they are not specified, take the data's min and max
                    Both have to be specified or none of them.
        Options:
            filter : string or list of strings corresponding to a boolean flag
            name which should be used to filter the data before returning it.
            Data is returned only if all of the provided flags are True.
            colwise : instead of returning a list of tuples for each data point,
                      return a dictionnary with flag names as keys.
        """
        if 'filter' in kwargs:
            filt = kwargs.pop('filter')
        else:
            filt = None
        if filt:
            if isinstance(filt, str):
                flagnos = [self.flag_dict[filt]]
            else:
                flagnos = [self.flag_dict[name] for name in filt]
        else:
            flagnos = []
        if 'colwise' in kwargs:
            colwise = kwargs.pop('colwise')
        else:
            colwise = False
        if len(args) == 0 and len(kwargs) == 0:
            qmin = self.data[0][0]
            qmax = self.data[-1][0]
        elif (len(args) == 2) ^ (len(kwargs) == 2):
            if len(args) == 2:
                try:
                    qmin=float(args[0])
                    qmax=float(args[1])
                except:
                    raise TypeError, "arguments have incorrect type"
                if qmin > qmax:
                    raise ValueError, "qmin > qmax !"
            else:
                if set(kwargs.keys()) != set(['qmax','qmin']):
                    raise TypeError, "incorrect keyword argument(s)"
                qmin=kwargs['qmin']
                qmax=kwargs['qmax']
        else:
            raise TypeError, "provide zero or two arguments (qmin, qmax)"
        if colwise:
            retval=dict.fromkeys(self.get_flag_names()+('id',))
            for i in retval:
                retval[i] = []
        else:
            retval = []
        for i,d in enumerate(self.data):
            if d[0] < qmin:
                continue
            if d[0] > qmax:
                break
            if len(flagnos) > 0 and False in [d[k] is True for k in flagnos]:
                continue
            cd = copy.copy(d)
            cd[1]=self.gamma*cd[1]
            cd[2]=self.gamma*cd[2]
            if colwise:
                retval['id'].append(i)
                for k in self.get_flag_names():
                    retval[k].append(cd[self.flag_dict[k]])
            else:
                retval.append(tuple([i,]+cd))
        return retval

    def get_gamma(self):
        return self.gamma

    def set_gamma(self, gamma):
        self.gamma = gamma

    def new_flag(self, name, tp):
        """add extra flag to all data points. Initialized to None"""
        self.flag_names.append(name)
        self.flag_types.append(tp)
        self.flag_dict[name] = self.nflags
        self.nflags += 1
        for d in self.data:
            d.append(None)
        self.intervals[name]=[]

    def set_flag(self, id, name, value):
        "set the flag of a data point"
        flagno = self.flag_dict[name]
        flagtype = self.flag_types[flagno]
        try:
            converted = flagtype(value)
        except:
            raise TypeError, "unable to cast given value to %s" % flagtype
        self.data[id][flagno]=converted

    def get_flag(self, val, flag, default = '---'):
        """get the flag of a point in the profile
        call with (id, flag) to get the flag of data point id
        call with (qvalue, flag) to get the flag of an unobserved data point.
                                Uses the flag intervals for this.
        If the flag is not defined for that point, return default
        """
        if isinstance(val, int):
            try:
                retval = self.data[val][self.flag_dict[flag]]
                if retval is None:
                    raise ValueError
            except ValueError:
                return default
            return retval
        elif not isinstance(val, float):
            raise ValueError, "first argument must be int or float"
        if not flag in self.get_flag_names():
            raise KeyError, "flag %s does not exist!" % flag
        for qmin,qmax,flagval in self.get_flag_intervals(flag):
            if qmin <= val <= qmax:
                return flagval
        return default

    def set_interpolant(self, gp, particles, model):
        """store a class that gives interpolated values,
        usually an instance of the GaussianProcessInterpolation
        """
        if not hasattr(gp, "get_posterior_mean"):
            raise TypeError, "interpolant.get_posterior_mean does not exist"
        if not hasattr(gp, "get_posterior_covariance"):
            raise TypeError, "interpolant.get_posterior_covariance "\
                                "does not exist"
        if not isinstance(particles, dict):
            raise TypeError, "second argument should be dict"
        for p in particles.values():
            if not IMP.isd.Nuisance.particle_is_instance(p):
                raise TypeError, "particles should all be ISD Nuisances"
        if not isinstance(model, IMP.Model):
            raise TypeError, "third argument is expected to be an IMP.Model()"
        self.gp = gp
        self.particles = particles
        self.model = model

    def set_flag_interval(self, flag, qmin, qmax, value):
        if flag not in self.flag_names:
            raise ValueError, "unknown flag %s" % flag
        intervals = []
        for imin,imax,ival in self.intervals[flag]:
            #add non-overlapping intervals
            if imax < qmin or imin > qmax:
                intervals.append((imin,imax,ival))
            else:
                if value == ival:
                    #merge intervals with same value
                    #no need to start over since all other intervals
                    #don't overlap
                    qmin = min(qmin,imin)
                    qmax = max(qmax,imax)
                else:
                    #discard new portion that disagrees
                    if imin < qmin and imax < qmax:
                        #discard qmin -> imax
                        imax = qmin
                        intervals.append((imin,imax,ival))
                    elif imin > qmin and imax > qmax:
                        #discard imin -> qmax
                        imin = qmax
                        intervals.append((imin,imax,ival))
                    elif imin < qmin and imax > qmax:
                        #split interval in two
                        intervals.append((imin,qmin,ival))
                        intervals.append((qmax,imax,ival))
                    else:
                        #just discard old interval
                        pass
        #NoneType is accepted, but otherwise cast to target value
        if value is None:
            newinterval = (qmin,qmax,None)
        else:
            newinterval = (qmin,qmax,
                                self.flag_types[self.flag_dict[flag]](value))
        intervals.append(newinterval)
        #sort by ascending qmin value
        intervals.sort(key=lambda a:a[0])
        self.intervals[flag]=intervals

    def get_flag_intervals(self, flag):
        """returns a list of [qmin, qmax, flag_value] lists"""
        return self.intervals[flag]

    def get_mean(self, **kwargs):
        """returns (q,I,err,...) tuples for num points (default 200) ranging
        from qmin to qmax (whole data range by default)
        possible keyword arguments:
            num qmin qmax filter
        the length of the returned tuple is the same as the number of flags.
        You can also give specific values via the qvalues keyword argument.
        see get_data for details on the filter argument. If num is used with
        filter, the function returns at most num elements and possibly none.
        """
        rem = set(kwargs.keys()) \
                - set(['qvalues','num','qmin','qmax','filter','colwise'])
        if len(rem) > 0:
            raise TypeError, "Unknown keyword(s): %s" % list(rem)
        #
        if 'qvalues' in kwargs:
            qvals = kwargs['qvalues']
        else:
            if 'num' in kwargs:
                num = kwargs['num']
            else:
                num = 200
            if 'qmin' in kwargs:
                qmin = kwargs['qmin']
            else:
                qmin = self.data[0][0]
            if 'qmax' in kwargs:
                qmax = kwargs['qmax']
            else:
                qmax = self.data[-1][0]
            qvals = linspace(qmin,qmax,num)
        #
        if 'colwise' in kwargs:
            colwise = kwargs['colwise']
        else:
            colwise = False
        #
        if 'filter' in kwargs:
            filt = kwargs.pop('filter')
        else:
            filt = None
        if filt:
            if isinstance(filt, str):
                flagnos = [self.flag_dict[filt]]
            else:
                flagnos = [self.flag_dict[name] for name in filt]
        else:
            flagnos = []
        flagnames = self.get_flag_names()
        flags = []
        gamma = self.get_gamma()
        if colwise:
            retval=dict.fromkeys(self.get_flag_names())
            for i in retval:
                retval[i] = []
        else:
            retval=[]
        for q in qvals:
            if len(flagnames) > 3:
                flags = map(self.get_flag, [q]*len(flagnames[3:]),
                                           flagnames[3:])
                if False in [flags[i-3] for i in flagnos]:
                    continue
            thing = [q,gamma*self.gp.get_posterior_mean([q]),
                    gamma*sqrt(self.gp.get_posterior_covariance([q],[q]))]
            if flags:
                thing.extend(flags)
            if colwise:
                for flag in self.get_flag_names():
                    retval[flag].append(thing[self.flag_dict[flag]])
            else:
                retval.append(tuple(thing))
        return retval

    def get_flag_names(self):
        return tuple(self.flag_names)

    def get_params(self):
        retval={}
        for k,v in self.particles.items():
            retval[k]=v.get_nuisance()
        return retval

    def set_Nreps(self,nreps):
        self.Nreps = nreps

    def get_Nreps(self):
        return self.Nreps

    def write_data(self, filename, prefix='data_', suffix='', sep=' ',
            bool_to_int=False, dir='./', header=True, flags=None,
            float_fmt='%15f', *args, **kwargs):
        fl=open(os.path.join(dir,prefix+filename+suffix),'w')
        allflags = self.get_flag_names()
        if flags == None:
            flags=allflags
        if header:
            fl.write('#')
            for i,name in enumerate(flags):
                fl.write("%d:%s%s" % (i+1,name,sep))
            fl.write('\n')
        for d in self.get_data(*args, **kwargs):
            for flagname,ent in zip(allflags,d[1:]):
                if flagname not in flags:
                    continue
                if bool_to_int and isinstance(ent, bool):
                    fl.write('%d' % ent)
                elif isinstance(ent, float):
                    fl.write(float_fmt % ent)
                else:
                    fl.write('%s' % ent)
                fl.write(sep)
            fl.write('\n')
        fl.close()

    def write_mean(self, filename, prefix='mean_', suffix='', sep=' ',
            bool_to_int=False, dir='./', header=True, flags=None,
            float_fmt="%15f", *args, **kwargs):
        fl=open(os.path.join(dir,prefix+filename+suffix),'w')
        allflags = self.get_flag_names()
        if flags == None:
            flags=allflags
        if header:
            fl.write('#')
            for i,name in enumerate(flags):
                fl.write("%d:%s%s" % (i+1,name,sep))
            fl.write('\n')
        for d in self.get_mean(*args, **kwargs):
            for flagname,ent in zip(allflags,d):
                if flagname not in flags:
                    continue
                if bool_to_int and isinstance(ent, bool):
                    fl.write('%d' % ent)
                elif isinstance(ent, float):
                    fl.write(float_fmt % ent)
                else:
                    fl.write('%s' % ent)
                fl.write(sep)
            fl.write('\n')
        fl.close()

    def set_filename(self, fname):
        self.filename = fname

    def get_filename(self):
        return self.filename


class _RawEpilogFormatter(optparse.IndentedHelpFormatter):
    """Output the epilog help text as-is"""
    def format_epilog(self, epilog):
        return epilog


def parse_args():
    parser = optparse.OptionParser(
            description="Perform a statistical merge of the given SAXS curves"
                  "\n\nfile is a 3-column file that contains SAXS data. "
                  "To specify the number of repetitions of this experiment, "
                  "use the syntax file.txt=20 indicating that data"
                  "in file.txt is an average of 20 experiments. Default is "
                  "10. Note that in the case of different number of "
                  "repetitions, the minimum is taken for the final "
                  "fitting step (Step 5).",
            formatter=_RawEpilogFormatter(),
            usage="%prog [options] file [file ...]",
            version='%prog 0.2',
            epilog="""Output file legend:\n
Cleanup
    agood     (bool)   : True if SNR is high enough
    apvalue   (float)  : p-value of the student t test
Rescaling
    cgood     (bool)   : True if data point is both valid (wrt SNR) and in the
                         validity domain of the gamma reference curve (the last
                         curve)
Classification
    drefnum   (int)    : number of the reference profile for this point
    drefname  (string) : associated filename
    dgood     (bool)   : True if this point is compatible with the reference and
                         False otherwise. Undefined if 'agood' is False for that
                         point.
    dselfref  (bool)   : True if this curve was it's own reference, in which
                         case dgood is also True
    dpvalue   (float)  : p-value for the classification test
Merging
    eorigin   (int)    : profile index from which this point originates
    eoriname  (string) : associated filename
    eextrapol (bool)   : True if mean function is being extrapolated.
""")
    parser.add_option('--verbose', '-v', action="count",
            dest='verbose', default=0,
            help="Increase verbosity. Can be repeated up to 3 times "
                 "for more output.")
    #general
    group = optparse.OptionGroup(parser, title="general")
    parser.add_option_group(group)
    group.add_option('--mergename', help="filename suffix for output "
            "(default is merged.dat)", default='merged.dat', metavar='SUFFIX')
    group.add_option('--sumname', metavar='NAME', default='summary.txt',
            help="File to which the merge summary will be written."
            " Default is summary.txt")
    group.add_option('--destdir', default="./", metavar='DIR',
            help="Destination folder in which files will be written")
    group.add_option('--header', default=False, action='store_true',
            help="First line of output files is a header (default False)")
    group.add_option('--outlevel', default='normal', help="Set the output "
            "level, sparse is for q,I,err columns only, normal adds eorigin, "
            "eoriname and eextrapol (default), and full outputs all flags.",
            type="choice", choices=['normal','sparse','full'])
    group.add_option('--allfiles', default=False, action='store_true',
            help="Output data files for parsed input files as well (default "
            "is only to output merge and summary files).")
    #cleanup
    group = optparse.OptionGroup(parser, title="Cleanup (Step 1)",
                              description="Discard or keep SAXS curves' "
                              "points based on their SNR. Points with an error"
                              " of zero are discarded as well")
    parser.add_option_group(group)

    group.add_option('--aalpha', help='type I error (default 1e-7)',
                     type="float", default=1e-7, metavar='ALPHA')
    group.add_option('--acutoff', help='when a value after CUT is discarded,'
            ' the rest of the curve is discarded as well (default is 0.1)',
            type="float", default=0.1, metavar='CUT')
    #fitting
    group = optparse.OptionGroup(parser, title="Fitting (Step 2)",
                description="Estimate the mean function and the noise level "
                "of each SAXS curve.")
    parser.add_option_group(group)
    group.add_option('--ba', help='Initial value for a (default -100)',
                     type="int", default=-100, metavar='A')
    group.add_option('--bb', help='Initial value for b (default 10)',
                     type="int", default=10, metavar='B')
    group.add_option('--btau', help='Initial value for tau (default 10)',
                     type="int", default=10, metavar='TAU')
    group.add_option('--blambda', help='Initial value for lambda '
                                        '(default 0.05)',
                     type="float", default=0.05, metavar='LAMBDA')
    group.add_option('--bsigma', help='Initial value for sigma (default 10)',
                     type="int", default=10, metavar='SIGMA')
    group.add_option('--bschedule', help='Simulation schedule. Default is '
            '"10:1000/5:100/1:10" which means use every 10 data points for '
            'the first 1000 steps, then every 5 data points for 100 steps, '
            ' and finally all data points for the last 10 steps.',
            default = "10:1000/5:100/1:10", metavar="SCHEDULE")
    #rescaling
    group = optparse.OptionGroup(parser, title="Rescaling (Step 3)",
                description="Find the most probable scaling factor of all "
                "curves wrt the first curve.")
    parser.add_option_group(group)
    group.add_option('--creference', default='last', help="Define which "
            "input curve the other curves will be recaled to. Options are "
            "first or last (default is last)", type="choice",
            choices=['first','last'])
    group.add_option('--cnormal', action='store_true', default=False,
            help="Use the normal model instead of the lognormal model "
            "to calculate gamma")
    group.add_option('--cnpoints', type="int", default=200, metavar="NUM",
            help="Number of points to use to compute gamma (default 200)")
    #classification
    group = optparse.OptionGroup(parser, title="Classification (Step 4)",
                description="Classify the mean curves by comparing them using "
                "a two-sided two-sample student t test")
    parser.add_option_group(group)
    group.add_option('--dalpha', help='type I error (default 0.05)',
                     type="float", default=0.05, metavar='ALPHA')
    #merging
    group = optparse.OptionGroup(parser, title="Merging (Step 5)",
                description="Collect compatible data and produce best estimate "
                "of mean function")
    parser.add_option_group(group)
    group.add_option('--eschedule', help='Simulation schedule, see fitting'
            ' step. (default 10:1000/5:100/1:10)',
            default = "10:1000/5:100/1:10", metavar="SCHEDULE")
    group.add_option('--eextrapolate', metavar="NUM", help='Extrapolate '
            "NUM percent outside of the curve's bounds. Example: if NUM=50 "
            "and the highest acceptable data point is at q=0.3, the mean will "
            "be estimated up to q=0.45. Default is 0 (just extrapolate at low "
            "angle).", type="int", default=0)
    group.add_option('--enoextrapolate', action='store_true', default=False,
            help="Don't extrapolate at all, even at low angle (default False)")
    (args, files) = parser.parse_args()
    if len(files) == 0:
        parser.error("No files specified")
    return (args, files)

def parse_filenames(fnames, defaultvalue=10):
    files = []
    Nreps = []
    for fname in fnames:
        if '=' in fname:
            tokens = fname.split()
            files.append(tokens[0])
            Nreps.append(int(tokens[1]))
        else:
            files.append(fname)
            Nreps.append(defaultvalue)
    return files, Nreps

def create_profile(file, nreps):
    p=SAXSProfile()
    p.add_data(file)
    p.set_Nreps(nreps)
    p.set_filename(file)
    return p

def parse_schedule(schedstr):
    #list of tuples (subsampling, nsteps)
    return [map(int, i.split(':')) for i in schedstr.split('/')]

def ttest_one(mu,s,n):
    """one-sample right-tailed t-test against 0
    Returns: pval, t, nu
    """
    v = s**2/float(n)
    t = mu/sqrt(v)
    nu = n-1
    pval = (1-student_t.cdf(t,nu))
    return pval, t, nu

def ttest_two(mu1,s1,n1,mu2,s2,n2):
    """Welch's t-test
    two-sample two-sided t-test
    Returns: pval, t, nu
    """
    v1 = s1**2/float(n1)
    v2 = s2**2/float(n2)
    t = (mu1-mu2)/sqrt(v1+v2)
    nu = (v1+v2)**2/(v1**2/(n1-1)+v2**2/(n2-1))
    pval = 2*(1-student_t.cdf(abs(t),nu))
    return pval, t, nu

def setup_particles(initvals):
    """ initvals : dict of initial values for the parameters
    returns: model, dict(a, b, sigma, lambda, tau), dict(mean, covariance)
    """

    model = IMP.Model()
    #mean function
    a=IMP.isd.Nuisance.setup_particle(IMP.Particle(model),initvals['a'])
    b=IMP.isd.Nuisance.setup_particle(IMP.Particle(model),initvals['b'])
    m = IMP.isd.Linear1DFunction(a,b)
    #covariance function
    tau=IMP.isd.Scale.setup_particle(IMP.Particle(model),initvals['tau'])
    lam=IMP.isd.Scale.setup_particle(IMP.Particle(model),initvals['lambda'])
    w = IMP.isd.Covariance1DFunction(tau,lam,2.0)
    #sigma
    sigma=IMP.isd.Scale.setup_particle(IMP.Particle(model),initvals['sigma'])
    #prior on scales
    model.add_restraint(IMP.isd.JeffreysRestraint(lam))
    lam.set_lower(0.001)
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,lam))
    model.add_restraint(IMP.isd.JeffreysRestraint(tau))
    tau.set_lower(1)
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,tau))
    model.add_restraint(IMP.isd.JeffreysRestraint(sigma))
    sigma.set_lower(1)
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,sigma))
    #
    a.set_is_optimized(a.get_nuisance_key(),True)
    b.set_is_optimized(b.get_nuisance_key(),True)
    tau.set_is_optimized(tau.get_nuisance_key(),True)
    lam.set_is_optimized(lam.get_nuisance_key(),True)
    sigma.set_is_optimized(sigma.get_nuisance_key(),True)
    #
    particles = {}
    particles['a'] = a
    particles['b'] = b
    particles['tau'] = tau
    particles['lambda'] = lam
    particles['sigma'] = sigma
    functions = {}
    functions['mean'] = m
    functions['covariance'] = w
    return model, particles, functions

def do_quasinewton(model,nsteps):
    qn=IMP.gsl.QuasiNewton(model)
    #fl=open('qn.txt','w')
    #write_header(fl)
    #write_params(fl,model,a,b,tau,lam,sigma)
    #if print_steps >0:
    #    for i in xrange(nsteps):
    #        if print_steps >0 and i % print_steps == 0 :
    #            print i,
    #            sys.stdout.flush()
    #        IMP.set_log_level(IMP.TERSE)
    #        qn.optimize(1)
    #        IMP.set_log_level(0)
    #        write_params(fl,model,a,b,tau,lam,sigma)
    #else:
    qn.optimize(nsteps)

def setup_process(data,initvals, subs):
    model,particles,functions = setup_particles(initvals)
    q = [ [i] for i in data['q'][::subs]]
    I = data['I'][::subs]
    err = data['err'][::subs]
    gp = IMP.isd.GaussianProcessInterpolation(q, I, err,
            data['N'], functions['mean'], functions['covariance'],
            particles['sigma'])
    return model, particles, gp

def fitting_step(data, initvals, subs, nsteps):
    model, particles, gp = setup_process(data, initvals, subs)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(gp)
    model.add_restraint(gpr)
    do_quasinewton(model,nsteps)
    return dict([(k,v.get_nuisance()) for (k,v) in particles.iteritems()])

def create_intervals_from_data(profile, flag):
    """This function creates intervals for the mean function so that
    the flag for the mean is consistent with that of the data.
    example: q1:True, q2:False, q3:True q4:True
    creates [q1:q2[ (True), [q2:q3[ (False) and [q3:q4[ (True)
    """
    if flag not in profile.get_flag_names():
        raise KeyError, "No such flag: %s" % flag
    data = profile.get_data(colwise=True)
    new=True
    interval=[]
    oldb = None
    oldval = None
    for q,b in zip(data['q'],data[flag]):
        if new:
            new = False
            oldval = q
            oldb = b
        elif b != oldb:
            profile.set_flag_interval(flag, oldval, q, oldb)
            oldval = q
            oldb = b
    if q != oldval:
        profile.set_flag_interval(flag, oldval, q, oldb)

def get_gamma_lognormal(refdata,data):
    I0=array(refdata['I'])
    s0=array(refdata['err'])
    I1=array(data['I'])
    s1=array(data['err'])
    errors=(s0/I0 + s1/I1)
    weights = errors**(-2)
    lg = (weights*log(I0/I1)).sum()/weights.sum()
    return exp(lg)

def get_gamma_normal(refdata,data):
    I0=array(refdata['I'])
    s0=array(refdata['err'])
    I1=array(data['I'])
    s1=array(data['err'])
    weights = (s0**2+s1**2)**(-1)
    return (weights*I0/I1).sum()/weights.sum()

def initialize():
    args, files = parse_args()
    if args.verbose >= 2 :
        print "Parsing files and creating profile classes"
    filenames,Nreps = parse_filenames(files, defaultvalue=10)
    args.filenames = filenames
    args.Nreps = Nreps
    profiles = map(create_profile, filenames, Nreps)
    args.bschedule = parse_schedule(args.bschedule)
    args.eschedule = parse_schedule(args.eschedule)
    return profiles, args

def cleanup(profiles, args):
    """first stage of merge: discard low SNR data
    Created flags:
        agood : True if SNR is high enough
        apvalue : p-value of the student t test
    """
    verbose = args.verbose
    alpha = args.aalpha
    q_cutoff = args.acutoff
    if verbose >0:
        print "1. cleanup"
    #loop over profiles
    good_profiles=[]
    for p in profiles:
        if verbose > 1:
            print "   ",p.filename
        N = p.get_Nreps()
        p.new_flag('agood',bool)
        p.new_flag('apvalue',float)
        all_points_bad = True
        #loop over individual points
        had_outlier = False
        for datum in p.get_data():
            id,q,I,err = datum[:4]
            if err == 0:
                p.set_flag(id,'agood', False)
                p.set_flag(id, 'apvalue', -1)
                continue
            pval,t = ttest_one(I,err,N)[0:2]
            if pval > alpha or had_outlier:  #the point is invalid
                p.set_flag(id, 'agood', False)
                if q >= q_cutoff and had_outlier == False:
                    had_outlier = True
            else:
                p.set_flag(id, 'agood', True)
                if all_points_bad:
                    all_points_bad = False
            p.set_flag(id, 'apvalue', pval)
        if all_points_bad:
            print "Warning: all points in file %s have been discarded"\
                    " on cleanup" % p.filename
        else:
            good_profiles.append(p)
    #need a continuous indicator of validity
    for p in good_profiles:
        create_intervals_from_data(p, 'agood')
    return good_profiles, args

def fitting(profiles, args):
    """second stage of merge: gp fitting
    sets and optimizes the interpolant and enables calls to get_mean()
    """
    schedule = args.bschedule
    verbose = args.verbose
    if verbose >0:
        print "2. fitting"
    for p in profiles:
        if verbose > 1:
            print "   ",p.filename,
        data = p.get_data(filter='agood',colwise=True)
        data['N'] = p.get_Nreps()
        initvals={}
        initvals['a']=args.ba
        initvals['b']=args.bb
        initvals['tau']=args.btau
        initvals['lambda']=args.blambda
        initvals['sigma']=args.bsigma
        for subs,nsteps in schedule:
            if verbose >2:
                print " %d:%d" % (subs,nsteps),
                sys.stdout.flush()
            initvals = fitting_step(data,initvals,subs,nsteps)
        model, particles, gp = setup_process(data,initvals,1)
        p.set_interpolant(gp, particles, model)
        if verbose > 1:
            print ""
    return profiles, args

def rescaling(profiles, args):
    """third stage of merge: rescaling
    Created flag:
        cgood : True if data point is both valid (wrt SNR) and in the validity
                domain of the rescaling reference curve (option --creference)
    sets profile.gamma to the correct value
    """
    use_normal = args.cnormal
    numpoints = args.cnpoints
    verbose = args.verbose
    reference = args.creference
    if verbose >0:
        print "3. rescaling"
    #take last as internal reference as there's usually good overlap
    pref = profiles[-1]
    gammas = []
    for p in profiles:
        #find intervals where both functions are valid
        p.new_flag('cgood',bool)
        pdata = p.get_data(colwise=True)
        for id,q,flag in zip(pdata['id'],pdata['q'],pdata['agood']):
            refflag = pref.get_flag(q,'agood')
            p.set_flag(id, 'cgood', flag and refflag)
        create_intervals_from_data(p, 'cgood')
        #generate points in these intervals to compute gamma
        goodip = [i for i in p.get_flag_intervals('cgood') if i[2]]
        totaldist = array([i[1]-i[0] for i in goodip]).sum()
        qvalues = []
        for interval in goodip:
            qmin = interval[0]
            qmax = interval[1]
            dist = qmax - qmin
            numint = int(round(dist/totaldist*numpoints))
            if numint <= 1:
                continue
            for i in xrange(numint):
                qvalues.append((float(i)/(numint-1))*dist + qmin)
        pvalues = p.get_mean(qvalues=qvalues, colwise=True)
        prefvalues = pref.get_mean(qvalues=qvalues, colwise=True)
        if use_normal:
            gamma = get_gamma_normal(prefvalues, pvalues)
        else:
            gamma = get_gamma_lognormal(prefvalues, pvalues)
        gammas.append(gamma)
    #set gammas wrt reference
    if reference == 'first':
        gr=gammas[0]
    else:
        gr = gammas[-1]
    for p,g in zip(profiles,gammas):
        gamma = g/gr
        p.set_gamma(gamma)
        if verbose >1:
            print "   ",p.filename,"   ",gamma
    return profiles,args

def classification(profiles, args):
    """fourth stage of merge: classify mean functions
    Created flags:
        drefnum : number of the reference profile for this point
        drefname : associated filename
        dgood : True if this point is compatible with the reference
                and False otherwise. Undefined if 'agood' is False for
                that point.
        dselfref : True if this curve was it's own reference, in which case
                dgood is also True
        dpvalue : p-value for the classification test
    """
    alpha = args.dalpha
    verbose = args.verbose
    if verbose >0:
        print "4. classification"
    for i in xrange(len(profiles)):
        p = profiles[i]
        if verbose >1:
            print "   ",p.filename
        p.new_flag('drefnum',int)
        p.new_flag('drefname',str)
        p.new_flag('dgood',bool)
        p.new_flag('dselfref',bool)
        p.new_flag('dpvalue',float)
        N = p.get_Nreps()
        for entry in p.get_data(filter='agood'):
            #find out who is the reference here
            for refnum,pref in enumerate(profiles[:i+1]):
                if pref.get_flag(entry[1],'agood',default=None):
                    break
            p.set_flag(entry[0], 'drefnum', refnum)
            p.set_flag(entry[0], 'drefname', pref.get_filename())
            p.set_flag(entry[0], 'dselfref', pref is p)
            #do classification
            data = p.get_mean(qvalues=[entry[1]],colwise=True)
            refdata = pref.get_mean(qvalues=[entry[1]],colwise=True)
            Nref = pref.get_Nreps()
            pval, t, nu = ttest_two(data['I'][0],data['err'][0],N,
                                    refdata['I'][0],refdata['err'][0],Nref)
            p.set_flag(entry[0], 'dgood', pval >= alpha)
            p.set_flag(entry[0], 'dpvalue', pval)
        create_intervals_from_data(p, 'drefnum')
        create_intervals_from_data(p, 'drefname')
        create_intervals_from_data(p, 'dselfref')
        create_intervals_from_data(p, 'dgood')
    return profiles, args

def merging(profiles, args):
    """last stage of merge: collect valid data points and fit them
    Creates a profile that contains all compatible data points. This profile has
    the following flags:
        dselfref, drefnum, drefname (see above)
        eorigin : profile index from which this point originates
        eoriname : associated filename
    it then sets and optimizes the interpolant and enables calls to get_mean()
    """
    schedule = args.eschedule
    verbose = args.verbose
    do_extrapolation = not args.enoextrapolate
    extrapolate = 1+args.eextrapolate/float(100)
    if verbose > 0:
        print "5. merging"
        print "   gathering data"
    merge = SAXSProfile()
    merge.set_filename(args.mergename)
    merge.new_flag('dselfref',bool)
    merge.new_flag('drefnum',int)
    merge.new_flag('drefname',str)
    merge.new_flag('eorigin',int)
    merge.new_flag('eoriname',str)
    merge.new_flag('eextrapol',bool)
    flags_to_keep=['q','I','err', 'dselfref','drefnum','drefname']
    #loop over profiles and add data
    for i,p in enumerate(profiles):
        if verbose >1:
            print "      ",p.filename
        #get data and keep only relevant flags
        data = p.get_data(filter='dgood',colwise=True)
        #flag_numbers = [p.flag_dict[k]+1 for k in flags_to_keep]
        #print len(flag_numbers)
        #cleaned = [[d for j,d in enumerate(dat) if j in flag_numbers]
        #            + [i,p.filename] for dat in data]
        #print cleaned
        for k in data.keys():
            if k not in flags_to_keep:
                data.pop(k)
        data['eorigin']=[i]*len(data['q'])
        data['eoriname']=[p.filename]*len(data['q'])
        data['eextrapol']=[False]*len(data['q'])
        cleaned = []
        all_flags = flags_to_keep + ['eorigin','eoriname','eextrapol']
        for j in xrange(len(data['q'])):
            entry=[]
            for k in all_flags:
                entry.append(data[k][j])
            cleaned.append(entry)
        merge.add_data(cleaned)
    if verbose >0:
        print "   calculating merged mean"
    #recompute all intervals
    for n,t in zip(merge.flag_names,merge.flag_types):
        if t != float and n not in ['q','I','err']:
            create_intervals_from_data(merge, n)
    #create interval for extrapolation
    data = merge.get_data(colwise=True)['q']
    merge.set_flag_interval('eextrapol', min(data), max(data), False)
    if do_extrapolation:
        merge.set_flag_interval('eextrapol',0,min(data),True)
        if args.eextrapolate > 0:
            merge.set_flag_interval('eextrapol',
                    max(data), max(data)*extrapolate, True)
    #set Nreps to min of all
    #its the bet we can do when fitting all points simultaneously
    merge.set_Nreps(min([p.get_Nreps() for p in profiles]))
    #fit curve
    data = merge.get_data(colwise=True)
    data['N'] = merge.get_Nreps()
    #take initial values from the curve which has gamma == 1
    initvals = profiles[-1].get_params()
    if verbose > 2:
        print "     ",
    for subs,nsteps in schedule:
        if verbose >2:
            print " %d:%d" % (subs,nsteps),
            sys.stdout.flush()
        initvals = fitting_step(data,initvals,subs,nsteps)
    model, particles, gp = setup_process(data,initvals,1)
    merge.set_interpolant(gp, particles, model)
    if verbose > 2:
        print ""
    return merge, profiles, args

def write_data(merge, profiles, args):
    if args.verbose > 0:
        print "writing data"
    if not os.path.isdir(args.destdir):
        os.mkdir(args.destdir)
    #individual profiles
    if args.allfiles:
        for i in profiles:
            destname = os.path.basename(i.get_filename())
            i.write_data(destname, bool_to_int=True, dir=args.destdir,
                    header=args.header)
            i.write_mean(destname, bool_to_int=True, dir=args.destdir,
                    header=args.header)
    #merge profile
    if args.outlevel == 'sparse':
        dflags = ['q','I','err']
        mflags = ['q','I','err']
    elif args.outlevel == 'normal':
        dflags = ['q','I','err','eorigin','eoriname']
        mflags = ['q','I','err','eorigin','eoriname','eextrapol']
    else:
        dflags = None
        mflags = None
    merge.write_data(merge.get_filename(), bool_to_int=True, dir=args.destdir,
            header=args.header, flags=dflags)
    qmax = max([i[1] for i in merge.get_flag_intervals('eextrapol')])
    if args.enoextrapolate:
        qmin = min([i[0] for i in merge.get_flag_intervals('eextrapol')])
    else:
        qmin=0
    merge.write_mean(merge.get_filename(), bool_to_int=True, dir=args.destdir,
            qmin=qmin, qmax=qmax, header=args.header, flags=mflags)
    #summary
    fl=open(os.path.join(args.destdir,args.sumname),'w')
    fl.write("#STATISTICAL MERGE: SUMMARY\n\n")
    fl.write("Ran with the following arguments:\n")
    fl.write(os.path.basename(sys.argv[0]) + " ".join(sys.argv[1:]) + "\n\n")
    fl.write("Merge file\n"
             "  General\n"
             "   Filename: " + merge.filename + "\n")
    data = merge.get_data(colwise=True)
    fl.write("   Number of points: %d\n" % len(data['q']) +
             "   Data range: %.5f %.5f\n" % (data['q'][0],data['q'][-1]))
    for i in xrange(len(profiles)):
        fl.write("   %d points from profile %d (%s)\n" %
                   (len([1 for k in data['eorigin'] if k == i]),
                       i, profiles[i].filename))
    fl.write("  Gaussian Process parameters\n")
    data = merge.get_params()
    for i in sorted(data.keys()):
        fl.write("   %s : %f\n" % (i,data[i]))
    fl.write("\n")
    for i,p in enumerate(profiles):
        fl.write("Input file %d\n" % i +
                 "  General\n" +
                 "   Filename: " + p.filename + "\n")
        data = p.get_raw_data()
        fl.write("   Number of points: %d \n" % len(data) +
                 "   Data range: %.5f %.5f \n" % (data[0][0],data[-1][0]))
        data = p.get_data(filter='agood',colwise=True)
        fl.write("  1. Cleanup\n" +
                 "   Number of significant points: %d \n" % len(data['q']) +
                 "   Data range: %.5f %.5f \n" % (data['q'][0],data['q'][-1]))
        data = p.get_params()
        fl.write("  2. GP parameters (values for non-rescaled curve)\n")
        for i in sorted(data.keys()):
            fl.write("   %s : %f\n" % (i,data[i]))
        data = p.get_gamma()
        fl.write("  3. Rescaling\n")
        fl.write("   gamma : %f\n" % data)
        data = p.get_data(filter='dgood',colwise=True)
        fl.write("  4. Classification\n" +
                 "   Number of valid points: %d \n" % len(data['q']) +
                 "   Data range: %.5f %.5f \n" % (data['q'][0],data['q'][-1]))
        fl.write("\n")
    if args.verbose > 0:
        print "Done."

def main():
    profiles, args = initialize()
    profiles, args = cleanup(profiles, args)
    profiles, args = fitting(profiles, args)
    profiles, args = rescaling(profiles, args)
    profiles, args = classification(profiles, args)
    merge, profiles, args = merging(profiles, args)
    write_data(merge, profiles, args)

if __name__ == "__main__":
    main()
