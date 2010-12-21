## CHANGED (remove sleep)

##
## The Inferential Structure Determination (ISD) software library
##
## Authors: Michael Habeck and Wolfgang Rieping
##        
##          Copyright (C) Michael Habeck and Wolfgang Rieping
## 
##          All rights reserved.
##
## NO WARRANTY. This library is provided 'as is' without warranty of any
## kind, expressed or implied, including, but not limited to the implied
## warranties of merchantability and fitness for a particular purpose or
## a warranty of non-infringement.
##
## Distribution of substantively modified versions of this module is
## prohibited without the explicit permission of the copyright holders.
##
 

from Isd.misc.TypeChecking import *
from Isd.sampling.rex.Replica import Heatbath, ReplicaSampler
#from Isd.FileBasedGrid import FileBasedGrid, SSHBasedGrid
#from Isd.AbstractGrid import GridService

def check_states(states):

    from Isd.misc.legacy import check_state

    for s in states:
        if not check_state(s):
            raise StandardError, 'Initial states (or ensemble) have been created with an older version of ISD. Please migrate project to the current version of ISD. See isd --help or the manual for instructions of how to do this.'

def map_states(states, heatbaths):

    from Isd.misc.mathutils import standardDeviation
    from numpy.oldnumeric import array, argmin, sum

    l = []

    X = [(x.replica_parameters['lambda'], x.replica_parameters['q']) \
         for x in states]

    sd = standardDeviation(X)
    X = array(X) / sd

    mapped = False

    for i in range(len(heatbaths)):
        
        p = array([heatbaths[i].replica_parameters['lambda'],
                   heatbaths[i].replica_parameters['q']]) / sd

        closest = argmin(sum(abs(X-p), 1))

        l.append(states[closest])

        mapped = mapped or closest <> i

    return mapped, l

# class AbstractISDGrid:
    
#     SERVICE_ID = 'ISD remote sampler'
    
#     def __init__(self, posterior):
#         self.posterior = posterior

#     def create_sampler_server(self):
#         return ISDSamplerServer(self.posterior)

#     def create_server(self, host, display=True):

#         from Isd.Grid import Server

#         check_type(host, STRING, 'Host')

#         if type(host) == type(''):

#             from Isd.hosts import Host

#             host = Host(host)

#         servers = []

#         for i in range(host.n_cpu):

#             object_instance = self.create_sampler_server()

#             proxy = self.create_proxy(object_instance, host,
#                                       display = display,
#                                       daemon = 1)

#             server = Server(proxy, self.SERVICE_ID, host)

#             servers.append(server)

#         return servers

#     def create_heatbath(self):
#         raise NotImplementedError

        
# class ISDGrid(AbstractISDGrid, Grid):
    
#     def __init__(self, posterior, temp_path, debug=False):
#         AbstractISDGrid.__init__(self, posterior)
#         Grid.__init__(self, temp_path, debug=debug)

#     def create_heatbath(self):
#         return ISDHeatBath(self)

# class FileBasedISDGrid(AbstractISDGrid, FileBasedGrid):
    
#     def __init__(self, posterior, temp_path, debug=False, nfs_care=False):
#         AbstractISDGrid.__init__(self, posterior)
#         FileBasedGrid.__init__(self, temp_path, debug=debug, \
#                                nfs_care=nfs_care)

#     def create_heatbath(self):
#         return FileBasedISDHeatbath(self)
    
# class SSHBasedISDGrid(AbstractISDGrid, SSHBasedGrid):
    
#     def __init__(self, posterior, temp_path, debug=False):
#         AbstractISDGrid.__init__(self, posterior)
#         SSHBasedGrid.__init__(self, temp_path, debug=debug)
        
#     def create_heatbath(self):
#         return SSHBasedISDHeatbath(self)
    

class ISDSamplerServer:
    
    def __init__(self, posterior):

        check_type(posterior, 'Posterior')

        self.init_sampler(posterior)
        self.set_replica_parameters({})

        self.elapsed = 0.

    def init_sampler(self, posterior):

        from Isd.sampling.gibbs.ISDSampler import ISDSampler

        self.sampler = ISDSampler(posterior)
        
    def __generate_candidate(self, start):

        from time import time, sleep 

        ##sleep(10)
        
        t0 = time()

        sampler = self.sampler

        sampler.set_initial_sample(start)

##         if start.stepsize > 0.:
##             sampler.torsion_sampler.set_stepsize(start.stepsize)

        n = self.replica_parameters['steps']

        x = sampler.generate_sequence(n)

        elapsed = time() - t0

        if elapsed > 1000.:
            elapsed = 0.

        self.elapsed = elapsed

        candidate = x[-1]

        candidate.label = start.label

        return candidate

    def generate_candidate(self, start):

        return self.__generate_candidate(start)

##         try:
##             return self.__generate_candidate(start)

##         except Exception, msg:

##             import os, socket

##             root = os.environ['ISD_ROOT']

##             s = 'Host: %s\nReplica parameters: %s\nError message:%s\n'

##             f = open(root + '/isd.log', 'w')

##             f.write(s % (socket.gethostname(),
##                          str(self.replica_parameters), msg))

##             f.close()

##             raise Exception, msg
        
    def set_replica_parameters(self, p):
        "input: p (a schedule). Sets the replica parameters to p."
        
        check_dict(p)

        self.replica_parameters = p

        if p:

            if 'verbose_level' in p:
                self.sampler.set_verbose_level(p['verbose_level'])

            torsion_sampler = self.sampler.get_torsion_sampler()
            torsion_sampler.set_md_steps(p['md_steps'])

            posterior = self.sampler.get_posterior()

            if 'tsallis_prior' in posterior.get_conformational_priors():
                posterior.set_q(p['q'])
            if 'simplenl_prior' in posterior.get_conformational_priors():
                posterior.set_q(p['q'])
                posterior.set_alphamax(p['alphamax'])
                posterior.set_Emin(p['Emin'])

            posterior.set_lambda(p['lambda'])

            if 'adjust_stepsize' in p:
                torsion_sampler.adjust_stepsize(int(p['adjust_stepsize']))

            if 'stepsize' in p:
                torsion_sampler.set_stepsize(float(p['stepsize']))

            if 'likelihoods' in p:

                for name, d in p['likelihoods'].items():

                    likelihood = self.sampler.posterior.likelihoods[name]

                    [setattr(likelihood, param, value) for \
                     param, value in d.items()]

            if 'error_models' in p:

                for name, d in p['error_models'].items():

                    error_model = posterior.likelihoods[name].error_model

                    [setattr(error_model, param, value) for \
                     param, value in d.items()]

            if 'posterior' in p:

                [setattr(posterior, param, value) for \
                 param, value in p['posterior'].items()]

    def get_replica_parameters(self):
        return self.replica_parameters

    def calculate_energy(self, state):
        return (self.sampler.calculate_energy(state), state)

    def calculate_energies(self, data):
        """If data == { key1:(schedule1, state), ...}
        returns (E, states)
        where E == { key1: energy of state in schedule1, ...}
        and states == { key1:state, ... }
        """

        E = {}
        states = {}

        for key, (params, state) in data.items():

            self.set_replica_parameters(params)

            E[key] = self.sampler.calculate_energy(state)

            states[key] = state

        return E, states

    def get_time(self):
        return self.elapsed

    def __getstate__(self):

        return {'posterior': self.sampler.get_posterior(),
                'replica_parameters': self.get_replica_parameters(),
                'elapsed': self.elapsed,
                'update_torsion_angles': self.sampler.update_torsion_angles}

    def __setstate__(self, x):

        ## we initialize a new ISDSampler object, since
        ## the current implementation of HMCSampler cannot be pickled

        self.init_sampler(x['posterior'])
        self.set_replica_parameters(x['replica_parameters'])
        self.elapsed = x['elapsed']
        self.sampler.update_torsion_angles = x['update_torsion_angles']
        

class ISDHeatbath(Heatbath):

    """ 
    This is an ISD-specific heatbath class.
    """
    #SERVICE_ID = 'ISD remote sampler'

    def __init__(self, grid, service_id, debug):

        Heatbath.__init__(self)

        #if grid is not None:
        #    check_type(grid, 'Grid')

        self.grid = grid
        self.SERVICE_ID = service_id

        self.debug = debug

    #def set_grid(self, x):
    #    check_type(x, 'Grid')
    #    self.grid = x
        
    #def create_callback(self, service):
    #    return lambda result, s = self, sv = service: \
    #                              s.grid.release_service(sv)

    def generate_candidate(self, start):

        #print '\n\nDEBIG ISDReplica.generate_candidate: %s' %start
        
        proxy = self.grid.acquire_service(self.SERVICE_ID)

        result = proxy.set_replica_parameters(self.replica_parameters)
        result = proxy.generate_candidate(start)

        result.proxy = proxy

        self.grid.release_service(proxy)
        
        return result

    def calculate_energy(self, state):
        """
        assumes that the current task represents the heatbath
        based on which the energy is calculated.
        """
        #print '\n\nDEBIG ISDReplica.calculate_energy: %s' %state

        proxy = self.grid.acquire_service(self.SERVICE_ID)

        result = proxy.set_replica_parameters(self.replica_parameters)
        
        result = proxy.calculate_energy(state)
        result.proxy = proxy

        self.grid.release_service(proxy)

        return result

    def calculate_energies(self, data):
        """
        data: dict. key mapps to length-2-tuple: (replica_parameters, state)        
        """

        #print '\n\nDEBIG ISDReplica.calculate_energies: %s' %data

        proxy = self.grid.acquire_service(self.SERVICE_ID)

        result = proxy.calculate_energies(data)
        result.proxy = proxy

        self.grid.release_service(proxy)

        return result

    def set_replica_parameters(self, p):
        self.replica_parameters = p

    def isthreaded(self):
        return 1

# class PyroISDHeatbath(ISDHeatbath):

#     def create_callback(self, service):
#         pass

#     def generate_candidate(self, start):

#         if self.debug:
#             print 'PyroISDReplica.generate_candidate: %s' %start
        
#         proxy = self.grid.acquire_service(self.SERVICE_ID)

#         proxy.set_replica_parameters(self.replica_parameters)
#         result = proxy.generate_candidate(start)

#         #result.proxy = proxy

#         self.grid.release_service(proxy)
        
#         return result

#     def calculate_energy(self, state):
#         """
#         assumes that the current task represents the heatbath
#         based on which the energy is calculated        
#         """

#         if self.debug:
#             print 'PyroISDReplica.calculate_energy: %s' %state

#         proxy = self.grid.acquire_service(self.SERVICE_ID)

#         proxy.set_replica_parameters(self.replica_parameters)        
#         result = proxy.calculate_energy(state)
        
#         #result.proxy = proxy

#         self.grid.release_service(proxy)

#         return result

#     def calculate_energies(self, data):
#         """
#         data: dict. key mapps to length-2-tuple: (replica_parameters, state)
#         """

#         if self.debug:
#             print 'PyroISDReplica.calculate_energies: %s' %data

#         proxy = self.grid.acquire_service(self.SERVICE_ID)

#         result = proxy.calculate_energies(data)
#         #result.proxy = proxy

#         self.grid.release_service(proxy)
        
#         return result
    
#     def isthreaded(self):
#         return 1
    
#     def get_id(self):
#         return self.id
    
# class FileBasedISDHeatbath(ISDHeatbath):

#     def generate_candidate(self, start):

#         if self.debug:
#             print 'FileBasedISDReplica.generate_candidate: %s' %start
            
#         proxy = self.grid.acquire_service(self.SERVICE_ID)

#         proxy.set_replica_parameters(self.replica_parameters)
#         #proxy._call_method('set_replica_parameters', self.replica_parameters)

#         #result = proxy._call_method('generate_candidate', start)
#         result = proxy.generate_candidate(start)
        
#         self.grid.release_service(proxy)

#         #result.proxy = proxy
        
#         return result

#     def calculate_energies(self, data):
#         """
#         data: dict. key mapps to length-2-tuple: (replica_parameters, state)
#         """

#         if self.debug:
#             print 'FileBasedISDReplica.calculate_energies: %s' %data
        
#         proxy = self.grid.acquire_service(self.SERVICE_ID)

#         result = proxy._call_method('calculate_energies', data)

#         self.grid.release_service(proxy)

#         #result.proxy = proxy

#         return result

#     def set_replica_parameters(self, p):

#         if self.debug:
#             print 'FileBasedISDReplica.set_replica_parameters: %s' %p
        
#         self.replica_parameters = p

#     def isthreaded(self):
#         return 1

# class SSHBasedISDHeatbath(FileBasedISDHeatbath):
#     pass

class ISDReplicaSampler(ReplicaSampler):
    """
    main sampling class from which generate_sequence (implemented in 
    ReplicaSampler) will be called
    """

    def __init__(self, heatbaths, dump_dir=None, dump_prefix='E',
                 dump_n=50, full_save=True, threaded=True, debug=False,
                 optimize=False):

        #TODO: propagate xchg_method option to runisd.py
        ReplicaSampler.__init__(self, heatbaths, dump_path=dump_dir,
                dump_prefix=dump_prefix, dump_n=dump_n,
                                threaded=threaded, debug=debug,
                                optimize=optimize)

        from Isd.misc import utils

        self.full_save = full_save

        self.init_ensembles()

        self.times = []
        self.adjust_until = 1e100

        self.history['states'] = []
        self.history['energies'] = []

        ## stepsize adjustment

        self.stepsizes = [utils.Pipe(50) for x in heatbaths]

    def init_ensembles(self):
        from Isd.analysis.Ensemble import Ensemble

        self.samples = [Ensemble() for i in range(len(self.heatbaths))]

    def next_sample(self, x0, direction):

        from time import time

        t0 = time()

        result = ReplicaSampler.next_sample(self, x0, direction)

        self.times.append(time() - t0)

        return result

    def generate_candidate(self, initial_states):

        candidates = ReplicaSampler.generate_candidate(self, initial_states)

        ## automatic stepsize adjustment

        if self.heatbaths[0].replica_parameters['adjust_stepsize'] and \
           len(self.history['energies']) < self.adjust_until:

            ## put individual stepsizes into queue

            for i in range(len(candidates)):

                s = candidates[i].stepsize
                
                self.stepsizes[i].put(s)
                self.heatbaths[i].replica_parameters['stepsize'] = s

        return candidates

    def add_sample(self, x):

        ## add information about replica parameters

        for i in range(len(x)):
            x[i].replica_parameters = self.heatbaths[i].replica_parameters
            self.samples[i].append(x[i])

        self.history['states'] = list(x)

        self.manage_stepsize_adjustment(x)
        
    def manage_stepsize_adjustment(self, x):

        if self.heatbaths[0].replica_parameters['adjust_stepsize'] and \
               len(self.history['energies']) >= self.adjust_until:

            if len(self.stepsizes[0]):

                from numpy import median

                for i in range(len(x)):
                    
                    P = self.heatbaths[i].replica_parameters
                    P['stepsize'] = median(self.stepsizes[i], 0)

                    x[i].stepsize = P['stepsize']

            for i in range(len(x)):
                self.heatbaths[i].replica_parameters['adjust_stepsize'] = False

            if self.debug:
                print 'Using median stepsizes:'
                
                for i in range(len(x)):
                    print i, self.heatbaths[i].replica_parameters['stepsize']

    def add_energy(self, x):
        self.history['energies'].append([a.E for a in x])

    def __getitem__(self, index):
        return self.samples[index]

    def __setitem__(self, index, value):
        self.samples[index] = value

    def number_of_samples(self):
        return len(self[0])

    def dump(self, path, prefix):

        from Isd.misc.utils import save_dump
        from time import ctime

        ReplicaSampler.dump(self, path, prefix)

        if self.full_save:
            indices = range(len(self))
        else:
            indices = [0, len(self)-1]

        for i in indices:

            ensemble = self.samples[i]
            ensemble.replica_parameters = self.heatbaths[i].replica_parameters

            filename = path + '/%s_%d' % (prefix, i)

            save_dump(ensemble, filename, err_msg='Could not save ensemble %d. Trying again...' % i, mode='a')

        print '%d ensembles with %d samples saved (%s).' % (len(indices), self.number_of_samples(), ctime())

        self.init_ensembles()

    def load(self):
        """loads replica exchange history and try to reuse old params as much as
        possible by returning a correct "intial_states" for this simulation.
        """

        ReplicaSampler.load(self)

        states = self.history.get('states')

        if states:

            check_states(states)

            if hasattr(states[0], 'replica_parameters'):
                
                mapped, initial_states = map_states(states, self.heatbaths)

                if mapped:

                    self.history['energies'] = []

                    if 'stepsizes' in self.history:
                        self.history['stepsizes'] = []
                    
                    print 'Replica parameters have changed: Using initial states that fit best, energies have been reset.'
                    
                else:
                    print 'Using exisiting states as initial states.'

            else:

                if len(states) == len(self.heatbaths):
            
                    initial_states = list(states)

                    print 'Using exisiting states as initial states.'

                    ## legacy: add missing attributes

                    for i in range(len(initial_states)):
                        initial_states[i].replica_parameters = self.heatbaths[i].replica_parameters

                else:
                    print 'Number of replicas has changed. Using new initial states.'

                    return None

            return initial_states

        else:

            ## legacy code

            from Isd.misc.utils import Load
            import os

            filename_template = self.dump_path + '/%s_' % self.dump_prefix + '%d'

            if os.path.exists(filename_template % 0):

                initial_states = []

                print 'Loading initial states from Isd.Ensembles located in %s ...' % self.dump_path

                for i in range(len(self.heatbaths)):

                    filename = self.dump_path + '/%s_%d' % (self.dump_prefix, i)

                    if os.path.exists(filename):

                        print filename

                        replica = Load(filename)
                        initial_states.append(replica[-1])

                    else:
                        break

                if len(initial_states) == len(self.heatbaths):

                    check_states(initial_states)
                    
                    return initial_states

                else:

                    print 'Ensembles inaccessible.'
                    
                    return None

            else:
                return None

