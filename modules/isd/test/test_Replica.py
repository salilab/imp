#!/usr/bin/env python

import os
import numpy.random as random

import IMP
import IMP.isd.Replica
import MockGrid
import IMP.test

class TestReplicaExchange(IMP.test.TestCase):
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
        #replica numbers as a function of state
        self.replica.replicanums = replist = [1, 3, 5, 7, 0, 2, 4, 6, 8]
        #state numbers as a function of replica
        self.replica.statenums = state = [4, 0, 5, 1, 6, 2, 7, 3, 8]
        self.assertEqual(self.replica.sort_per_state(state), 
                range(self.replica.nreps))

    def test_sort_per_replica(self):
        #replica numbers as a function of state
        self.replica.replicanums = replist = [1, 3, 5, 7, 0, 2, 4, 6, 8]
        #state numbers as a function of replica
        self.replica.statenums = state = [4, 0, 5, 1, 6, 2, 7, 3, 8]
        self.assertEqual(self.replica.sort_per_replica(replist),
                range(self.replica.nreps))

    def test_sort_sanity(self):
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
        self.assertRaises(ValueError, 
                self.replica.sort_per_state, range(self.nreps-1))
        self.assertRaises(ValueError, 
                self.replica.sort_per_replica, range(self.nreps-1))

    def test_energies(self):
        a=range(self.nreps)
        for i in xrange(100):
            random.shuffle(a)
            self.grid.set_replay(a)
            self.assertEqual(self.replica.get_energies(),a)

    def test_gen_pairs_list_gromacs(self):
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
        self.replica.nreps=9
        for i in xrange(100):
            for pair in self.replica.gen_pairs_list_rand():
                self.assertEqual(pair[1]-pair[0],1)
        self.replica.nreps=10
        for i in xrange(100):
            for pair in self.replica.gen_pairs_list_rand():
                self.assertEqual(pair[1]-pair[0],1)

    def test_gen_pairs_list_rand_singletons(self):
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
                self.assertEqual((i+1) in singletons, False)
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
                self.assertEqual((i+1) in singletons, False)
                self.assertEqual((i-1) in singletons, False)

    def test_2(self):
        pairs={}
        for i in xrange(100000):
            for pair in self.replica.gen_pairs_list_rand():
                if pair in pairs:
                    pairs[pair] += 1
                else:
                    pairs[pair] = 1
        print sorted(pairs.items())








if __name__ == '__main__':
    IMP.test.main()

        #tune_data={'rate':100000,'method':'cv','targetAR':0.4,'alpha':0.1,
         #   'CvMethod':'constant'})
        #for i in xrange(50000):
        #    replica.replica_exchange()
        #    replica.write_rex_stats()
