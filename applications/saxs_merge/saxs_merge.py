#!/usr/bin/env python

import sys,os
import optparse
import fractions
from random import sample
from numpy import *
import copy
from scipy.stats import t as student_t
import glob
import tempfile

import IMP
import IMP.isd
import IMP.gsl
import IMP.saxs

IMP.base.set_log_level(0)
#fitno=0

def subsample(idx, data, npoints):
    """keep min(npoints,len(data)) out of data if npoints>0,
    subsampling evenly along the idx (float) coordinate
    """
    #defined here because it's used in the class
    Ntot = len(data)
    if len(idx) != Ntot:
        raise ValueError
    if npoints >= Ntot or npoints <= 0:
        return data
    all = zip(idx,data)
    all.sort()
    qvals = array([i[0] for i in all])
    newdata=[]
    i=0
    qmin = min(qvals)
    qmax = max(qvals)
    for q in linspace(qmin,qmax,num=npoints):
        if q==qmax:
            i=Ntot-1
        else:
            while qvals[i] <= q:
                i += 1
            i -= 1
        newdata.append(all[i][1])
    return newdata

class FittingError(Exception):
    pass

class SAXSProfile:

    def __init__(self):
        self.gamma = 1.0
        self.offset = 0.0
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

    def add_data(self, input, offset=0, positive=False, err=True, scale=1):
        """add experimental data to saxs profile.
        offset=i means discard first i columns
        positive=True means only keep intensities that are >0
        err=True means keep only points that have >0 error bar
        scale is a factor by which to multiply input I and err
        """
        if isinstance(input, str):
            #read all lines
            stuff = [i.split() for i in open(input).readlines()]
            #keep the ones that have data
            stuff = filter(lambda a: (len(a)>=self.nflags)
                                      and not a[0].startswith('#'), stuff)
            #drop offset, and first 3 columns must be of float type and not nan
            stuff = map(lambda a:a[:self.nflags+offset], stuff)
            s2=[]
            for s in stuff:
                try:
                    map(float,s[:3])
                except ValueError:
                    continue
                if 'nan' in s[:3]:
                    continue
                s2.append(s)
            stuff=s2
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
            #keep positive data
            if positive and entry[1] <= 0:
                continue
            #keep weighted points
            if err and entry[2] <= 0:
                continue
            #multiply I and err by scale
            entry[1] *= scale
            entry[2] *= scale
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
        """get data, rescaled by self.gamma and self.offset
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
            maxpoints : only return at most N points, subsampling if necessary.
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
        maxpoints=-1
        if 'maxpoints' in kwargs:
            maxpoints=kwargs.pop('maxpoints')
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
            cd[1]=self.gamma*(cd[1]+self.offset)
            cd[2]=self.gamma*cd[2]
            if colwise:
                retval['id'].append(i)
                for k in self.get_flag_names():
                    retval[k].append(cd[self.flag_dict[k]])
            else:
                retval.append(tuple([i,]+cd))
        if colwise:
            keys,values = zip(*retval.items())
            values = zip(*self._subsample(retval['q'], zip(*values), maxpoints))
            values = map(list, values)
            return dict(zip(keys,values))
        else:
            return self._subsample([i[1] for i in retval], retval,maxpoints)

    def get_gamma(self):
        return self.gamma

    def get_offset(self):
        return self.offset

    def set_gamma(self, gamma):
        self.gamma = gamma

    def set_offset(self, offset):
        self.offset = offset

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

    def set_interpolant(self, gp, particles, functions, mean, model, bayes,
            hessian=None):
        """store a class that gives interpolated values,
        usually an instance of the GaussianProcessInterpolation
        """
        if hessian is not None:
            self._memoized_hessian = hessian
            self.recompute_hessian = False
        else:
            self.recompute_hessian = True
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
        if not set(functions.keys()) == set(['mean','covariance']):
            raise TypeError, "expected mean and covariance functions"
        if not isinstance(model, IMP.Model):
            raise TypeError, "fourth argument is expected to be an IMP.Model()"
        self.gp = gp
        self.particles = particles
        self.functions=functions
        self.model = model
        self.bayes = bayes
        self.mean = mean

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

    def _subsample(self, q, data, npoints):
        return subsample(q, data, npoints)

    def _setup_gp(self, npoints):
        if npoints <0:
            return self.gp
        q = self.gp.get_data_abscissa()
        I = self.gp.get_data_mean()
        S = self.gp.get_data_variance()
        err = [S[i][i] for i in xrange(len(S))]
        q,I,err = zip(*self._subsample(q,zip(q,I,err),npoints))
        gp = IMP.isd.GaussianProcessInterpolation(q, I, err,
            self.get_Nreps(), self.functions['mean'],
            self.functions['covariance'], self.particles['sigma2'])
        return gp

    def _get_hessian(self, gp):
        self.particles['A'].set_nuisance_is_optimized(True)
        if self.mean == 'Flat':
            self.particles['G'].set_lower(0)
            self.particles['G'].set_nuisance(0)
            self.particles['G'].set_upper(0)
            self.particles['G'].set_nuisance_is_optimized(False)
            self.particles['Rg'].set_nuisance_is_optimized(False)
            self.particles['d'].set_nuisance_is_optimized(False)
            self.particles['s'].set_nuisance_is_optimized(False)
        else:
            self.particles['G'].set_nuisance_is_optimized(True)
            self.particles['Rg'].set_nuisance_is_optimized(True)
            if self.mean == 'Simple':
                self.particles['d'].set_nuisance_is_optimized(False)
            else:
                self.particles['d'].set_nuisance_is_optimized(True)
                if self.mean == 'Generalized':
                    self.particles['s'].set_nuisance_is_optimized(False)
                else:
                    self.particles['s'].set_nuisance_is_optimized(True)
        self.particles['tau'].set_nuisance_is_optimized(True)
        self.particles['lambda'].set_nuisance_is_optimized(True)
        self.particles['sigma2'].set_nuisance_is_optimized(True)
        if self.recompute_hessian is False:
            return self._memoized_hessian
        gpr=IMP.isd.GaussianProcessInterpolationRestraint(gp)
        self.model.add_restraint(gpr)
        Hessian = matrix(gpr.get_hessian(False))
        if linalg.det(Hessian) == 0:
            print Hessian,[(i,j.get_nuisance())
                    for i,j in self.particles.items()
                if j.get_nuisance_is_optimized()]
        self.model.remove_restraint(gpr)
        #fl=open('avcov','w')
        #fl.write("#q I err err*lapl lapl det (Hessian= %f )\n"
        #        % linalg.det(Hessian))
        #fl.close()
        self._memoized_hessian = Hessian
        self.recompute_hessian = False
        return Hessian

    def get_mean(self, **kwargs):
        """returns (q,I,err,mean,...) tuples for num points (default 200)
        ranging from qmin to qmax (whole data range by default)
        possible keyword arguments:
            num qmin qmax filter average verbose
        mean is the mean function of the gaussian process.
        the length of the returned tuple is the same as the number of flags plus
        one for the mean function.
        You can also give specific values via the qvalues keyword argument.
        see get_data for details on the filter argument. If num is used with
        filter, the function returns at most num elements and possibly none.
        average : if 0, just output maximum posterior estimate (faster).
                  if <0, compute hessians and average out parameter vector.
                  if >0, same as <0 but only take N data points instead of all
        verbose : if True, display progress meter in %
        """
        rem = set(kwargs.keys()) \
                - set(['qvalues','num','qmin','qmax','filter',
                       'colwise','average','verbose'])
        if len(rem) > 0:
            raise TypeError, "Unknown keyword(s): %s" % list(rem)
        #
        average=0
        if 'average' in kwargs and kwargs['average'] != 0:
            average=kwargs['average']
            #compute hessian, which doesn't depend on q
            gp = self._setup_gp(average)
            Hessian = self._get_hessian(gp)
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
        verbose=False
        if 'verbose' in kwargs:
            verbose = kwargs['verbose']
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
        flagnames = list(self.get_flag_names())
        flagnames = flagnames[:3]+['mean']+flagnames[3:]
        flags = []
        gamma = self.get_gamma()
        offset = self.get_offset()
        if colwise:
            retval=dict.fromkeys(flagnames)
            for i in retval:
                retval[i] = []
        else:
            retval=[]
        flagdict = copy.deepcopy(self.flag_dict)
        for k in flagdict:
            if flagdict[k] >= 3:
                flagdict[k] += 1
        flagdict['mean']=3
        for i,q in enumerate(qvals):
            if verbose:
                percent=str(int(round((i+1)/float(len(qvals))*100)))
                sys.stdout.write("%s%%" % (percent))
                sys.stdout.flush()
                sys.stdout.write('\b'*(len(percent)+1))
            if len(flagnames) > 4:
                flags = map(self.get_flag, [q]*len(flagnames[4:]),
                                           flagnames[4:])
                if False in [flags[i-3] for i in flagnos]:
                    continue
            thing = [q,gamma*(self.gp.get_posterior_mean([q])+offset),
                    gamma*sqrt(self.gp.get_posterior_covariance([q],[q])),
                    gamma*(self.functions['mean']([q])[0]+offset) ]
            if average != 0:
                #compute hessian of -log(cov)
                cov = thing[2]**2
                Hder = matrix(gp.get_posterior_covariance_derivative(
                    [q],False)).T
                Hcov = matrix(gp.get_posterior_covariance_hessian(
                    [q],False))
                Hlcov = 1/cov*(-Hcov + 1/cov * Hder*Hder.T)
                #fl=open('avcov','a')
                #fl.write(" ".join(["%f" % i for  i in thing[:3]]))
                lapl = linalg.det(matrix(eye(Hessian.shape[0]))
                                  + linalg.solve(Hessian,Hlcov))**(-0.5)
                #fl.write(" %f" % sqrt(cov*lapl))
                #fl.write(" %f" % lapl)
                #fl.write(" %f\n" % linalg.det(Hlcov))
                #fl.close()
                #print thing[0],thing[1],cov,cov*lapl,lapl
                thing[2] = sqrt(cov*lapl)
            if flags:
                thing.extend(flags)
            if colwise:
                for flag in flagnames:
                    retval[flag].append(thing[flagdict[flag]])
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
            float_fmt='%-15.14G', *args, **kwargs):
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
            float_fmt="%-15.14G", *args, **kwargs):
        """write gaussian process interpolation to a file named
        dir+prefix+filename+suffix.
        bool_to_int : convert booleans to 0 or 1 in output.
        header : if True, first line starts with # and is a header
        flags : specify which flags to write, if None write all.
        float_fmt : the format for outputting floats
        """
        fl=open(os.path.join(dir,prefix+filename+suffix),'w')
        allflags = list(self.get_flag_names())
        allflags = allflags[:3]+['mean']+allflags[3:]
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
    parser = IMP.OptionParser(
            description="Perform a statistical merge of the given SAXS curves. "
                  "file is a 3-column file that contains SAXS data. "
                  "To specify the number of repetitions of this experiment, "
                  "use the syntax file.txt=20 indicating that data "
                  "in file.txt is an average of 20 experiments. Default is "
                  "10. Note that in the case of different number of "
                  "repetitions, the minimum is taken for the final "
                  "fitting step (Step 5).",
            formatter=_RawEpilogFormatter(),
            usage="%prog [options] file [file ...]",
            version='%prog 0.3',
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
            "level, 'sparse' is for q,I,err columns only, 'normal' adds "
            "eorigin, eoriname and eextrapol (default), and 'full' outputs "
            "all flags.", type="choice", choices=['normal','sparse','full'])
    group.add_option('--allfiles', default=False, action='store_true',
            help="Output data files for parsed input files as well (default "
            "is only to output merge and summary files).")
    group.add_option('--stop', type='choice', help="stop after the given step, "
            "one of cleanup, fitting, rescaling, classification, merging "
            "(default: merging)", choices=["cleanup","fitting","rescaling",
                "classification", "merging"], default="merging")
    group.add_option('--postpone_cleanup', action='store_true', default=False,
            help="Cleanup step comes after rescaling step (default is False)")
    group.add_option('--npoints', type="int", default=200, metavar="NUM",
            help="Number of points to output for the mean function. Negative "
            "values signify to take the same q values as the first data file. "
            "In that case extrapolation flags are ignored, and extrapolation "
            "is performed when the data file's q values fall outside of the "
            "range of accepted data points. Default is NUM=200 points.")
    group.add_option('--lambdamin', type="float", default=0.005, metavar="MIN",
            help="lower bound for lambda parameter in steps 2 and 5")
    #cleanup
    group = optparse.OptionGroup(parser, title="Cleanup (Step 1)",
                              description="Discard or keep SAXS curves' "
                              "points based on their SNR. Points with an error"
                              " of zero are discarded as well")
    parser.add_option_group(group)

    group.add_option('--aalpha', help='type I error (default 1e-4)',
                     type="float", default=1e-4, metavar='ALPHA')
    group.add_option('--acutoff', help='when a value after CUT is discarded,'
            ' the rest of the curve is discarded as well (default is 0.1)',
            type="float", default=0.1, metavar='CUT')
    #fitting
    group = optparse.OptionGroup(parser, title="Fitting (Step 2)",
                description="Estimate the mean function and the noise level "
                "of each SAXS curve.")
    parser.add_option_group(group)
    group.add_option('--bd', help='Initial value for d (default 4)',
                     type="float", default=4., metavar='D')
    group.add_option('--bs', help='Initial value for s (default 0)',
                     type="float", default=0., metavar='S')
    group.add_option('--bmean', help='Defines the most complex mean '
            'function that will be tried during model comparison.'
            " One of Flat (the offset parameter A is optimized), "
            "Simple (optimizes A, G and Rg), "
            "Generalized (optimizes G, Rg and d), "
            "Full (default, optimizes G, Rg, d and s) "
            "If --bnocomp is given, will try to fit only with this model",
            type="choice", default="Full",
            choices=['Flat','Simple','Generalized','Full'])
    group.add_option('--bnocomp', help='Don\'t perform model comparison. '
            "Default: perform it.",
            action='store_true', default=False)
    group.add_option('--baverage', help='Average over all possible parameters '
            'instead of just taking the most probable set of parameters. '
            'Default is not to perform the averaging.',
            action='store_true', default=False)
    group.add_option('--blimit_fitting', metavar='NUM', default=-1, type='int',
            help='To save resources, set the maximum number of points used in'
            'the fitting step. Dataset will be subsampled if it is bigger than'
            ' NUM. If NUM=-1 (default), all points will be used.')
    group.add_option('--blimit_hessian', metavar='NUM', default=-1, type='int',
            help='To save resources, set the maximum number of points used in'
            ' the Hessian calculation (model comparison, options --baverage, '
            'and --berror ). Dataset will be subsampled if it is bigger than'
            'NUM. If NUM=-1 (default), all points will be used.')
    group.add_option('--berror', action='store_true', default=False,
            help="Compute error bars on all parameters even in case where "
            "model comparison was disabled. Involves the computation of a "
            "Hessian. Default: no extra computation.")
    #rescaling
    group = optparse.OptionGroup(parser, title="Rescaling (Step 3)",
                description="Find the most probable scaling factor of all "
                "curves wrt the first curve.")
    parser.add_option_group(group)
    group.add_option('--creference', default='last', help="Define which "
            "input curve the other curves will be rescaled to. Options are "
            "first or last (default is last)", type="choice",
            choices=['first','last'])
    group.add_option('--cmodel', default='normal',
            help="Which rescaling model to use to calculate gamma. "
            "'normal-offset' for a normal model with offset, "
            "'normal' (default) for a normal model with zero offset "
            "and 'lognormal' for a lognormal model.",
            choices=['normal','normal-offset','lognormal'], type='choice')
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
    group.add_option('--eextrapolate', metavar="NUM", help='Extrapolate '
            "NUM percent outside of the curve's bounds. Example: if NUM=50 "
            "and the highest acceptable data point is at q=0.3, the mean will "
            "be estimated up to q=0.45. Default is 0 (just extrapolate at low "
            "angle).", type="int", default=0)
    group.add_option('--enoextrapolate', action='store_true', default=False,
            help="Don't extrapolate at all, even at low angle (default False)")
    group.add_option('--emean', help='Which most complex mean function '
            'to try for model comparison.'
            ' See --bmean. Default is Full', type="choice",
            default="Full", choices=['Simple','Generalized','Full','Flat'])
    group.add_option('--enocomp', help='Don\'t perform model comparison, '
            'see --bnocomp. Default is not to perform it.',
            action='store_true', default=False)
    group.add_option('--eaverage', help="Average over all possible parameters "
            "instead of just taking the most probable set of parameters. "
            "Default is not to perform the averaging.",
            action='store_true', default=False)
    group.add_option('--elimit_fitting', metavar='NUM', default=-1, type='int',
            help='To save resources, set the maximum number of points used in'
            'the fitting step. Dataset will be subsampled if it is bigger than'
            ' NUM. If NUM=-1 (default), all points will be used.')
    group.add_option('--elimit_hessian', metavar='NUM', default=-1, type='int',
            help='To save resources, set the maximum number of points used in'
            ' the Hessian calculation (model comparison, options --eaverage, '
            'and --eerror ). Dataset will be subsampled if it is bigger than'
            'NUM. If NUM=-1 (default), all points will be used.')
    group.add_option('--eerror', action='store_true', default=False,
            help="Compute error bars on all parameters even in case where "
            "model comparison was disabled. Involves the computation of a "
            "Hessian. Default: no extra computation.")
    (args, files) = parser.parse_args()
    if len(files) == 0:
        parser.error("No files specified")
    return (args, files)

def parse_filenames(fnames, defaultvalue=10):
    files = []
    Nreps = []
    for fname in fnames:
        if '=' in fname:
            tokens = fname.split('=')
            paths = glob.glob(tokens[0])
            if len(paths) == 0:
                sys.exit("File %s not found!" % fname)
            files.extend(paths)
            Nreps.append(int(tokens[1]))
        else:
            paths = glob.glob(fname)
            if len(paths) == 0:
                sys.exit("File %s not found!" % fname)
            files.extend(paths)
            Nreps.append(defaultvalue)
    return files, Nreps

def get_global_scaling_factor(file):
    """get an order of magnitude for 100/I(0)"""
    p=SAXSProfile()
    p.add_data(file, positive=True)
    data=p.get_raw_data()
    n=len(data)
    #take the first 50 points or 10% whichever comes first
    m=min(int(0.1*n),50)
    return 100./mean([i[1] for i in data[:m]])

def create_profile(file, nreps, scale=1):
    p=SAXSProfile()
    p.add_data(file, positive=True, scale=scale)
    p.set_Nreps(nreps)
    p.set_filename(file)
    return p

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
    returns: model, dict(*particles), dict(mean, covariance)
    """

    model = IMP.Model()
    #mean function
    G=IMP.isd.Scale.setup_particle(IMP.Particle(model,"G"),initvals['G'])
    #model.add_restraint(IMP.isd.JeffreysRestraint(G))
    Rg=IMP.isd.Scale.setup_particle(IMP.Particle(model,"Rg"),initvals['Rg'])
    d=IMP.isd.Scale.setup_particle(IMP.Particle(model,"d"),initvals['d'])
    s=IMP.isd.Scale.setup_particle(IMP.Particle(model,"s"),initvals['s'])
    A=IMP.isd.Nuisance.setup_particle(IMP.Particle(model,"A"),initvals['A'])
    m = IMP.isd.GeneralizedGuinierPorodFunction(G,Rg,d,s,A)
    #covariance function
    tau=IMP.isd.Scale.setup_particle(IMP.Particle(model,"tau"),initvals['tau'])
    lam=IMP.isd.Scale.setup_particle(IMP.Particle(model,"lambda"),
                                        initvals['lambda'])
    w = IMP.isd.Covariance1DFunction(tau,lam,2.0)
    #sigma
    sigma=IMP.isd.Scale.setup_particle(IMP.Particle(model,"sigma2"),
                                    initvals['sigma2'])
    #prior on scales
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,G,"Constrain_G"))
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,Rg,"Constrain_Rg"))
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,d,"Constrain_d"))
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,A,"Constrain_A"))
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,s,"Constrain_s"))
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,lam,"Constrain_lambda"))
    #model.add_restraint(IMP.isd.JeffreysRestraint(tau))
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,tau,"Constrain_tau"))
    model.add_restraint(IMP.isd.JeffreysRestraint(sigma))
    model.add_score_state(IMP.core.SingletonConstraint(
        IMP.isd.NuisanceRangeModifier(),None,sigma,"Constrain_sigma2"))
    #set lower and upper bounds for Rg, d and s
    Rg.set_lower(0.1)
    d.set_lower(0.1)
    d.set_upper(8.)
    s.set_upper(3.)
    s.set_upper(d)
    #set lower bounds for cov particles
    tau.set_lower(0.01)
    sigma.set_lower(0.01)
    sigma.set_upper(1000.)
    #
    particles = {}
    particles['G'] = G
    particles['Rg'] = Rg
    particles['d'] = d
    particles['s'] = s
    particles['A'] = A
    particles['tau'] = tau
    particles['lambda'] = lam
    particles['sigma2'] = sigma
    functions = {}
    functions['mean'] = m
    functions['covariance'] = w
    return model, particles, functions

def do_conjugategradients(model,nsteps):
    cg=IMP.gsl.ConjugateGradients(model)
    cg.optimize(nsteps)

def do_quasinewton(model,nsteps):
    qn=IMP.gsl.QuasiNewton(model)
    #fl=open('qn.txt','w')
    #write_header(fl)
    #write_params(fl,model,a,b,tau,lam,sigma)
    #for i in xrange(nsteps):
    #    if print_steps >0 and i % print_steps == 0 :
    #        print i,
    #        sys.stdout.flush()
    #    IMP.base.set_log_level(IMP.base.TERSE)
    #    qn.optimize(1)
    #    IMP.base.set_log_level(0)
    #    write_params(fl,model,a,b,tau,lam,sigma)
    qn.optimize(nsteps)

def setup_process(data,initvals, subs, maxpoints=-1):
    model,particles,functions = setup_particles(initvals)
    q = [ [i] for i in data['q'][::subs]]
    I = data['I'][::subs]
    err = data['err'][::subs]
    if maxpoints >0:
        q,I,err = zip(*subsample(q,zip(q,I,err),maxpoints))
    gp = IMP.isd.GaussianProcessInterpolation(q, I, err,
            data['N'], functions['mean'], functions['covariance'],
            particles['sigma2'])
    return model, particles, functions, gp

def get_initial_Rg(data):
    fl, name = tempfile.mkstemp(text=True)
    handle = os.fdopen(fl,'w')
    for dat in zip(data['q'],data['I'],data['err']):
        handle.write(' '.join(['%G' % i for i in dat]))
        handle.write('\n')
    handle.close()
    profile = IMP.saxs.Profile(name)
    Rg = profile.radius_of_gyration()
    os.unlink(name)
    return Rg

def set_defaults_mean(data, particles, mean_function):
    #set initial value for G to be a rough estimate of I(0)
    #take first 10 pc or 20 points at least
    npoints=max(len(data['q'])/10,20)
    Ivals = [data['I'][i] for i in xrange(len(data['q']))][:npoints]
    particles['G'].set_nuisance(mean(Ivals))
    particles['G'].set_lower(min(Ivals))
    particles['G'].set_upper(2*max(Ivals))
    #optimize mean particles only
    particles['A'].set_nuisance_is_optimized(True)
    if mean_function == 'Flat':
        particles['G'].set_lower(0)
        particles['G'].set_nuisance(0)
        particles['G'].set_upper(0)
        particles['G'].set_nuisance_is_optimized(False)
        particles['Rg'].set_nuisance_is_optimized(False)
        particles['d'].set_nuisance_is_optimized(False)
        particles['s'].set_nuisance_is_optimized(False)
    else:
        particles['G'].set_nuisance_is_optimized(True)
        particles['Rg'].set_nuisance_is_optimized(True)
        particles['Rg'].set_nuisance(get_initial_Rg(data))
        if mean_function == 'Simple':
            particles['d'].set_nuisance_is_optimized(False)
        else:
            particles['d'].set_nuisance_is_optimized(True)
            if mean_function == 'Generalized':
                particles['s'].set_nuisance_is_optimized(False)
            else:
                particles['s'].set_nuisance_is_optimized(True)
    particles['tau'].set_nuisance_is_optimized(False)
    particles['lambda'].set_nuisance_is_optimized(False)
    particles['sigma2'].set_nuisance_is_optimized(False)

def find_fit_mean(data, initvals, verbose, mean_function):
    model, particles, functions, gp = \
            setup_process(data, initvals, 1)
    IMP.base.set_log_level(IMP.base.TERSE)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(gp)
    model.add_restraint(gpr)
    set_defaults_mean(data, particles, mean_function)
    #set tau to 0 allows for faster estimate (diagonal covariance matrix)
    #no need to store its value since it will be reset later on
    taulow = None
    if particles['tau'].has_lower():
        taulow = particles['tau'].get_lower()
    particles['tau'].set_nuisance(0.)
    particles['tau'].set_lower(0.)
    #particles['A'].set_nuisance(-2.88)
    #particles['G'].set_nuisance(5485.31)
    #particles['Rg'].set_nuisance(42.09)
    #particles['d'].set_nuisance(2.92)
    #for q in linspace(0.001,0.3):
    #    print "cmp",q,gp.get_posterior_mean([q])
    #IMP.base.set_log_level(IMP.base.TERSE)
    #for q in linspace(0.001,0.3):
    #    print "cmp",q,gp.get_posterior_mean([q])
    #sys.exit()
    #optimize mean particles for 3x100 steps
    #when getting initvals, calling model.evaluate() ensures constraints are met
    #print particles['A'].get_nuisance()
    #print [(a,b.get_nuisance()) for a,b in particles.items()]
    #dg=IMP.get_dependency_graph(model)
    #IMP.base.show_graphviz(dg)
    #print IMP.get_required_score_states(gpr, [], dg,
    #        IMP.get_dependency_graph_vertex_index(dg))
    #sys.exit()
    rgopt = particles['Rg'].get_nuisance_is_optimized()
    particles['Rg'].set_nuisance_is_optimized(False)
    for i in xrange(2):
        model.evaluate(False)
        do_conjugategradients(model,100)
    particles['Rg'].set_nuisance_is_optimized(rgopt)
    for i in xrange(2):
        #print particles['A'].get_nuisance()
        #print [(k,v.get_nuisance()) for (k,v) in particles.items()]
        model.evaluate(False)
        do_conjugategradients(model,100)
    for i in xrange(2):
        #print particles['A'].get_nuisance()
        #print [(k,v.get_nuisance()) for (k,v) in particles.items()]
        model.evaluate(False)
        do_quasinewton(model,100)
    #set_defaults_mean(data, particles, 'Full')
    #for A in linspace(-100,100):
    #    particles['A'].set_nuisance(A)
    #    print "pyy",A,model.evaluate(False)
    #set_defaults_mean(data, particles, 'Generalized')
    #for A in linspace(-100,100):
    #    particles['A'].set_nuisance(A)
    #    print "pyy",A,model.evaluate(False)
    #sys.exit()
    #for q in linspace(0.001,0.3):
    #    print "cmp",q,gp.get_posterior_mean([q])
    #for i in particles:
    #    print i,particles[i].get_nuisance()
    #sys.exit()
    #reset tau bounds
    for i in particles.values():
        if i.get_nuisance_is_optimized():
            if isnan(i.get_nuisance()):
                raise FittingError
    if taulow:
        particles['tau'].set_lower(taulow)
    model.evaluate(False)
    newvals = dict([(k,v.get_nuisance())
                    for (k,v) in particles.iteritems()])
    return newvals

def set_defaults_covariance(data, particles, functions, args):
    #set sigma to be equal to 10x the noise level, assuming mean has been fitted
    chisquares = [functions['mean']([q])[0] for q in data['q']]
    chisquares = [(data['I'][i]-chisquares[i])**2 + data['err'][i]**2
                        for i in xrange(len(data['q']))]
    noiselevel = (sum(chisquares)/len(chisquares))
    errorlevel = (sum([data['err'][i]**2 for i in xrange(len(data['q']))])
                    /len(data['err']))
    sigmaval = 10*noiselevel/errorlevel
    particles['sigma2'].set_nuisance(sigmaval)
    #set tau to be equal to this value
    particles['tau'].set_nuisance(sigmaval)

def find_fit_lambda(data, initvals, args, verbose):
    #optimize covariance, first on lambda by subsampling the curve
    #print "lambda opt"
    meandist = mean(array(data['q'][1:])-array(data['q'][:-1]))
    initial=True
    for subs,nsteps in [(10,1000),(5,500)]:
        model, particles, functions, gp = \
                setup_process(data, initvals, subs)
        if initial:
            initial=False
            set_defaults_covariance(data, particles, functions, args)
        model.evaluate(False)
        #initvals = dict([(k,v.get_nuisance())
        #                for (k,v) in particles.iteritems()])
        #print " ".join(["%5s=%10G" % d for d in initvals.items()])
        #set lambda to be bigger than the distance between points
        particles['lambda'].set_lower(2*meandist)
        particles['G'].set_nuisance_is_optimized(False)
        particles['Rg'].set_nuisance_is_optimized(False)
        particles['d'].set_nuisance_is_optimized(False)
        particles['s'].set_nuisance_is_optimized(False)
        particles['A'].set_nuisance_is_optimized(False)
        particles['tau'].set_nuisance_is_optimized(False)
        particles['lambda'].set_nuisance_is_optimized(True)
        particles['sigma2'].set_nuisance_is_optimized(False)
        gpr = IMP.isd.GaussianProcessInterpolationRestraint(gp)
        model.add_restraint(gpr)
        do_quasinewton(model,nsteps)
        model.evaluate(False)
        initvals = dict([(k,v.get_nuisance())
                        for (k,v) in particles.iteritems()])
        #print " ".join(["%5s=%10G" % d for d in initvals.items()])
    return initvals

def find_fit_covariance(data, initvals, args, verbose):
    #then on all covariance particles
    #print "cov opt"
    model, particles, functions, gp = \
            setup_process(data, initvals, 1, args)
    set_defaults_covariance(data, particles, functions, args)
    meandist = mean(array(data['q'][1:])-array(data['q'][:-1]))
    particles['lambda'].set_lower(meandist)
    #optimize all covariance particles for 3x10 steps and all data points
    particles['G'].set_nuisance_is_optimized(False)
    particles['Rg'].set_nuisance_is_optimized(False)
    particles['d'].set_nuisance_is_optimized(False)
    particles['s'].set_nuisance_is_optimized(False)
    particles['A'].set_nuisance_is_optimized(False)
    particles['tau'].set_nuisance_is_optimized(True)
    particles['lambda'].set_nuisance_is_optimized(True)
    particles['sigma2'].set_nuisance_is_optimized(True)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(gp)
    model.add_restraint(gpr)
    for i in xrange(3):
        do_quasinewton(model,30)
    model.evaluate(False)
    initvals = dict([(k,v.get_nuisance()) for (k,v) in particles.iteritems()])
    #print " ".join(["%5s=%10G" % d for d in initvals.items()])
    if False:
        #plotrange
        minval=(model.evaluate(False),particles['lambda'].get_nuisance())
        initvals = dict([(k,v.get_nuisance()) for (k,v) in particles.iteritems()])
        particles['lambda'].set_lower(0.)
        for l in linspace(.01,10,num=100):
            particles['lambda'].set_nuisance(l)
            ene = model.evaluate(True)
            deriv = psarticles['lambda'].get_nuisance_derivative()
            if minval[0] > ene:
                minval = (ene, l)
            print "plotrange",l,ene,deriv
        print "minimum: ene=%f lambda=%f" % minval
    if False:
        #residuals
        for k,v in particles.items():
            v.set_nuisance(initvals[k])
        for q,I,err in zip(data['q'],data['I'],data['err']):
            gpval=gp.get_posterior_mean([q])
            avg=functions['mean']([q])[0]
            print "residuals",q,I,err,gpval,avg,(gpval-avg),(I-avg)
    return initvals

def find_fit_by_minimizing(data, initvals, verbose, lambdalow):
    model, particles, functions, gp = \
            setup_process(data, initvals, 1)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(gp)
    model.add_restraint(gpr)
    meandist = mean(array(data['q'][1:])-array(data['q'][:-1]))
    particles['lambda'].set_lower(max(2*meandist,lambdalow))
    particles['lambda'].set_upper(max(data['q'])/2.)
    particles['lambda'].set_nuisance(max(data['q'])/10.)
    lambdamin = particles['lambda'].get_lower()
    lambdamax = particles['lambda'].get_upper()
    particles['sigma2'].set_lower(1)
    particles['sigma2'].set_upper(1000)
    particles['sigma2'].set_nuisance(100.0)
    sigmamin = particles['sigma2'].get_lower()
    sigmamax = particles['sigma2'].get_upper()
    particles['tau'].set_lower(0.001)
    particles['tau'].set_nuisance(10.0)
    taumin = particles['tau'].get_lower()
    taumax = 100
    #print "minimizing"
    particles['lambda'].set_nuisance_is_optimized(True)
    particles['sigma2'].set_nuisance_is_optimized(False)
    particles['tau'].set_nuisance_is_optimized(True)
    minimas = []
    while len(minimas) < 5:
        initm = model.evaluate(False)
        initt = particles['tau'].get_nuisance()
        inits = particles['sigma2'].get_nuisance()
        initl = particles['lambda'].get_nuisance()
        #steepest descent
        sd = IMP.core.SteepestDescent(model)
        sd.optimize(500)
        #conjugate gradients
        cg = IMP.core.ConjugateGradients(model)
        cg.optimize(500)
        #add to minimas
        minimas.append((model.evaluate(False), initm,
            particles['tau'].get_nuisance(), initt,
            particles['sigma2'].get_nuisance(), inits,
            particles['lambda'].get_nuisance(), initl))
        print "--- new min"
        print "   tau",initt, minimas[-1][2]
        print "   sig",inits, minimas[-1][4]
        print "   lam",initl, minimas[-1][6]
        print "   mod",initm, minimas[-1][0]
        #draw new random starting position
        particles['tau'].set_nuisance(
                exp(random.uniform(log(taumin)+1,log(taumax)-1)))
        particles['sigma2'].set_nuisance(
                exp(random.uniform(log(sigmamin)+1,log(sigmamax)-1)))
        particles['lambda'].set_nuisance(
                exp(random.uniform(log(lambdamin)+1,log(lambdamax)-1)))
    minimas.sort(key=lambda a:a[0])
    print "best is ",minimas[0][0]
    particles['tau'].set_nuisance(minimas[0][2])
    particles['sigma2'].set_nuisance(minimas[0][4])
    particles['lambda'].set_nuisance(minimas[0][6])
    ene = model.evaluate(False)
    initvals = dict([(k,v.get_nuisance()) for (k,v) in particles.iteritems()])
    return initvals

def find_fit_by_gridding(data, initvals, verbose, lambdalow):
    """use the fact that sigma2 can be factored out of the covariance matrix and
    drawn from a gamma distribution. Calculates a 2D grid on lambda (D1) and
    tau**2/sigma2 (D2) to get the minimum value.
    """
    model, particles, functions, gp = \
            setup_process(data, initvals, 1)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(gp)
    model.add_restraint(gpr)
    meandist = mean(array(data['q'][1:])-array(data['q'][:-1]))
    particles['lambda'].set_lower(max(2*meandist,lambdalow))
    lambdamin = particles['lambda'].get_lower()
    lambdamax = 100
    numpoints=30
    gridvalues = []
    particles['tau'].set_lower(0.)
    particles['sigma2'].set_lower(0.)
    #fl=open('grid.txt','w')
    particles['sigma2'].set_nuisance(100.0)
    particles['sigma2'].set_upper(1000.0)
    particles['tau'].set_nuisance(10.0)
    #print "gridding"
    experror=0
    for lambdaval in logspace(log(lambdamin),log(lambdamax),
            base=exp(1),num=numpoints):
        for rel in logspace(-2, 2, num=numpoints):
            particles['lambda'].set_nuisance(lambdaval)
            #set new value of tau**2/sigma
            tauval = particles['tau'].get_nuisance()
            if tauval**2/rel > particles['sigma2'].get_upper():
                continue
            particles['sigma2'].set_nuisance(tauval**2/rel)
            #print "sigma2 has val",particles['sigma2'].get_nuisance(), \
            #        "tau has val",particles['tau'].get_nuisance()
            #get exponent and compute reduced exponent
            exponent = gpr.get_minus_exponent()
            if isnan(exponent) or exponent <=0:
                if experror < 2:
                    print "got invalid exponent at grid point "\
                      "lambda=%f rel=%f exp=%s" % (lambdaval,rel, exponent)
                experror += 1
                continue
            redexp = tauval * exponent
            #compute maximum posterior value for tau**2 assuming jeffreys prior
            tauval = (redexp/(len(data['q'])+2))**.5
            sigmaval = tauval**2/rel
            #print "sigma=",sigmaval,"tau=",tauval,\
            #        "tau**2/sigma2=",rel,"lambda=",lambdaval
            if sigmaval > particles['sigma2'].get_upper():
                #skip value if outside of bounds for sigma
                continue
            particles['tau'].set_nuisance(tauval)
            #reset tau to correct value and get minimized energy
            if sigmaval > particles['sigma2'].get_upper():
                #skip value if outside of bounds for sigma
                continue
            particles['sigma2'].set_nuisance(sigmaval)
            ene = model.evaluate(False)
            gridvalues.append((lambdaval,rel,sigmaval,tauval,ene))
            #fl.write(" ".join(["%f" % i
            #            for i in lambdaval,rel,sigmaval,tauval,ene]))
            #fl.write('\n')
        #fl.write('\n')
    #print "minimizing"
    if experror >=2:
        print "skipped another " + str(experror-2) + " similar errors"
    particles['tau'].set_lower(0.01)
    particles['tau'].set_upper(100)
    particles['sigma2'].set_lower(1.)
    particles['sigma2'].set_upper(1000)
    if len(gridvalues) == 0:
        raise FittingError
    minval = gridvalues[0][:]
    for i in gridvalues:
        if i[4] < minval[4]:
            minval = i[:]
    minval=list(minval)
    #minval[0]=minval[0]
    #minval[1]=minval[1]
    #print "minimum",minval
    particles['lambda'].set_nuisance_is_optimized(True)
    particles['sigma2'].set_nuisance_is_optimized(True)
    particles['tau'].set_nuisance_is_optimized(True)
    #do 3 independent minimizations, pick best run
    bestmin=[]
    for i in xrange(3):
        particles['lambda'].set_nuisance(minval[0])
        particles['sigma2'].set_nuisance(minval[2])
        particles['tau'].set_nuisance(minval[3])
        do_quasinewton(model,10)
        bestmin.append((model.evaluate(False),
            particles['lambda'].get_nuisance(),
            particles['sigma2'].get_nuisance(),
            particles['tau'].get_nuisance()))
    bestmin.sort()
    particles['lambda'].set_nuisance(bestmin[0][1])
    particles['sigma2'].set_nuisance(bestmin[0][2])
    particles['tau'].set_nuisance(bestmin[0][3])
    #print " "
    #for i in bestmin:
    #    print i
    #for i,p in particles.items():
    #    print i,p.get_nuisance(),p.get_lower(),p.get_upper()
    #print " "
    #global fitno
    #fl=open('fit_%d.dat' % fitno,'w')
    #fitno += 1
    #for i,q in enumerate(data['q']):
    #    fl.write('%s %s %s %s %s\n' % (q,data['I'][i],data['err'][i],
    #                gp.get_posterior_mean([q]),
    #                sqrt(gp.get_posterior_covariance([q],[q]))))
    #ene = model.evaluate(False)
    #newmin = [particles['lambda'].get_nuisance(),None,
    #        particles['sigma2'].get_nuisance(),particles['tau'].get_nuisance(),
    #        ene]
    #newmin[1]=newmin[3]**2/newmin[2]
    #newmin=tuple(newmin)
    initvals = dict([(k,v.get_nuisance()) for (k,v) in particles.iteritems()])
    return initvals

def bayes_factor(data, initvals, verbose, mean_func, maxpoints):
    """given optimized values for the parameters (in initvals), the data, and
    the model specification (in mean_func), return
    0 the number of parameters (Np) that have been optimized
    1 sqrt(2pi)^Np
    2 hessian (H) wrt parameter vector
    3 1/2*log(abs(det(H)))
    4 minus log maximum posterior value (MP)
    5 minus log maximum likelihood value
    6 minus log maximum prior value
    7 exp(-MP)*sqrt(2Pi)^Np*det(H)^(-1/2) (bayes factor)
    8 MP - Np/2*log(2Pi) + 1/2*log(det(H)) (minus log bayes factor)
    """
    model, particles, functions, gp = setup_process(data, initvals, 1,
                                                        maxpoints=maxpoints)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(gp)
    model.add_restraint(gpr)
    particles['A'].set_nuisance_is_optimized(True)
    if mean_func == 'Flat':
        particles['G'].set_nuisance_is_optimized(False)
        particles['Rg'].set_nuisance_is_optimized(False)
        particles['d'].set_nuisance_is_optimized(False)
        particles['s'].set_nuisance_is_optimized(False)
    else:
        particles['G'].set_nuisance_is_optimized(True)
        particles['Rg'].set_nuisance_is_optimized(True)
        if mean_func == 'Simple':
            particles['d'].set_nuisance_is_optimized(False)
        else:
            particles['d'].set_nuisance_is_optimized(True)
            if mean_func == 'Generalized':
                particles['s'].set_nuisance_is_optimized(False)
            else:
                particles['s'].set_nuisance_is_optimized(True)
    particles['tau'].set_nuisance_is_optimized(True)
    particles['lambda'].set_nuisance_is_optimized(True)
    particles['sigma2'].set_nuisance_is_optimized(True)

    H = matrix(gpr.get_hessian(False))
    Np = H.shape[0]
    MP = model.evaluate(False)
    ML = gpr.unprotected_evaluate(None)
    try:
        retval = linalg.slogdet(H)
        if retval[0] == 0 and retval[1] == -inf:
            print "Warning: skipping model %s" % mean_func
            logdet = inf
        else:
            logdet = retval[1]/2.
    except AttributeError:
        #older numpy versions don't have slogdet, try built-in
        #at the cost of an extra hessian calculation
        try:
            retval = gpr.get_logdet_hessian()
        except IMP.ModelException:
            print "Warning: Hessian is not positive definite"
            logdet=inf
        if isinf(retval):
            print "Warning: re-skipping model %s" % mean_func
            logdet = inf
        else:
            logdet = retval/2.
    return (Np, (2*pi)**(Np/2.), H, logdet, MP, ML, MP-ML,
            exp(-MP)*(2*pi)**(Np/2.)*exp(-logdet),
            MP - Np/2.*log(2*pi) + logdet)

def find_fit(data, initvals, verbose, model_comp=False, model_comp_maxpoints=-1,
        mean_function='Simple', lambdamin=0.005):
    #if verbose >2:
    #    print " %d:%d" % (subs,nsteps),
    #    sys.stdout.flush()
    #
    #compile a list of functions to fit, ordered by number of params
    functions=[]
    if model_comp:
        for i in ['Flat','Simple','Generalized','Full']:
            functions.append(i)
            if i==mean_function:
                break
    else:
        functions.append(mean_function)
    param_vals = {}
    #optimize parameters for each function
    if verbose > 2:
        print ""
    for mean_func in functions:
        if verbose > 2:
            sys.stdout.write("     "+mean_func+": mean ")
            sys.stdout.flush()
        try:
            param_vals[mean_func] = \
                find_fit_mean(data, initvals, verbose, mean_func)
        except FittingError:
            if model_comp:
                print "error"
                sys.stdout.flush()
                continue
            else:
                raise FittingError, "Error while fitting! Choose"\
                        " another model or do model comparison"
        if verbose > 2:
            sys.stdout.write("A=%1.2f " % param_vals[mean_func]['A'])
            if mean_func in ["Simple","Generalized","Full"]:
                sys.stdout.write("G=%1.2f " % param_vals[mean_func]['G'])
                sys.stdout.write("Rg=%1.2f " % param_vals[mean_func]['Rg'])
            if mean_func in ["Generalized","Full"]:
                sys.stdout.write("d=%1.2f " % param_vals[mean_func]['d'])
            if mean_func == "Full":
                sys.stdout.write("s=%1.2f " % param_vals[mean_func]['s'])
            sys.stdout.write("covariance ")
            sys.stdout.flush()
        #initvals = find_fit_lambda(data, initvals, args, verbose)
        #initvals = find_fit_covariance(data, initvals, args, verbose)
        try:
            param_vals[mean_func] = \
                find_fit_by_gridding(data, param_vals[mean_func], verbose,
                        lambdamin)
        except FittingError:
            if model_comp:
                print "error"
                sys.stdout.flush()
                continue
            else:
                raise FittingError, "Error while fitting! Choose"\
                        " another model or do model comparison"
        if verbose > 2:
            for i in ['tau','lambda','sigma2']:
                sys.stdout.write("%s=%1.2f " % (i,param_vals[mean_func][i]))
            print ""
            sys.stdout.flush()
    #compare functions via model comparison
    if len(functions) == 1:
        return functions[0], param_vals[functions[0]], {}
    free_energies={}
    if verbose > 2:
        print "     -log(Bayes Factor):",
    for f in functions:
        if verbose > 2:
            print f+" =",
        if f in param_vals:
            free_energies[f]=bayes_factor(data, param_vals[f], verbose, f,
                                        model_comp_maxpoints)
        else:
            free_energies[f] = [-1]+[inf]*8
        if verbose > 2:
            print "%.1f" % free_energies[f][8],
            sys.stdout.flush()
    if verbose > 2:
        print ""
    minf = sorted([(free_energies[i][8],i) for i in functions])[0][1]
    return minf,param_vals[minf],free_energies

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


def rescale_curves(refdata, data, normal = False, offset = False):
    """Find gamma and (optionnally) c so that
    normal model: I0 = gamma*(I1 + c)
    lognormal model log I0 = log(gamma*I1)
    if offset==False we have c=0
    """
    I0=array(refdata['I'])
    s0=array(refdata['err'])
    I1=array(data['I'])
    s1=array(data['err'])
    if not offset:
        if normal:
            weights = (s0**2+(s1*I0/I1)**2)**(-1)
            return (weights*I0*I1).sum()/(weights*I1**2).sum(),0
        else:
            weights=(s0**2/I0**2 + s1**2/I1**2)**(-1)
            lg = (weights*log(I0/I1)).sum()/weights.sum()
            return exp(lg),0
    else:
        if normal:
            weights = (s0**2+(s1*I0/I1)**2)**(-1)
            iexp = (weights*I0).sum()/weights.sum()
            icalc = (weights*I1).sum()/weights.sum()
            icalc2 = (weights*I1*I1).sum()/weights.sum()
            icie = (weights*I1*I0).sum()/weights.sum()
            c = (icalc*icie-icalc2*iexp)/(icalc*iexp-icie)
            gamma = (icie-iexp*icalc)/(icalc2-icalc**2)
            return gamma,c
        else:
            raise NotImplementedError



def write_individual_profile(prof, qvals, args):
    destname = os.path.basename(prof.get_filename())
    if args.outlevel == 'sparse':
        dflags = ['q','I','err']
        mflags = ['q','I','err']
    elif args.outlevel == 'normal':
        dflags = ['q','I','err','agood']
        mflags = ['q','I','err','mean','agood']
    else:
        dflags = None
        mflags = None
    prof.write_data(destname, bool_to_int=True, dir=args.destdir,
            header=args.header, flags=dflags)
    if args.postpone_cleanup or args.stop != "cleanup" :
        if args.npoints >0:
            prof.write_mean(destname, bool_to_int=True, dir=args.destdir,
                header=args.header, average=args.eaverage, num=args.npoints,
                flags=mflags)
        else:
            qvalues = prof.get_data(colwise=True)['q']
            qmin = min(qvalues)
            qmax = max(qvalues)
            qvalues = qvals[where(qvals >= qmin)]
            qvalues = qvalues[where(qvalues <= qmax)]
            prof.write_mean(destname, bool_to_int=True, dir=args.destdir,
                header=args.header, average=args.eaverage, qvalues=qvalues,
                flags=mflags)

def write_merge_profile(merge,qvals, args):
    if args.outlevel == 'sparse':
        dflags = ['q','I','err']
        mflags = ['q','I','err']
    elif args.outlevel == 'normal':
        dflags = ['q','I','err','eorigin','eoriname']
        mflags = ['q','I','err','mean','eorigin','eoriname','eextrapol']
    else:
        dflags = None
        mflags = None
    if args.verbose > 2:
        print "   data ",
    merge.write_data(merge.get_filename(), bool_to_int=True, dir=args.destdir,
            header=args.header, flags=dflags)
    qmax = max([i[1] for i in merge.get_flag_intervals('eextrapol')])
    if args.enoextrapolate:
        qmin = min([i[0] for i in merge.get_flag_intervals('eextrapol')])
    else:
        qmin=0
    if args.npoints >0:
        if args.verbose > 2:
            print " mean ",
        merge.write_mean(merge.get_filename(), bool_to_int=True,
                dir=args.destdir, qmin=qmin, qmax=qmax, header=args.header,
                flags=mflags, num=args.npoints, average=args.eaverage,
                verbose=(args.verbose > 2))
    else:
        if args.verbose > 2:
            print " mean ",
        qvalues = qvals[where(qvals >= qmin)]
        qvalues = qvals[where(qvalues <= qmax)]
        merge.write_mean(merge.get_filename(), bool_to_int=True,
                dir=args.destdir, qvalues=qvalues, header=args.header,
                flags=mflags, average=args.eaverage, verbose=(args.verbose > 2))

def get_hessian_stats(profile, nmax):
    gp = profile._setup_gp(nmax)
    hessian = profile._get_hessian(gp)
    hessian_names = []
    for k in ['G','Rg','d','s','A','sigma2','tau','lambda']:
        if profile.particles[k].get_nuisance_is_optimized():
            hessian_names.append(k)
    return array(hessian), hessian_names

def write_summary_file(merge, profiles, args):
    fl=open(os.path.join(args.destdir,args.sumname),'w')
    #header
    fl.write("#STATISTICAL MERGE: SUMMARY\n\n")
    fl.write("Ran with the following arguments:\n")
    fl.write(os.path.basename(sys.argv[0]) + " "
            + " ".join(sys.argv[1:]) + "\n\n")
    #merge file
    if args.stop == "merging":
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
        fl.write("   mean function : %s\n" % merge.mean)
        data = merge.get_params()
        if args.eerror or (not args.enocomp):
            Hessian, Hess_names = get_hessian_stats(merge,args.elimit_hessian)
        else:
            Hessian = None
        if Hessian is not None:
            for i,k in enumerate(Hess_names):
                fl.write("   %s : %f +- %f\n" % (k,data[k],sqrt(Hessian[i][i])))
        else:
            for i in sorted(data.keys()):
                fl.write("   %s : %f\n" % (i,data[i]))
        fl.write("  Calculated Values\n")
        Q1Rg = sqrt((data['d']-data['s'])*(3-data['s'])/2)
        fl.write("   Q1 : %f\n" % (Q1Rg/data['Rg']))
        fl.write("   Q1.Rg : %f\n" % Q1Rg)
        fl.write("   I(0) : %f\n" % \
                        (merge.get_mean(qvalues=[0],
                                            average=args.eaverage)[0][1]))
        if not args.enocomp:
            fl.write("  Model Comparison : num_params -log10(Bayes Factor) "
                    "-log(Posterior) -log(Likelihood) BIC AIC\n")
            for i in merge.bayes:
                name = '*'+i if i==merge.mean else i
                fl.write("   %s : %d\t%f\t%f\t%f\t%f\t%f\n" %
                        (name, merge.bayes[i][0], merge.bayes[i][8]/log(10),
                            merge.bayes[i][4], merge.bayes[i][5],
                            -2*merge.bayes[i][5]
                             +merge.bayes[i][0]*log(len(merge.get_raw_data())),
                            -2*merge.bayes[i][5]+2*merge.bayes[i][0]))
            fl.write("  Model Comparison : best model\n")
            fl.write("   Name : %s\n" % merge.mean)
            fl.write("   Number of parameters : %d\n" %
                                merge.bayes[merge.mean][0])
            fl.write("   -log(Posterior) : %f\n" %
                                merge.bayes[merge.mean][4])
            fl.write("   -log(Likelihood) : %f\n" %
                                merge.bayes[merge.mean][5])
        if Hessian is not None:
            fl.write("  Hessian matrix : ")
            fl.write(" ".join(Hess_names))
            fl.write("\n")
            for i in Hessian:
                fl.write("   ")
                fl.write(" ".join(['{0: <12}'.format(s) for s in i]))
                fl.write("\n")
        fl.write("\n")
    else:
        fl.write("Merge step skipped\n\n")
    #individual profiles
    for i,p in enumerate(profiles):
        #general stuff
        fl.write("Input file %d\n" % i +
                 "  General\n" +
                 "   Filename: " + p.filename + "\n")
        data = p.get_raw_data()
        fl.write("   Number of points: %d \n" % len(data) +
                 "   Data range: %.5f %.5f \n" % (data[0][0],data[-1][0]))
        #cleanup
        if not args.postpone_cleanup:
            data = p.get_data(filter='agood',colwise=True)
            fl.write("  1. Cleanup\n" +
                     "   Number of significant points: %d \n" % \
                             len(data['q']) +
                     "   Data range: %.5f %.5f \n" % \
                             (data['q'][0],data['q'][-1]))
            if args.stop == "cleanup":
                fl.write("  Skipped further steps\n\n")
                continue
        #fitting
        data = p.get_params()
        fl.write("  2. GP parameters (values for non-rescaled curve)\n")
        fl.write("   mean function : %s\n" % p.mean)
        if args.berror or (not args.bnocomp):
            Hessian, Hess_names = get_hessian_stats(p,args.blimit_hessian)
        else:
            Hessian = None
        if Hessian is not None:
            for i,k in enumerate(Hess_names):
                fl.write("   %s : %f +- %f\n" % (k,data[k],
                                sqrt(Hessian[i][i])))
        else:
            for i in sorted(data.keys()):
                fl.write("   %s : %f\n" % (i,data[i]))
        fl.write("   Calculated Values\n")
        qrg = sqrt((data['d']-data['s'])*(3-data['s'])/2)
        fl.write("    Q1 : %f\n" % (qrg/data['Rg']))
        fl.write("    Q1.Rg : %f\n" % qrg)
        fl.write("    I(0) : %f\n" % (p.get_mean(qvalues=[0],
                                                average=args.eaverage)[0][1]))
        if Hessian is not None:
            fl.write("   Hessian matrix : ")
            fl.write(" ".join(Hess_names))
            fl.write("\n")
            for i in Hessian:
                fl.write("    ")
                fl.write(" ".join(['{0: <12}'.format(s) for s in i]))
                fl.write("\n")
        if args.stop == "fitting":
            fl.write("  Skipped further steps\n\n")
            continue
        #rescaling
        data = p.get_gamma()
        fl.write("  3. Rescaling\n")
        if args.cmodel == 'normal-offset':
            fl.write("  normal model with offset\n")
            fl.write("   offset : %f\n" % p.get_offset())
        elif args.cmodel == 'normal':
            fl.write("  normal model\n")
        else:
            fl.write("  lognormal model\n")
        fl.write("   gamma : %s\n" % data)
        if args.stop == "rescaling":
            fl.write("  Skipped further steps\n\n")
            continue
        #cleanup (if it has been postponed)
        if args.postpone_cleanup:
            data = p.get_data(filter='agood',colwise=True)
            fl.write("  1. Cleanup (postponed)\n" +
                     "   Number of significant points: %d \n" % \
                             len(data['q']) +
                     "   Data range: %.5f %.5f \n" % \
                             (data['q'][0],data['q'][-1]))
            if args.stop == "cleanup":
                fl.write("  Skipped further steps\n\n")
                continue
        #classification
        data = p.get_data(filter='dgood',colwise=True)
        if data == {}:
            fl.write("  4. Classification\n" +
                 "   Number of valid points: 0\n")
        else:
            fl.write("  4. Classification\n" +
                 "   Number of valid points: %d \n" % len(data['q']) +
                 "   Data range: %.5f %.5f \n" % (data['q'][0],data['q'][-1]))
        fl.write("\n")
    #command-line arguments
    fl.write("List of all options used for the merge:\n")
    fl.write(' name (type) = "value"\n')
    for name in [ i for i in dir(args) if
                (not i.startswith('_'))
            and (not i in ['ensure_value','read_file','read_module'] ) ] :
        fl.write(" %s " % name)
        val = getattr(args,name)
        fl.write("(%s)" % type(val).__name__)
        fl.write('= "%s"\n' % val)


def initialize():
    args, files = parse_args()
    if args.verbose >= 2 :
        print "Parsing files and creating profile classes"
        print ""
    filenames,Nreps = parse_filenames(files, defaultvalue=10)
    args.filenames = filenames
    args.Nreps = Nreps
    scale = get_global_scaling_factor(filenames[-1])
    profiles = map(create_profile, filenames, Nreps, [scale]*len(filenames))
    #args.bschedule = parse_schedule(args.bschedule)
    #args.eschedule = parse_schedule(args.eschedule)
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
        print "1. cleanup ( alpha = %2G %% )" % (alpha*100)
    #loop over profiles
    good_profiles=[]
    for p in profiles:
        if verbose > 1:
            print "   ",p.filename,
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
            if verbose > 2:
                qvals = p.get_data(filter="agood", colwise=True)['q']
                print "\tqmin = %.3f\tqmax = %.3f" % (qvals[0], qvals[-1])
            elif verbose == 2:
                print ""
            good_profiles.append(p)
    #need a continuous indicator of validity
    for p in good_profiles:
        create_intervals_from_data(p, 'agood')
    return good_profiles, args

def fitting(profiles, args):
    """second stage of merge: gp fitting
    sets and optimizes the interpolant and enables calls to get_mean()
    """
    #schedule = args.bschedule
    verbose = args.verbose
    maxpointsF = args.blimit_fitting
    maxpointsH = args.blimit_hessian
    mean_function = args.bmean
    model_comp = not args.bnocomp
    if verbose >0:
        print "2. fitting"
    for p in profiles:
        if verbose > 1:
            print "   ",p.filename,
        try:
            data = p.get_data(filter='agood',colwise=True, maxpoints=maxpointsF)
        except KeyError:
            data = p.get_data(colwise=True, maxpoints=maxpointsF)
        if verbose > 1:
            if len(data['q']) == maxpointsF:
                print " (subsampled fitting: %d points) " % maxpointsF,
            if model_comp and maxpointsH > 0 and maxpointsH < len(data['q']):
                print " (subsampled hessian: %d points) " % maxpointsH,
        data['N'] = p.get_Nreps()
        initvals={}
        initvals['G']=1. #will be calculated properly
        initvals['Rg']=30. #same
        initvals['d']=args.bd
        initvals['s']=args.bs
        initvals['A']=0.
        initvals['tau']=1.
        initvals['lambda']=1.
        initvals['sigma2']=1.
        mean, initvals, bayes = find_fit(data, initvals, #schedule,
                verbose, model_comp=model_comp, model_comp_maxpoints=maxpointsH,
                mean_function=mean_function,
                lambdamin=args.lambdamin)
        model, particles, functions, gp = setup_process(data,initvals,1)
        if bayes:
            p.set_interpolant(gp, particles, functions, mean, model, bayes,
                    hessian=bayes[mean][2])
        else:
            p.set_interpolant(gp, particles, functions, mean, model, bayes)
        if verbose > 1 and model_comp:
            print "    => "+mean
    return profiles, args

def rescaling(profiles, args):
    """third stage of merge: rescaling
    Created flag:
        cgood : True if data point is both valid (wrt SNR) and in the validity
                domain of the rescaling reference curve (option --creference)
    sets profile.gamma to the correct value
    """
    use_normal = args.cmodel.startswith('normal')
    use_offset = use_normal and args.cmodel.endswith('offset')
    numpoints = args.cnpoints
    verbose = args.verbose
    reference = args.creference
    average = args.baverage
    if verbose >0:
        print "3. rescaling",
        if use_normal:
            print "(normal model",
        else:
            print "(lognormal model",
        if use_offset:
            print "with constant offset)"
        else:
            print "without constant offset)"
    #take last as internal reference as there's usually good overlap
    pref = profiles[-1]
    gammas = []
    for ctr,p in enumerate(profiles):
        #find intervals where both functions are valid
        p.new_flag('cgood',bool)
        pdata = p.get_data(colwise=True)
        if 'agood' in pdata: #assumed true for pref also
            for id,q,flag in zip(pdata['id'],pdata['q'],pdata['agood']):
                refflag = pref.get_flag(q,'agood')
                p.set_flag(id, 'cgood', flag and refflag)
        else: #assumed false for pref also
            refq = pref.get_data(colwise=True)['q']
            for id,q in zip(pdata['id'],pdata['q']):
                refflag = (refq[0] <= q <= refq[-1])
                p.set_flag(id, 'cgood', refflag)
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
        pvalues = p.get_mean(qvalues=qvalues, colwise=True, average=average)
        if True in map(isnan,pvalues['I']):
            raise RuntimeError, "Got NAN in I"
        if True in map(isnan,pvalues['err']):
            raise RuntimeError, "Got NAN in err"
        prefvalues = pref.get_mean(qvalues=qvalues, colwise=True,
                        average=average)
        if True in map(isnan,prefvalues['I']):
            raise RuntimeError, "Got NAN in ref I"
        if True in map(isnan,prefvalues['err']):
            raise RuntimeError, "Got NAN in ref err"
        gammas.append(rescale_curves(prefvalues, pvalues,
            normal = use_normal, offset = use_offset))
        #fl=open('rescale_%d.npy' % ctr, 'w')
        #import cPickle
        #cPickle.dump([pvalues,prefvalues],fl)
        #fl.close()
        #fl=open('rescale_%d.dat' % ctr, 'w')
        #for i in xrange(len(pvalues['q'])):
        #    fl.write("%s " % pvalues['q'][i])
        #    fl.write("%s " % pvalues['I'][i])
        #    fl.write("%s " % pvalues['err'][i])
        #    fl.write("%s " % pvalues['q'][i])
        #    fl.write("%s " % (gammas[-1][0]*pvalues['I'][i]))
        #    fl.write("%s " % (gammas[-1][0]*pvalues['err'][i]))
        #    fl.write("%s " % prefvalues['q'][i])
        #    fl.write("%s " % prefvalues['I'][i])
        #    fl.write("%s\n" % prefvalues['err'][i])
    #set gammas wrt reference
    if reference == 'first':
        gr=gammas[0][0]
        cr=gammas[0][1]
    else:
        gr = gammas[-1][0]
        cr = gammas[-1][1]
    for p,g in zip(profiles,gammas):
        gamma = g[0]/gr
        c = g[1] - cr
        p.set_gamma(gamma)
        p.set_offset(c)
        if verbose >1:
            print "   ",p.filename,"   gamma = %.3G" % gamma,
            if use_offset:
                print "   offset = %.3G" % c,
            print ""
    if True in map(isnan,gammas[-1]):
        raise RuntimeError, "Got NAN in rescaling step, try another rescaling " \
                            "or fitting model."
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
    average=args.baverage
    if verbose >0:
        print "4. classification ( alpha = %2G %% )" % (alpha*100)
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
            data = p.get_mean(qvalues=[entry[1]],colwise=True, average=average)
            refdata = pref.get_mean(qvalues=[entry[1]],colwise=True,
                    average=average)
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
    #schedule = args.eschedule
    verbose = args.verbose
    maxpointsF = args.elimit_fitting
    maxpointsH = args.elimit_hessian
    do_extrapolation = not args.enoextrapolate
    extrapolate = 1+args.eextrapolate/float(100)
    mean_function = args.emean
    model_comp = not args.enocomp
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
            print "    ",p.filename
        #get data and keep only relevant flags
        data = p.get_data(filter='dgood',colwise=True)
        if data == {}: #no good datapoints
            continue
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
        print "   calculating merged mean",
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
    #it's the bet we can do when fitting all points simultaneously
    merge.set_Nreps(min([p.get_Nreps() for p in profiles]))
    #fit curve
    data = merge.get_data(colwise=True, maxpoints=maxpointsF)
    if verbose > 1:
        if len(data['q']) == maxpointsF:
            print " (subsampled fitting: %d points) " % maxpointsF,
        if model_comp and maxpointsH > 0 and maxpointsH < len(data['q']):
            print " (subsampled hessian: %d points) " % maxpointsH,
    data['N'] = merge.get_Nreps()
    #take initial values from the curve which has gamma == 1
    initvals = profiles[-1].get_params()
    mean, initvals, bayes = find_fit(data, initvals, #schedule,
                    verbose, model_comp=model_comp,
                    model_comp_maxpoints=maxpointsH,
                    mean_function=mean_function,
                    lambdamin=args.lambdamin)
    if verbose > 1 and model_comp:
        print "    => "+mean
    model, particles, functions, gp = setup_process(data,initvals,1)
    if bayes:
        merge.set_interpolant(gp, particles, functions, mean, model, bayes,
                hessian=bayes[mean][2])
    else:
        merge.set_interpolant(gp, particles, functions, mean, model, bayes)
    if verbose > 0:
        print ""
    return merge, profiles, args

def write_data(merge, profiles, args):
    if args.verbose > 0:
        print "writing data"
    if not os.path.isdir(args.destdir):
        os.mkdir(args.destdir)
    qvals = array(profiles[0].get_raw_data(colwise=True)['q'])
    #individual profiles
    if args.allfiles:
        if args.verbose > 1:
            print "   individual profiles"
        for i in profiles:
            if args.verbose > 2:
                print "    %s" % (i.get_filename())
            write_individual_profile(i, qvals, args)
    #merge profile
    if args.stop == 'merging':
        if args.verbose > 1:
            print "   merge profile",
        write_merge_profile(merge, qvals, args)
        if args.verbose > 1:
            print ""
    #summary
    if args.verbose > 1:
        print "   summary"
    write_summary_file(merge, profiles, args)
    if args.verbose > 0:
        print "Done."

def main():
    #get arguments and create profiles
    profiles, args = initialize()
    #create list of steps to do, in order
    if args.postpone_cleanup:
        steps_to_go = ["fitting","rescaling","cleanup",
                        "classification","merging"]
    else:
        steps_to_go = ["cleanup","fitting","rescaling",
                        "classification","merging"]
    steps_to_go = steps_to_go[:steps_to_go.index(args.stop)+1]
    #call steps in turn
    merge = None
    for step in steps_to_go:
        if step != 'merging':
            profiles, args = globals()[step](profiles, args)
        else:
            merge, profiles, args = merging(profiles, args)
    #write output
    write_data(merge, profiles, args)

if __name__ == "__main__":
    main()
