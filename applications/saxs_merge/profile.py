#!/usr/bin/python

import os,sys
import copy
from numpy import linspace,sqrt

import IMP.isd

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
            bool_to_int=False, dir='./', *args, **kwargs):
        fl=open(os.path.join(dir,prefix+filename+suffix),'w')
        fl.write('#')
        for i,name in enumerate(self.get_flag_names()):
            fl.write("%d:%s%s" % (i+1,name,sep))
        fl.write('\n')
        for d in self.get_data(*args, **kwargs):
            for ent in d[1:]:
                if bool_to_int and isinstance(ent, bool):
                    fl.write('%d' % ent)
                else:
                    fl.write('%s' % ent)
                fl.write(sep)
            fl.write(sep.join(['%s' % i for i in d[1:]]))
            fl.write('\n')
        fl.close()

    def write_mean(self, filename, prefix='mean_', suffix='', sep=' ',
            bool_to_int=False, dir='./', *args, **kwargs):
        fl=open(os.path.join(dir,prefix+filename+suffix),'w')
        fl.write('#')
        for i,name in enumerate(self.get_flag_names()):
            fl.write("%d:%s%s" % (i+1,name,sep))
        fl.write('\n')
        for d in self.get_mean(*args, **kwargs):
            for ent in d:
                if bool_to_int and isinstance(ent, bool):
                    fl.write('%d' % ent)
                else:
                    fl.write('%s' % ent)
                fl.write(sep)
            fl.write('\n')
        fl.close()

    def set_filename(self, fname):
        self.filename = fname

    def get_filename(self):
        return self.filename
