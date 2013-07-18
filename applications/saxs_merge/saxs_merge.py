#!/usr/bin/env python

import sys,os
import optparse
import fractions
from random import sample
from numpy import *
import copy
from scipy.stats import t as student_t
from scipy import linalg,optimize
import glob
import tempfile

import IMP
import IMP.isd
import IMP.gsl
import IMP.saxs

#IMP.base.set_log_level(IMP.base.SILENT)
IMP.base.set_log_level(IMP.base.VERBOSE)
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
            if not IMP.isd.Nuisance.get_is_setup(p):
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
                    self.particles['s'].set_lower(0)
                    self.particles['s'].set_nuisance(0)
                    self.particles['s'].set_upper(0)
                else:
                    self.particles['s'].set_nuisance_is_optimized(True)
        self.particles['tau'].set_nuisance_is_optimized(True)
        self.particles['lambda'].set_nuisance_is_optimized(True)
        self.particles['sigma2'].set_nuisance_is_optimized(True)
        if self.recompute_hessian is False:
            return self._memoized_hessian
        gpr=IMP.isd.GaussianProcessInterpolationRestraint(self.model,gp)
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
            num qmin qmax filter average verbose qvalues
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
            Hchol = linalg.cho_factor(Hessian)
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
                                  + linalg.cho_solve(Hcho,Hlcov))**(-0.5)
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

    def get_cov(self, qvals):
        """returns the covariance matrix for the provided q values.
        No posterior averaging is performed as it is too costy.
        """
        gamma = self.get_gamma()
        M=len(qvals)
        retval=zeros((M,M))
        for i,q1 in enumerate(qvals):
            for j,q2 in enumerate(qvals[:i+1]):
                retval[i,j] = \
                    gamma**2*self.gp.get_posterior_covariance([q1],[q2])
                if j!=i:
                    retval[j,i]=retval[i,j]
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
    group.add_option('--auto', help="Fully automatic merge. Will try to find"
            " correct order for input files (default is off)",
            default=False, action='store_true')
    group.add_option('--remove_noisy', help="Remove data points that have"
            " too large error bars, such as data at very low q (default is not "
            " to perform this cleanup) ", default=False, action='store_true')
    group.add_option('--remove_redundant', help="Remove high noise data if it "
            " is surrounded by low noise data (default is not to remove it)",
            default=False, action='store_true')
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
    #cleanup
    group = optparse.OptionGroup(parser, title="Cleanup (Step 1)",
                              description="Discard or keep SAXS curves' "
                              "points based on their SNR. Points with an error"
                              " of zero are discarded as well")
    parser.add_option_group(group)

    group.add_option('--aalpha', help='type I error (default 0.05 before '
            'Bonferroni correction)', type="float", default=0.05,
            metavar='ALPHA')
    group.add_option('--acutoff', help='when a value after CUT is discarded,'
            ' the rest of the curve is discarded as well (default is 0.1)',
            type="float", default=0.1, metavar='CUT')
    #fitting
    group = optparse.OptionGroup(parser, title="Fitting (Step 2)",
                description="Estimate the mean function and the noise level "
                "of each SAXS curve.")
    parser.add_option_group(group)
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

def setup_particles():
    """
    returns: model, dict(*particles), dict(mean, covariance)
    """

    model = IMP.Model()
    #mean function
    G=IMP.isd.Scale.setup_particle(IMP.Particle(model,"G"))
    #model.add_restraint(IMP.isd.JeffreysRestraint(G))
    Rg=IMP.isd.Scale.setup_particle(IMP.Particle(model,"Rg"))
    d=IMP.isd.Scale.setup_particle(IMP.Particle(model,"d"))
    s=IMP.isd.Scale.setup_particle(IMP.Particle(model,"s"))
    A=IMP.isd.Nuisance.setup_particle(IMP.Particle(model,"A"))
    m = IMP.isd.GeneralizedGuinierPorodFunction(G,Rg,d,s,A)
    #covariance function
    tau=IMP.isd.Scale.setup_particle(IMP.Particle(model,"tau"))
    lam=IMP.isd.Scale.setup_particle(IMP.Particle(model,"lambda"))
    jitter=0.
    w = IMP.isd.Covariance1DFunction(tau,lam,2.0,jitter)
    #sigma
    sigma=IMP.isd.Scale.setup_particle(IMP.Particle(model,"sigma2"))
    #prior on scales
    model.add_restraint(IMP.isd.JeffreysRestraint(tau))
    model.add_restraint(IMP.isd.JeffreysRestraint(sigma))
    model.add_restraint(IMP.isd.JeffreysRestraint(lam))
    #set upper bounds for s
    s.set_upper(3)
    s.set_upper(d)
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

def setup_process(data, subs, maxpoints=-1):
    model,particles,functions = setup_particles()
    q = [ [i] for i in data['q'][::subs]]
    I = data['I'][::subs]
    err = data['err'][::subs]
    if maxpoints >0:
        q,I,err = zip(*subsample(q,zip(q,I,err),maxpoints))
    gp = IMP.isd.GaussianProcessInterpolation(q, I, err,
            data['N'], functions['mean'], functions['covariance'],
            particles['sigma2'])
    return model, particles, functions, gp

def _get_guinier_fit_fixed(qs,Is,errs):
    X=array([ones(len(qs)),qs**2]).transpose()
    W=diagflat(errs/Is)
    y=atleast_2d(log(Is)).transpose()
    #weighted least squares estimator
    #beta = (X^T W X )^(-1) X^T W y
    XW=dot(X.transpose(),W)
    try:
        beta = linalg.solve( dot(XW,X), dot(XW,y), sym_pos=True)
    except linalg.LinAlgError:
        print "Warning: singular matrix during guinier fit"
        beta=atleast_2d(ones(2)).transpose()
    if beta[1,0]>0:
        rg=0
    else:
        rg=sqrt(-3*beta[1,0])
    izero = exp(beta[0,0])
    return rg,izero

def get_guinier_fit(data, end_q_rg=1.3):
    qs,Is,errs = map(array, [data['q'],data['I'],data['err']])
    #fit the whole range up to q_rg, take median values for robustness
    cut=5 #drop first 5 points, and take at least 5 points to estimate
    vals = []
    for i in xrange(cut+5,len(qs)):
        rg,izero = _get_guinier_fit_fixed(qs[cut:i],Is[cut:i],errs[cut:i])
        if rg >0:
            vals.append((rg,izero,qs[i-1]*rg))
            if qs[i-1]*rg > end_q_rg:
                break
    vals=array(vals).transpose()
    rg = median(vals[0])
    izero = median(vals[1])
    return rg,izero

def set_defaults_mean(data, particles, mean_function):
    Rg,I0 = get_guinier_fit(data)
    particles['G'].set_lower(I0/5.)
    particles['G'].set_upper(5*I0)
    particles['G'].set_nuisance(I0)
    particles['Rg'].set_lower(Rg/5.)
    particles['Rg'].set_upper(5*Rg)
    particles['Rg'].set_nuisance(Rg)
    #
    particles['A'].set_nuisance_is_optimized(True)
    if mean_function == 'Flat':
        particles['G'].set_lower(0)
        particles['G'].set_upper(0)
        particles['G'].set_nuisance(0)
        particles['A'].set_nuisance(mean(data['I']))
        particles['G'].set_nuisance_is_optimized(False)
        particles['Rg'].set_nuisance_is_optimized(False)
        particles['d'].set_nuisance_is_optimized(False)
        particles['s'].set_nuisance_is_optimized(False)
    else:
        particles['A'].set_nuisance(mean(data['I'][-10:]))
        particles['G'].set_nuisance_is_optimized(True)
        particles['Rg'].set_nuisance_is_optimized(True)
        if mean_function == 'Simple':
            particles['d'].set_nuisance_is_optimized(False)
            dmin=2/3.*(max(data['q'])*particles['Rg'].get_nuisance())**2
            particles['d'].set_lower(dmin)
            particles['d'].set_nuisance(dmin)
            particles['d'].set_upper(dmin+1)
            particles['s'].set_lower(0)
            particles['s'].set_upper(0)
            particles['s'].set_nuisance(0)
        else:
            particles['d'].set_nuisance_is_optimized(True)
            particles['d'].set_lower(0)
            particles['d'].set_upper(8)
            particles['d'].set_nuisance(4)
            if mean_function == 'Generalized':
                particles['s'].set_nuisance_is_optimized(False)
                particles['s'].set_lower(0)
                particles['s'].set_upper(0)
                particles['s'].set_nuisance(0)
            else:
                particles['s'].set_nuisance_is_optimized(True)
                particles['s'].set_upper(3.)
                particles['s'].set_upper(particles['d'])
    particles['tau'].set_nuisance_is_optimized(False)
    particles['lambda'].set_nuisance_is_optimized(False)
    particles['sigma2'].set_nuisance_is_optimized(False)

def set_defaults_cov(data,particles):
    particles['tau'].set_nuisance_is_optimized(True)
    particles['lambda'].set_nuisance_is_optimized(True)
    particles['sigma2'].set_nuisance_is_optimized(True)
    meandist = mean(array(data['q'][1:])-array(data['q'][:-1]))
    lambdalow=0
    particles['lambda'].set_lower(max(2*meandist,lambdalow))
    particles['lambda'].set_upper(1e8)
    particles['lambda'].set_nuisance(max(data['q'])/10.)
    particles['sigma2'].set_lower(1e-8)
    particles['sigma2'].set_upper(1e8)
    particles['sigma2'].set_nuisance(10.)
    particles['tau'].set_lower(sqrt(1e-2))
    particles['tau'].set_upper(sqrt(1e8))
    particles['tau'].set_nuisance(sqrt(10))

def transform(particle):
    """return
    tan(pi/2*((d-dmin)-(dmax-d))/(dmax-dmin))
    where d is the particle's value, and dmin,dmax are its bounds
    """
    d=particle.get_nuisance()
    dmin=particle.get_lower()
    dmax=particle.get_upper()
    return tan(pi/2*((d-dmin)-(dmax-d))/(dmax-dmin))

def untransform(x,particle):
    """return the particle's value
    (dmax-dmin)/pi * arctan(x) + (dmax+dmin)/2.
    where x is the transform, and dmin,dmax are the particle's bounds
    """
    dmin=particle.get_lower()
    dmax=particle.get_upper()
    return (dmax-dmin)/pi * arctan(x) + (dmax+dmin)/2.

def untransform_d(x,particle):
    """return the reverse transform derivative
    (dmax-dmin)/(pi * (1+x**2))
    where x is the transform, and dmin,dmax are the particle's bounds
    """
    dmin=particle.get_lower()
    dmax=particle.get_upper()
    return (dmax-dmin)/(pi * (1+x**2))

def residuals(point, mean, particles, mean_fn, data):
    if mean == 'Flat':
        A = point
    elif mean == 'Simple':
        A, mu, mv = point
    elif mean == 'Generalized':
        A, mu, mv, mw = point
    elif mean == 'Full':
        A, mu, mv, mw, mx = point
    else:
        raise ValueError, "unknown mean function %s" % mean
    particles['A'].set_nuisance(A)
    if mean in ['Simple','Generalized','Full']:
        particles['G'].set_scale(untransform(mu, particles['G']))
        particles['Rg'].set_scale(untransform(mv, particles['Rg']))
    if mean in ['Generalized','Full']:
        particles['d'].set_scale(untransform(mw, particles['d']))
    if mean == 'Full':
        particles['s'].set_scale(untransform(mx, particles['s']))
    #
    mean_fn.update()
    retval = (array([(I-mean_fn([q])[0])/err
                    for q,I,err in zip(data['q'],data['I'],data['err'])
            ])**2).sum()
    return retval

def target_function(point, mean, particles, model, gpr, M, deriv=False):
    if mean == 'cov':
        cu, cv, cl = point
    elif mean == 'Flat':
        A, cu, cv, cl = point
    elif mean == 'Simple':
        A, mu, mv, cu, cv, cl = point
    elif mean == 'Generalized':
        A, mu, mv, mw, cu, cv, cl = point
    elif mean == 'Full':
        A, mu, mv, mw, mx, cu, cv, cl = point
    else:
        raise ValueError, "unknown mean function %s" % mean
    particles['tau'].set_scale(untransform(cu, particles['tau']))
    particles['sigma2'].set_scale(untransform(cv, particles['sigma2']))
    particles['lambda'].set_scale(untransform(cl, particles['lambda']))
    if mean != 'cov':
        particles['A'].set_nuisance(A)
    if mean in ['Simple','Generalized','Full']:
        particles['G'].set_scale(untransform(mu, particles['G']))
        particles['Rg'].set_scale(untransform(mv, particles['Rg']))
    if mean in ['Generalized','Full']:
        particles['d'].set_scale(untransform(mw, particles['d']))
    if mean == 'Full':
        particles['s'].set_scale(untransform(mx, particles['s']))
    #
    retval = model.evaluate(deriv)
    if not deriv:
        return retval
    dtau = particles['tau'].get_nuisance_derivative()
    dsi2 = particles['sigma2'].get_nuisance_derivative()
    dlam = particles['lambda'].get_nuisance_derivative()
    dcu = untransform_d(cu, particles['tau'])*dtau
    dcv = untransform_d(cv, particles['sigma2'])*dsi2
    dcl = untransform_d(cl, particles['lambda'])*dlam
    if mean == 'cov':
        return retval, array([dcu,dcv,dcl])
    dA = particles['A'].get_nuisance_derivative()
    if mean == 'Flat':
        return retval, array([dA,dcu,dcv,dcl])
    dG = particles['G'].get_nuisance_derivative()
    dRg = particles['Rg'].get_nuisance_derivative()
    dmu = untransform_d(mu, particles['G'])*dG
    dmv = untransform_d(mv, particles['Rg'])*dRg
    if mean == 'Simple':
        return retval, array([dA,dmu,dmv,dcu,dcv,dcl])
    dd = particles['d'].get_nuisance_derivative()
    dmw = untransform_d(mw, particles['d'])*dd
    if mean == 'Generalized':
        return retval, array([dA,dmu,dmv,dmw,dcu,dcv,dcl])
    ds = particles['s'].get_nuisance_derivative()
    dmx = untransform_d(mx, particles['s'])*ds
    return retval, array([dA,dmu,dmv,dmw,dmx,dcu,dcv,dcl])

def find_fit_mean(data, verbose, mean_function):
    model, particles, functions, gp = \
            setup_process(data, 1)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(model,gp)
    model.add_restraint(gpr)
    #
    set_defaults_mean(data, particles, mean_function)
    #
    options={'maxiter':100, 'disp':False}
    #
    A0 = particles['A'].get_nuisance()
    if mean_function == 'Flat':
        res = optimize.minimize(residuals,
                array([A0]), method='Nelder-Mead',
            args=(mean_function, particles, functions['mean'], data),
            jac=False, options=options)
        Af = res.x
    else:
        mu0 = transform(particles['G'])
        mv0 = transform(particles['Rg'])
        #only fit guinier region for now
        dval = particles['d'].get_nuisance()
        dlo = particles['d'].get_lower()
        dmin=2/3.*(max(data['q'])*particles['Rg'].get_nuisance())**2
        particles['d'].set_lower(dmin)
        particles['d'].set_nuisance(dmin)
        particles['d'].set_upper(dmin+1)
        res = optimize.minimize(residuals,
            array([A0,mu0,mv0]), method='Nelder-Mead',
            args=('Simple', particles, functions['mean'], data),
                jac=False, options=options)
        Af,muf,mvf = res.x
        #relax bounds on d
        particles['d'].set_lower(0)
        particles['d'].set_upper(max(dmin+1,8))
        #
        if mean_function != 'Simple':
            mw0 = transform(particles['d'])
            res = optimize.minimize(residuals,
                array([Af,muf,mvf,mw0]), method='Nelder-Mead',
                args=('Generalized', particles, functions['mean'], data),
                jac=False, options=options)
            Af,muf,mvf,mwf = res.x
            if mean_function != 'Generalized':
                options={'maxiter':300, 'disp':False}
                particles['s'].set_nuisance(.1)
                mx0 = transform(particles['s'])
                res = optimize.minimize(residuals,
                    array([Af,muf,mvf,mwf,mx0]), method='Nelder-Mead',
                    args=('Full', particles, functions['mean'], data),
                    jac=False, options=options)
                Af,muf,mvf,mwf,mxf = res.x
                particles['s'].set_nuisance(untransform(mxf,particles['s']))
            particles['d'].set_nuisance(untransform(mwf,particles['d']))
        particles['Rg'].set_nuisance(untransform(mvf,particles['Rg']))
        particles['G'].set_nuisance(untransform(muf,particles['G']))
    particles['A'].set_nuisance(Af)
    #
    newvals = dict([(k,v.get_nuisance())
                    for (k,v) in particles.iteritems()])
    ene = model.evaluate(False)
    return newvals, ene

def find_fit_all(data, initvals, verbose, mean_function):
    model, particles, functions, gp = \
            setup_process(data, 1)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(model,gp)
    model.add_restraint(gpr)
    set_defaults_mean(data, particles, mean_function)
    for p,v in initvals.items():
        particles[p].set_nuisance(v)
    set_defaults_cov(data, particles)
    M=len(data['I'])
    #
    cu0 = transform(particles['tau'])
    cv0 = transform(particles['sigma2'])
    cl0 = transform(particles['lambda'])
    #
    #
    #first fit covariance particles only
    options={'maxiter':100, 'disp':False}
    res = optimize.minimize(target_function,
            array([cu0,cv0,cl0]), method='Nelder-Mead',
        args=('cov', particles, model, gpr, M, False),
        jac=False, options=options)
    cuf,cvf,clf = res.x
    #refine all particles
    options={'maxiter':100, 'disp':False,'gtol':1e-5}
    A0 = particles['A'].get_nuisance()
    if mean_function == 'Flat':
        res = optimize.minimize(target_function,
                array([A0,cuf,cvf,clf]), method='BFGS',
            args=(mean_function, particles, model, gpr, M, True),
            jac=True, options=options)
        Af,cuf,cvf,clf = res.x
    else:
        mu0 = transform(particles['G'])
        mv0 = transform(particles['Rg'])
        if mean_function == 'Simple':
            res = optimize.minimize(target_function,
                array([A0,mu0,mv0,cuf,cvf,clf]), method='BFGS',
                args=(mean_function, particles, model, gpr, M, True),
                jac=True, options=options)
            Af,muf,mvf,cuf,cvf,clf = res.x
        else:
            mw0 = transform(particles['d'])
            if mean_function == 'Generalized':
                res = optimize.minimize(target_function,
                    array([A0,mu0,mv0,mw0,cuf,cvf,clf]), method='BFGS',
                    args=(mean_function, particles, model, gpr, M, True),
                    jac=True, options=options)
                Af,muf,mvf,mwf,cuf,cvf,clf = res.x
            else:
                mx0 = transform(particles['s'])
                res = optimize.minimize(target_function,
                    array([A0,mu0,mv0,mw0,mx0,cuf,cvf,clf]), method='BFGS',
                    args=(mean_function, particles, model, gpr, M, True),
                    jac=True, options=options)
                Af,muf,mvf,mwf,mxf,cuf,cvf,clf = res.x
                particles['s'].set_nuisance(untransform(mxf,particles['s']))
            particles['d'].set_nuisance(untransform(mwf,particles['d']))
        particles['Rg'].set_nuisance(untransform(mvf,particles['Rg']))
        particles['G'].set_nuisance(untransform(muf,particles['G']))
    particles['A'].set_nuisance(Af)
    #
    particles['tau'].set_scale(untransform(cuf, particles['tau']))
    particles['sigma2'].set_scale(untransform(cvf, particles['sigma2']))
    particles['lambda'].set_scale(untransform(clf, particles['lambda']))
    #
    newvals = dict([(k,v.get_nuisance())
                    for (k,v) in particles.iteritems()])
    ene = model.evaluate(False)
    #make sure the errors are defined
    if not res.success:
        ene = inf
    else:
        for q in data['q']:
            err = gp.get_posterior_covariance([q],[q])
            if err <0 or isnan(err) or isinf(err):
                ene=inf
                break
    return newvals, ene

def bayes_factor(data, initvals, verbose, mean_func, maxpoints,
        calc_hessian=True):
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
    model, particles, functions, gp = setup_process(data, 1, maxpoints=maxpoints)
    gpr = IMP.isd.GaussianProcessInterpolationRestraint(model,gp)
    model.add_restraint(gpr)
    set_defaults_mean(data, particles, mean_func)
    set_defaults_cov(data, particles)
    for p,v in initvals.items():
        particles[p].set_nuisance(v)
    #
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
            if isinf(retval):
                print "Warning: re-skipping model %s" % mean_func
                logdet = inf
            else:
                logdet = retval/2.
        except IMP.base.ModelException:
            print "Warning: Hessian is not positive definite"
            logdet=inf
    return (Np, (2*pi)**(Np/2.), H, logdet, MP, ML, MP-ML,
            exp(-MP)*(2*pi)**(Np/2.)*exp(-logdet),
            MP - Np/2.*log(2*pi) + logdet)

def find_fit(data, verbose, model_comp=False, model_comp_maxpoints=-1,
        mean_function='Simple'):
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
            sys.stdout.write("     "+mean_func+": ")
            sys.stdout.flush()
        meanvals, ene = find_fit_mean(data, verbose, mean_func)
        endvals, ene = find_fit_all(data, meanvals, verbose, mean_func)
        param_vals[mean_func] = endvals
        if mean_func not in param_vals:
            if model_comp:
                print "error"
                sys.stdout.flush()
                continue
            else:
                raise FittingError, "Error while fitting! Choose"\
                        " another model or do model comparison"
        if verbose > 2:
            sys.stdout.write("A=%1.3f " % param_vals[mean_func]['A'])
            if mean_func in ["Simple","Generalized","Full"]:
                sys.stdout.write("G=%1.3f " % param_vals[mean_func]['G'])
                sys.stdout.write("Rg=%1.3f " % param_vals[mean_func]['Rg'])
            if mean_func in ["Generalized","Full"]:
                sys.stdout.write("d=%1.3f " % param_vals[mean_func]['d'])
            if mean_func == "Full":
                sys.stdout.write("s=%1.3f " % param_vals[mean_func]['s'])
            for i in ['tau','lambda','sigma2']:
                sys.stdout.write("%s=%1.3f " % (i,param_vals[mean_func][i]))
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
    #must have at least one model with admissible bayes factor
    if not (False in map(isinf, [free_energies[f][8] for f in functions])):
        raise FittingError, "Error while fitting! All Bayes factors are "\
             "undefined.\n"\
             "Change initial parameters or disable model comparison."
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

def rescale_curves(I0, I1, Sigma1, normal = False, offset = False):
    """Find gamma and (optionnally) c so that
    normal model: I0 = gamma*(I1 + c)
    lognormal model log I0 = log(gamma*I1)
    if offset==False we have c=0
    """
    I0 = atleast_2d(I0).transpose()
    I1 = atleast_2d(I1).transpose()
    M=I1.shape[0]
    J = ones(I1.shape)
    try:
        Scho = linalg.cho_factor(Sigma1)
        mat = 1
    except linalg.LinAlgError:
        try:
            Sigma2 = Sigma1 + eye(M)*max(Sigma1.diagonal())*0.05
            Scho = linalg.cho_factor(Sigma2)
            mat = 2
        except linalg.LinAlgError:
            try:
                Sigma3 = zeros(Sigma1.shape)
                Sigma3[diag_indices_from(Sigma3)] = Sigma1.diagonal()
                Scho = linalg.cho_factor(Sigma3)
                mat = 3
            except linalg.LinAlgError:
                try:
                    Sigma4 = zeros(Sigma1.shape)
                    Sigma4[diag_indices_from(Sigma4)] = Sigma1.diagonal()
                    Sigma4 += eye(M)*max(Sigma1.diagonal())*0.05
                    Scho = linalg.cho_factor(Sigma4)
                    mat = 4
                except linalg.LinAlgError:
                    Sigma5 = zeros(Sigma1.shape)
                    Sigma5[diag_indices_from(Sigma5)] = abs(Sigma1.diagonal())
                    Scho = linalg.cho_factor(Sigma5)
                    mat = 5

    if normal and offset:
        #normal with offset
        Su = linalg.cho_solve(Scho, I1).transpose()
        Sj = linalg.cho_solve(Scho, J)
        osj = dot(I0.transpose(),Sj)
        usu = dot(Su,I1)
        uso = dot(Su,I0)
        usj = dot(Su,J)
        jsj = dot(J.transpose(), Sj)
        c = float((M*osj+usu*osj-uso*usj)/(uso*jsj-usj*osj))
        gamma = float(uso/(M+usu+c*usj))
        return gamma,c,mat
    elif normal:
        #normal without offset
        Si1 = linalg.cho_solve(Scho, I1).transpose()
        gamma = float(dot(Si1,I0)/(dot(Si1,I1)+M))
        return gamma,0,mat
    elif not offset:
        #lognormal
        if (I0<0).any():
            raise ValueError, "negative entries in I0"
        if (I1<0).any():
            raise ValueError, "negative entries in I1"
        li = log(I0/I1).transpose()
        Sij = linalg.cho_solve(Scho, J)
        lg = float(dot(li,Sij)/dot(J.transpose(),Sij))
        gamma = exp(lg)
        return gamma,0,mat
    else:
        #lognormal with offset
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

def reorder_profiles(profiles):
    """try to guess which profile has predecence over which.
    criterion is increasing I(0) at constant noise level
    """
    Izeros=[]
    #fl=open(profiles[0].get_filename().split('/')[-2]+'.txt','w')
    #fl.write("id name Izero mnoise Izero/mnoise Rg G\n")
    for i,p in enumerate(profiles):
        #gamma = p.get_gamma()
        data = p.get_data(filter='agood',colwise=True)
        Rg,Izero = get_guinier_fit(data)
        mnoise = median(data['err'])
        Izeros.append((Izero/mnoise,i))
        """
        fl.write("%d\t" % i)
        fl.write("%s\t" % p.get_filename().split('/')[-1])
        fl.write("%f\t" % (Izero/gamma))
        fl.write("%f\t" % (mnoise/gamma))
        fl.write("%f\t" % (Izero/mnoise))
        fl.write("%f\n" % p.get_params()['Rg'])
        """
    Izeros.sort()
    newprofs = [ profiles[i] for q,i in Izeros ]
    return newprofs

def remove_redundant(profiles, verbose=0):
    if len(profiles) == 1:
        return profiles
    data = [i.get_data(filter='dgood',colwise=True) for i in profiles]
    #remove profile if it has no data
    tbd=[]
    for i,d in enumerate(data):
        if d.keys() == []:
            tbd.append(i)
    tbd.reverse()
    for i in tbd:
        profiles.pop(i)
        data.pop(i)
        if verbose>0:
            print "    removed profile %d because it is redundant" % i
    if len(profiles) <= 1:
        return profiles
    #map noise levels to profiles
    noiselevels = [(median(p['err']),i) for i,p in enumerate(data)]
    noiselevels.sort()
    n2p = [i[1] for i in noiselevels] #n2p[-1] is noisiest profile
    dbest = data[n2p[0]]
    #get median distance between q values
    qdist = median([ dbest['q'][i+1]-dbest['q'][i]
                        for i in xrange(len(dbest['q'])-1) ])
    for p in n2p[1:]:
        prof = profiles[p]
        dat = data[p]
        imin = 0
        for qbest in dbest['q']:
            #find qs in data
            while imin < len(dat['q']) and dat['q'][imin] <qbest-qdist:
                imin += 1
            #if this profile has no more data, stop loop
            if imin == len(dat['q']):
                break
            #if there is no data around qbest, continue
            if dat['q'][imin] > qbest+qdist:
                continue
            #find upper bound
            imax=imin+1
            while imax < len(dat['q']) and dat['q'][imax] <= qbest+qdist:
                imax += 1
            #loop over qs and discard them
            for i in xrange(imin,imax):
                prof.set_flag(dat['id'][i],'dgood', False)
                prof.set_flag(dat['id'][i],'dpvalue', -1)
    #apply same alg to remaining profiles
    pnew = [p for i,p in enumerate(profiles) if i != n2p[0] ]
    pnew = remove_redundant(pnew, verbose)
    pnew.insert(n2p[0], profiles[n2p[0]])
    return pnew

def write_fit(data, initvals, outfile, mean_function, npoints=200):
    fl=open(outfile,'w')
    fl.write("#q I err gp sd gp_mean\n")
    model, particles, functions, gp = setup_process(data, 1, jitter=0.)
    for p,v in initvals.items():
        particles[p].set_nuisance(v)
    qs,Is,errs = data['q'],data['I'],data['err']
    for q,I,err in zip(qs,Is,errs):
        fl.write("%s\t" % q)
        fl.write("%s\t" % I)
        fl.write("%s\t" % err)
        fl.write("%s\t" % gp.get_posterior_mean([q]))
        fl.write("%s\t" % sqrt(gp.get_posterior_covariance([q],[q])))
        fl.write("%s\n" % functions['mean']([q])[0])



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
        print "1. cleanup ( alpha = %2G %% / M " % (alpha*100),
        if args.remove_noisy:
            print "and removing noisy points",
        print ")"
    #loop over profiles
    good_profiles=[]
    for p in profiles:
        if verbose > 1:
            print "   ",os.path.basename(p.filename),
        N = p.get_Nreps()
        p.new_flag('agood',bool)
        p.new_flag('apvalue',float)
        #loop over individual points
        had_outlier = False
        data = p.get_data()
        M = len(data)
        for datum in data:
            id,q,I,err = datum[:4]
            if err == 0:
                p.set_flag(id,'agood', False)
                p.set_flag(id, 'apvalue', -1)
                continue
            pval,t = ttest_one(I,err,N)[0:2]
            if pval > alpha/M or had_outlier:  #the point is invalid
                p.set_flag(id, 'agood', False)
                if q >= q_cutoff and had_outlier == False:
                    had_outlier = True
            else:
                p.set_flag(id, 'agood', True)
            p.set_flag(id, 'apvalue', pval)
        #compute error average and discard 2*sigma points if requested
        if args.remove_noisy:
            data = p.get_data(filter="agood", colwise=True)
            if len(data)==0:
                print "Warning: all points in file %s have been discarded"\
                    " on cleanup" % os.path.basename(p.filename)
                continue
            med = median(data['err'])
            ids = []
            for datum in p.get_data(filter="agood"):
                id,q,I,err = datum[:4]
                if err > 20*med:
                    p.set_flag(id,'agood',False)
                    p.set_flag(id, 'apvalue', -1)
        data = p.get_data(filter="agood", colwise=True)
        if data=={}:
            print "Warning: all points in file %s have been discarded"\
                " on cleanup" % os.path.basename(p.filename)
            continue
        qvals = data['q']
        if verbose > 2:
            print "\tqmin = %.3f\tqmax = %.3f" % \
                    (qvals[0], qvals[-1])
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
    verbose = args.verbose
    maxpointsF = args.blimit_fitting
    maxpointsH = args.blimit_hessian
    mean_function = args.bmean
    model_comp = not args.bnocomp
    if verbose >0:
        print "2. fitting"
    for p in profiles:
        if verbose > 1:
            print "   ",os.path.basename(p.filename),
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
        mean, initvals, bayes = find_fit(data, verbose,
                model_comp=model_comp, model_comp_maxpoints=maxpointsH,
                mean_function=mean_function)
        model, particles, functions, gp = setup_process(data, 1)
        for part,v in initvals.items():
            particles[part].set_nuisance(v)
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
        prefvalues = pref.get_mean(qvalues=qvalues, colwise=True,
                        average=average)
        if True in map(isnan,prefvalues['I']):
            raise RuntimeError, "Got NAN in ref I"
        pcov = p.get_cov(pvalues['q'])
        if isnan(pcov).any():
            raise RuntimeError, "Got NAN in covariance matrix"
        if isinf(pcov).any():
            raise RuntimeError, "Got inf in covariance matrix"
        if True in map(isnan,prefvalues['err']):
            raise RuntimeError, "Got NAN in ref err"
        gammas.append(rescale_curves(prefvalues['I'],
                                     pvalues['I'],
                                     pcov, normal = use_normal,
                                     offset = use_offset))
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
        mat = g[2]
        p.set_gamma(gamma)
        p.set_offset(c)
        if verbose >1:
            print "   ",os.path.basename(p.filename),"   gamma = %.3G" % gamma,
            if use_offset:
                print "   offset = %.3G" % c,
            print " (%d)" % mat
    if True in map(isnan,gammas[-1]):
        raise RuntimeError, "Got NAN in rescaling step, try another rescaling " \
                            "or fitting model."
    return profiles,args

def classification(profiles, args):
    """fourth stage of merge: classify mean functions. If the auto flag
    is on, guesses the correct order of the input profiles.
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
    if args.auto:
        if verbose >0:
            print "   Reordering files"
        profiles = reorder_profiles(profiles)
    for i in xrange(len(profiles)):
        p = profiles[i]
        if verbose >1:
            print "   ",os.path.basename(p.filename)
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
    #remove unused profiles
    tbd=[]
    for i,p in enumerate(profiles):
        if p.get_data(filter='dgood',colwise=True).keys() == []:
            tbd.append(i)
    tbd.reverse()
    for i in tbd:
        name = os.path.basename(profiles[i].get_filename())
        profiles.pop(i)
        if verbose>1:
            print "   All points of profile %d (%s) were discarded" % \
                    (i,name)
    #remove redundant data if required
    if args.remove_redundant:
        if verbose >0:
            print "   Removing redundant data"
        profiles = remove_redundant(profiles,verbose=verbose)
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
            print "    ",os.path.basename(p.filename)
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
    mean, initvals, bayes = find_fit(data, verbose,
                    model_comp=model_comp,
                    model_comp_maxpoints=maxpointsH,
                    mean_function=mean_function)
    if verbose > 1 and model_comp:
        print "    => "+mean
    model, particles, functions, gp = setup_process(data, 1)
    for part,v in initvals.items():
        particles[part].set_nuisance(v)
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
        print "writing data to %s" % args.destdir
    if not os.path.isdir(args.destdir):
        os.mkdir(args.destdir)
    qvals = array(profiles[0].get_raw_data(colwise=True)['q'])
    #individual profiles
    if args.allfiles:
        if args.verbose > 1:
            print "   individual profiles"
        for i in profiles:
            if args.verbose > 2:
                print "    %s" % os.path.basename(i.get_filename())
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
