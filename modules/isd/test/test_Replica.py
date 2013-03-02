#!/usr/bin/env python

import os
import numpy.random as random
from numpy import exp

import IMP
import IMP.isd.Replica
import MockGrid
import IMP.test

# Use faster built-in 'set' type on newer Pythons; fall back to the older
# 'sets' module on older Pythons
try:
    x = set
    del x
except NameError:
    import sets
    set = sets.Set

class Tests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        random.seed()
        nreps = self.nreps = 9
        temps = self.temps = [300.0 + (600.0-300.0)*i/float(nreps-1) for i in
                xrange(nreps)]
        steps = self.steps = [1.0 for i in xrange(nreps)]
        grid=self.grid=MockGrid.MockGrid(nreps,temps,steps)
        replica=self.replica=IMP.isd.Replica.ReplicaTracker(grid.nreps,
            [1/(MockGrid.kB*t) for t in grid._temps],
            grid, 123, tune_temps=False)

    def tearDown(self):
        IMP.test.TestCase.tearDown(self)
        if os.path.exists('temps.txt'):
            os.remove('temps.txt')
        if os.path.exists('replicanums.txt'):
            os.remove('replicanums.txt')

    def test_sort_per_state(self):
        """Test ReplicaTracker sort_per_state"""
        #replica numbers as a function of state
        self.replica.replicanums = replist = [1, 3, 5, 7, 0, 2, 4, 6, 8]
        #state numbers as a function of replica
        self.replica.statenums = state = [4, 0, 5, 1, 6, 2, 7, 3, 8]
        self.assertEqual(self.replica.sort_per_state(state),
                range(self.replica.nreps))

    def test_sort_per_replica(self):
        """Test ReplicaTracker sort_per_replica"""
        #replica numbers as a function of state
        self.replica.replicanums = replist = [1, 3, 5, 7, 0, 2, 4, 6, 8]
        #state numbers as a function of replica
        self.replica.statenums = state = [4, 0, 5, 1, 6, 2, 7, 3, 8]
        self.assertEqual(self.replica.sort_per_replica(replist),
                range(self.replica.nreps))

    def test_sort_sanity(self):
        """Test ReplicaTracker sort sanity"""
        testlist = range(self.replica.nreps)
        for i in xrange(100):
            random.shuffle(testlist)
            self.assertEqual(testlist,
                    self.replica.sort_per_state(
                        self.replica.sort_per_replica(testlist)))
            self.assertEqual(testlist,
                    self.replica.sort_per_replica(
                        self.replica.sort_per_state(testlist)))

    def test_sort_errors(self):
        """Test ReplicaTracker sort methods with invalid input"""
        self.assertRaises(ValueError,
                self.replica.sort_per_state, range(self.nreps-1))
        self.assertRaises(ValueError,
                self.replica.sort_per_replica, range(self.nreps-1))

    def test_energies(self):
        """Test ReplicaTracker get_energies()"""
        a=range(self.nreps)
        for i in xrange(100):
            random.shuffle(a)
            self.grid.set_replay(a)
            self.assertEqual(self.replica.get_energies(),a)

    def test_gen_pairs_list_gromacs(self):
        """Test ReplicaTracker gen_pairs_list_gromacs()"""
        #nreps=9 dir=0
        self.replica.nreps=9
        expected=[(0,1),(2,3),(4,5),(6,7)]
        self.assertEqual(self.replica.gen_pairs_list_gromacs(0),expected)
        #nreps=9 dir=1
        expected=[(1,2),(3,4),(5,6),(7,8)]
        self.assertEqual(self.replica.gen_pairs_list_gromacs(1),expected)
        #nreps=10 dir=0
        self.replica.nreps=10
        expected=[(0,1),(2,3),(4,5),(6,7),(8,9)]
        self.assertEqual(self.replica.gen_pairs_list_gromacs(0),expected)
        #nreps=10 dir=1
        expected=[(1,2),(3,4),(5,6),(7,8)]
        self.assertEqual(self.replica.gen_pairs_list_gromacs(1),expected)

    def test_gen_pairs_list_gromacs_neighbors(self):
        """Test ReplicaTracker gen_pairs_list_gromacs() neighbors"""
        self.replica.nreps=9
        for i in xrange(100):
            for pair in self.replica.gen_pairs_list_gromacs(0):
                self.assertEqual(pair[1]-pair[0],1)
        self.replica.nreps=10
        for i in xrange(100):
            for pair in self.replica.gen_pairs_list_gromacs(0):
                self.assertEqual(pair[1]-pair[0],1)
        self.replica.nreps=9
        for i in xrange(100):
            for pair in self.replica.gen_pairs_list_gromacs(1):
                self.assertEqual(pair[1]-pair[0],1)
        self.replica.nreps=10
        for i in xrange(100):
            for pair in self.replica.gen_pairs_list_gromacs(1):
                self.assertEqual(pair[1]-pair[0],1)

    def test_gen_pairs_list_rand_neighbors(self):
        """Test ReplicaTracker gen_pairs_list_rand() neighbors"""
        self.replica.nreps=9
        for i in xrange(100):
            for pair in self.replica.gen_pairs_list_rand():
                self.assertEqual(pair[1]-pair[0],1)
        self.replica.nreps=10
        for i in xrange(100):
            for pair in self.replica.gen_pairs_list_rand():
                self.assertEqual(pair[1]-pair[0],1)

    def test_gen_pairs_list_rand_singletons(self):
        """Test ReplicaTracker gen_pairs_list_rand() singletons"""
        self.replica.nreps=9
        for i in xrange(100):
            plist=set()
            for p in self.replica.gen_pairs_list_rand():
                #print p
                plist.add(p[0])
                plist.add(p[1])
            a=set(range(self.nreps))
            singletons = a - plist
            #print singletons
            for i in singletons:
                if i != 0 and i+1 != self.nreps-1:
                    self.assertEqual((i+1) in singletons, False)
                if i-1 != 0 and  i != self.nreps-1:
                    self.assertEqual((i-1) in singletons, False)
        self.replica.nreps=10
        for i in xrange(100):
            plist=set()
            for p in self.replica.gen_pairs_list_rand():
                plist.add(p[0])
                plist.add(p[1])
            a=set(range(self.nreps))
            singletons = a - plist
            for i in singletons:
                if i != 0 and i+1 != self.nreps-1:
                    self.assertEqual((i+1) in singletons, False)
                if i-1 != 0 and  i != self.nreps-1:
                    self.assertEqual((i-1) in singletons, False)

    def test_gen_pairs_list_rand_uniform_distr(self):
        "Test uniformity of randomly generated replica pairs"
        pairs={}
        for i in xrange(100000):
            for pair in self.replica.gen_pairs_list_rand():
                if pair in pairs:
                    pairs[pair] += 1
                else:
                    pairs[pair] = 1
        s=float(sum(pairs.values()))/len(pairs)

        for i in pairs.values():
            self.assertAlmostEqual(i/s, 1.0, delta=0.015)
        #print sorted(pairs.items())

    def test_get_metropolis(self):
        """Test ReplicaTracker get_metropolis()"""
        pl=[(1,2)]
        ene=[1,2,3]
        self.replica.inv_temps=[6,5,4]
        expected=exp(-1)  #exp((5-6)*(3-2))
        self.assertAlmostEqual(self.replica.get_metropolis(pl,ene)[pl[0]],
                expected, delta=1e-3)
        ene=[3,2,1]
        expected=1
        self.assertAlmostEqual(self.replica.get_metropolis(pl,ene)[pl[0]],
                expected, delta=1e-3)

    def test_perform_exchanges_statenums(self):
        """Test ReplicaTracker perform_exchanges() statenums"""
        accepted=[(0,1),(5,6),(7,8)]
        sb = [i for i in self.replica.statenums]
        self.replica.perform_exchanges(accepted)
        sa = [i for i in self.replica.statenums]
        for (i,j) in accepted:
            ri = self.replica.replicanums[j]
            rj = self.replica.replicanums[i]
            self.assertEqual(sa[rj],sb[ri])
            self.assertEqual(sa[ri],sb[rj])

    def test_perform_exchanges_replicanums(self):
        """Test ReplicaTracker perform_exchanges() replicanums"""
        accepted=[(0,1),(5,6),(7,8)]
        rnb = [i for i in self.replica.replicanums]
        self.replica.perform_exchanges(accepted)
        rna = [i for i in self.replica.replicanums]
        for (i,j) in accepted:
            self.assertEqual(rna[j],rnb[i])
            self.assertEqual(rna[i],rnb[j])

    def test_perform_exchanges_temps(self):
        """Test ReplicaTracker perform_exchanges() temps"""
        accepted=[(0,1),(5,6),(7,8)]
        tb = self.replica.sort_per_state(self.grid.gather(
                self.grid.broadcast(123, 'get_temp')))
        self.replica.perform_exchanges(accepted)
        ta = self.replica.sort_per_state(self.grid.gather(
                self.grid.broadcast(123, 'get_temp')))
        for (i,j) in accepted:
            self.assertEqual(ta[j],tb[i])
            self.assertEqual(ta[i],tb[j])

    def test_perform_exchanges_steps(self):
        """Test ReplicaTracker perform_exchanges() steps"""
        accepted=[(0,1),(5,6),(7,8)]
        stb = self.replica.sort_per_state(self.grid.gather(
                self.grid.broadcast(123, 'get_mc_stepsize')))
        self.replica.perform_exchanges(accepted)
        sta = self.replica.sort_per_state(self.grid.gather(
                self.grid.broadcast(123, 'get_mc_stepsize')))
        for (i,j) in accepted:
            self.assertEqual(sta[j],stb[i])
            self.assertEqual(sta[i],stb[j])



if __name__ == '__main__':
    IMP.test.main()

        #tune_data={'rate':100000,'method':'cv','targetAR':0.4,'alpha':0.1,
         #   'CvMethod':'constant'})
        #for i in xrange(50000):
        #    replica.replica_exchange()
        #    replica.write_rex_stats()
