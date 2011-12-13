#!/usr/bin/env python

import sys,os
import argparse
from numpy import *
from scipy.stats import t as student_t

import IMP.isd
import IMP.gsl

from profile import SAXSProfile

class VAction(argparse.Action):
    def __call__(self, parser, args, values, option_string=None):
        if values == None:
            values = '1'
        try:
            values = int(values)
        except ValueError:
            values = values.count('v')+1
        setattr(args, self.dest, values)

def create_parser():
    parser = argparse.ArgumentParser(
            formatter_class=argparse.RawDescriptionHelpFormatter,
            description="Perform a statistical merge of the given SAXS curves",
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
    parser.add_argument('--version', action='version', version='%(prog)s 0.2')
    parser.add_argument('--verbose', '-v', nargs='?', action=VAction,
            dest='verbose', default=1, help="Verbose level. 0 is quiet, 1-3 is "
            "more and more verbose. Default is 1.")
    #general
    group = parser.add_argument_group(title="general")
    group.add_argument('files',
            nargs='+',
            type=str,
            help="a 3-column file that contains SAXS data. To specify the "
                 "number of repetitions of this experiment, use the syntax"
                 " file.txt=20 indicating that data in file.txt is an average"
                 " of 20 experiments. Default is 10. Note that in the case of "
                 "different number of repetitions, the minimum is taken for "
                 "the final fitting step (Step 5).")
    group.add_argument('--mergename', help="filename suffix for output "
            "(default is merged.dat)", default='merged.dat', metavar='SUFFIX')
    group.add_argument('--sumname', metavar='NAME', default='summary.txt',
            help="File to which the merge summary will be written."
            " Default is summary.txt")
    group.add_argument('--destdir', default="./", metavar='DIR',
            help="Destination folder in which files will be written")
    group.add_argument('--header', default=False, action='store_true',
            help="First line of output files is a header (default False)")
    group.add_argument('--outlevel', default='normal', help="Set the output "
            "level, sparse is for q,I,err columns only, normal adds eorigin, "
            "eoriname and eextrapol (default), and full outputs all flags.",
            choices=['normal','sparse','full'])
    group.add_argument('--allfiles', default=False, action='store_true',
            help="Output data files for parsed input files as well (default "
            "is only to output merge and summary files).")
    #cleanup
    group = parser.add_argument_group(title="Cleanup (Step 1)",
                              description="Discard or keep SAXS curves' "
                              "points based on their SNR. Points with an error"
                              " of zero are discarded as well")
    group.add_argument('--aalpha', help='type I error (default 0.001)',
            default=0.001, metavar='ALPHA')
    group.add_argument('--acutoff', help='when a value after CUT is discarded,'
            ' the rest of the curve is discarded as well (default is 0.1)',
            default=0.1, metavar='CUT')
    #fitting
    group = parser.add_argument_group(title="Fitting (Step 2)",
                description="Estimate the mean function and the noise level "
                "of each SAXS curve.")
    group.add_argument('--ba', help='Initial value for a (default -100)',
            default=-100, metavar='A')
    group.add_argument('--bb', help='Initial value for b (default 10)',
            default=10, metavar='B')
    group.add_argument('--btau', help='Initial value for tau (default 10)',
            default=10, metavar='TAU')
    group.add_argument('--blambda', help='Initial value for lambda '
                                        '(default 0.05)',
            default=0.05, metavar='LAMBDA')
    group.add_argument('--bsigma', help='Initial value for sigma (default 10)',
            default=10, metavar='SIGMA')
    group.add_argument('--bschedule', help='Simulation schedule. Default is '
            '"10:10000/5:1000/1:100" which means use every 10 data points for '
            'the first 10000 steps, then every 5 data points for 1000 steps, '
            ' and finally all data points for the last 100 steps.',
            default = "10:10000/5:1000/1:100", metavar="SCHEDULE")
    #rescaling
    group = parser.add_argument_group(title="Rescaling (Step 3)",
                description="Find the most probable scaling factor of all "
                "curves wrt the first curve.")
    group.add_argument('--creference', default='last', help="Define which "
            "input curve the other curves will be recaled to. Options are "
            "first or last (default is last)", choices=['first','last'])
    group.add_argument('--cnormal', action='store_true', default=False,
            help="Use the normal model instead of the lognormal model "
            "to calculate gamma")
    group.add_argument('--cnpoints', default=200, metavar="NUM",
            help="Number of points to use to compute gamma (default 200)")
    #classification
    group = parser.add_argument_group(title="Classification (Step 4)",
                description="Classify the mean curves by comparing them using "
                "a two-sided two-sample student t test")
    group.add_argument('--dalpha', help='type I error (default 0.05)',
            default=0.05, metavar='ALPHA')
    #merging
    group = parser.add_argument_group(title="Merging (Step 5)",
                description="Collect compatible data and produce best estimate "
                "of mean function")
    group.add_argument('--eschedule', help='Simulation schedule, see fitting'
            ' step. (default 10:10000/5:1000/1:100)',
            default = "10:10000/5:1000/1:100", metavar="SCHEDULE")
    group.add_argument('--eextrapolate', metavar="NUM", help='Extrapolate '
            "NUM percent outside of the curve's bounds. Example: if NUM=50 "
            "and the highest acceptable data point is at q=0.3, the mean will "
            "be estimated up to q=0.45. Default is 0 (just extrapolate at low "
            "angle).", default=0)
    return parser

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
    parser = create_parser()
    args = parser.parse_args()
    if args.verbose >= 2 :
        print "Parsing files and creating profile classes"
    filenames,Nreps = parse_filenames(args.files, defaultvalue=10)
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
    for p in profiles:
        if verbose > 1:
            print "   ",p.filename
        N = p.get_Nreps()
        p.new_flag('agood',bool)
        p.new_flag('apvalue',float)
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
            p.set_flag(id, 'apvalue', pval)
    #need a continuous indicator of validity
    for p in profiles:
        create_intervals_from_data(p, 'agood')
    return profiles, args

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
    merge.write_mean(merge.get_filename(), bool_to_int=True, dir=args.destdir,
            qmin=0, qmax=qmax, header=args.header, flags=mflags)
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
