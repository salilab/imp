#replica exchange class
#inspired by ISD Replica.py
#Yannick

from numpy import *
from numpy.random import random,randint
import TuneRex
kB = 1.3806503 * 6.0221415 / 4184.0 # Boltzmann constant in kcal/mol/K

class ReplicaTracker():
    
    def __init__(self,nreps,inv_temps,grid,sfo_id,
            rexlog='replicanums.txt', scheme='standard', xchg='random',
            convectivelog='stirred.txt', tune_temps = False,
            tune_data={}, templog = 'temps.txt'):
        self.nreps = nreps
        #replica number as a function of state no
        self.replicanums = range(nreps)
        #state no as a function of replica no
        self.statenums = range(nreps)
        self.grid = grid
        self.sfo_id = sfo_id
        #expect inverse temperatures
        self.inv_temps = inv_temps
        self.logfile = rexlog
        self.stepno = 1
        self.scheme = scheme
        self.xchg = xchg
        self.tune_temps = tune_temps
        self.tune_data = tune_data
        self.rn_history = []
        self.templog = templog
        #scheme is one of gromacs, randomneighbors or convective
        if scheme == "convective":
            self.stirred = {}
            #which order the replicas should be chosen
            self.stirred['order'] = range(self.nreps)
            self.stirred['order'].reverse()
            #which replica is currently being stirred
            self.stirred['replica']=self.stirred['order'][0]
            #which position in the order list this replica sits in
            self.stirred['pos']=0
            #what direction should this replica should go to
            if self.stirred['replica'] != self.nreps-1:
                self.stirred['dir']=1 #up 
            else:
                self.stirred['dir']=0 #down
            #how many steps are remaining til we change stirring replica
            self.stirred['steps']=2*(self.nreps-1)
            self.convectivelog = convectivelog
        self.write_rex_stats()

    def sort_per_state(self, inplist):
        "sorts a replica list per state"
        if len(inplist) != self.nreps:
            raise ValueError, 'list has wrong size'
        return [inplist[i] for i in self.replicanums]
        
    def sort_per_replica(self, inplist):
        "sorts a state list per replica number"
        if len(inplist) != self.nreps:
            raise ValueError, 'list has wrong size'
        return [inplist[i] for i in self.statenums]
        
    def get_energies(self):
        return self.grid.gather(
                self.grid.broadcast(self.sfo_id,'m','evaluate',False))

    def gen_pairs_list_gromacs(self, dir):
        "generate ordered list of pairs based on dir"
        nreps = self.nreps
        if dir != 0 and dir != 1:
            raise ValueError, dir
        if nreps == 2:
            return [(0,1)]
        ret = [(2*i+dir,2*i+1+dir) for i in xrange(nreps/2)]
        if nreps in ret[-1]:
            ret.pop()
        return ret

    def gen_pairs_list_rand(self, needed = []):
        "generate list of neighboring pairs of states"
        nreps = self.nreps
        init = range(nreps)
        pairslist = []
        for (i,j) in needed:
            pairslist.append((min(i,j),max(i,j)))
            init.pop(i)
            init.pop(j)
        while len(init) > 1:
            i = randint(0,len(init)-1)
            dr = 2*randint(0,1)-1
            r=init.pop(i)
            if r+dr in init:
                init.remove(r+dr)
                pairslist.append((min(r,r+dr),max(r,r+dr)))
            elif r-dr in init and not (r==0 or r==nreps-1):
                init.remove(r-dr)
                pairslist.append((min(r,r-dr),max(r,r-dr)))
        return sorted(pairslist)

    def gen_pairs_list_conv(self):
        nreps = self.nreps
        rep = self.stirred['replica']
        state = self.statenums[rep]
        self.stirred['pair'] = (state, state + 2*self.stirred['dir'] - 1)
        self.stirred['pair'] = tuple(sorted(self.stirred['pair']))
        if self.xchg == 'gromacs':
            dir = (state + 1 + self.stirred['dir']) % 2
            return self.gen_pairs_list_gromacs(dir)
        elif self.xchg == 'random':
            return self.gen_pairs_list_rand(needed=[self.stirred['pair']])
        else:
            raise NotImplementedError, \
                    "Unknown exchange method: %s" % self.xchg

    def gen_pairs_list(self):
        if self.scheme == 'standard':
            if self.xchg == 'gromacs':
                return self.gen_pairs_list_gromacs(self.stepno % 2)
            elif self.xchg == 'random':
                return self.gen_pairs_list_rand()
            else:
                raise NotImplementedError, \
                        "unknown exchange method: %s" % self.xchg
        elif self.scheme == 'convective':
            return self.gen_pairs_list_conv()
        else:
            raise NotImplementedError, \
                    "unknown exchange scheme: %s" % self.scheme
        
    def get_cross_energies(self, pairslist):
        "get energies assuming all exchanges have succeeded"
        print "this is not needed for temperature replica-exchange"
        raise NotImplementedError
            
    def get_metropolis(self, pairslist, old_ene):
        """compute metropolis criterion for temperature replica exchange
        e.g. exp(Delta beta Delta E)
        """
        metrop={}
        (s1,s2) = pairslist[0]
        for (s1,s2) in pairslist:
            metrop[(s1,s2)] = \
                exp((old_ene[s2]-old_ene[s1])*
                        (self.inv_temps[s2]-self.inv_temps[s1]))
        return metrop
                    
    def try_exchanges(self, plist, metrop):
        accepted = []
        for couple in plist:
            if (metrop[couple] >= 1) or (random() < metrop[couple]):
                accepted.append(couple)
        return accepted

    def perform_exchanges(self, accepted):
        "exchange given state couples both in local variables and on the grid"
        #locally
        for (i,j) in accepted:
            #states
            ri = self.replicanums[i]
            rj = self.replicanums[j]
            self.statenums[ri] = j
            self.statenums[rj] = i
            #replicas
            buf = self.replicanums[i]
            self.replicanums[i] = self.replicanums[j]
            self.replicanums[j] = buf
        #on the grid (suboptimal but who cares)
        newtemps = self.sort_per_replica(self.inv_temps)
        self.grid.gather(self.grid.scatter(self.sfo_id, 'set_temp', newtemps))
        steps = self.grid.gather(
                self.grid.broadcast(self.sfo_id, 'get_mc_stepsize'))
        for (i,j) in accepted:
            ri = self.replicanums[i]
            rj = self.replicanums[j]
            buf = steps[ri]
            steps[ri] = steps[rj]
            steps[rj] = buf
        self.grid.gather(
                self.grid.scatter(self.sfo_id, 'set_mc_stepsize', steps))

    def write_rex_stats(self):
        "write replica numbers as a function of state"
        fl=open(self.logfile,'a')
        fl.write('%8d ' % self.stepno)
        fl.write(' '.join(['%2d' % (i+1) for i in self.replicanums]))
        fl.write('\n')
        fl.close()
        if self.scheme == 'convective':
            fl=open(self.convectivelog, 'a')
            fl.write('%5d ' % self.stepno)
            fl.write('%2d ' % (self.stirred['replica']+1))
            fl.write('%2d ' % self.stirred['dir'])
            fl.write('%2d\n' % self.stirred['steps'])
            fl.close()
        if self.tune_temps and len(self.rn_history) == 1:
            fl=open(self.templog, 'a')
            fl.write('%5d ' % self.stepno)
            fl.write(' '.join(['%.3f' % i for i in self.inv_temps]))
            fl.write('\n')
            fl.close()

    def tune_rex(self):
        """use TuneRex to optimize temp set. Temps are optimized every 
        'rate' steps and 'method' is used. Data is accumulated as long as
        the temperatures weren't optimized.
        td keys that should be passed to init:
            rate : the rate at which to try tuning temps
            method : flux or cv
            targetAR : for cv only, target acceptance rate
            alpha : Type I error to use.
        """
        #update replicanum
        self.rn_history.append([i for i in self.replicanums])
        td = self.tune_data
        if len(self.rn_history) % td['rate'] == 0\
                and len(self.rn_history) > 0:
            temps = [1/(kB*la) for la in self.inv_temps]
            if td['method'] == 'cv':
                indicators = TuneRex.compute_indicators(
                        transpose(self.rn_history))
                changed, newparams = TuneRex.tune_params(indicators, temps,
                        td['targetAR'], td['alpha'])
            elif td['method'] == 'flux':
                changed, newparams = TuneRex.tune_params_nonergodic(
                        transpose(self.rn_history),
                        temps, alpha = td['alpha'])
            if changed:
                self.rn_history = []
                print newparams
                self.inv_temps = [1/(kB*t) for t in newparams]
                
    def do_bookkeeping_before(self):
        self.stepno += 1
        if self.scheme == 'convective':
            st = self.stirred
            #check if we are done stirring this replica
            if st['steps'] == 0:
                st['pos'] = (st['pos'] + 1) % self.nreps
                st['replica'] = st['order'][st['pos']]
                st['dir'] = 1
                st['steps'] = 2*(self.nreps - 1)
            rep = st['replica']
            state = self.statenums[rep]
            #update endpoints
            if state == self.nreps - 1:
                st['dir'] = 0
            elif state == 0:
                st['dir'] = 1
            self.stirred = st

    def do_bookkeeping_after(self, accepted):
        if self.scheme == 'convective':
            rep = self.stirred['replica']
            state = self.statenums[rep]
            dir = 2*self.stirred['dir'] - 1
            expected = (min(state,state+dir),max(state,state+dir))
            if self.stirred['pair'] in accepted:
                self.stirred['steps'] -= 1

    def replica_exchange(self):
        "main entry point for replica-exchange"
        #print "replica exchange"
        self.do_bookkeeping_before()
        #tune temperatures
        if self.tune_temps:
            self.tune_rex()
        #print "energies"
        energies = self.sort_per_state(self.get_energies())
        fl=open('energies.txt','a')
        fl.write('%d ' % self.stepno)
        fl.write(' '.join(['%f' % ene for ene in energies]))
        fl.write('\n')
        fl.close()
        #print "pairs list"
        plist = self.gen_pairs_list()
        #print "metropolis"
        metrop = self.get_metropolis(plist,energies)
        #print "exchanges"
        accepted = self.try_exchanges(plist, metrop)
        #print "propagate"
        self.perform_exchanges(accepted)
        #print "book"
        self.do_bookkeeping_after(accepted)
        #print "done"




