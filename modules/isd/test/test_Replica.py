#!/usr/bin/env python

import os
import random

import IMP
import IMP.isd.Replica
import MockGrid
import IMP.test

class TestReplicaExchangeNine(IMP.test.TestCase):
    def setUp(self):
        nreps = self.nreps = 9
        temps = self.temps = [300.0 + (600.0-300.0)*i/float(nreps-1) for i in 
            xrange(nreps)]
        steps = self.steps = [1.0 for i in xrange(nreps)]
        grid=self.grid=MockGrid.MockGrid(nreps,temps,steps)
        replica=self.replica=IMP.isd.Replica.ReplicaTracker(grid.nreps,
            [1/(MockGrid.kB*t) for t in grid._temps], 
            grid, 123, tune_temps=False)
    
    def tearDown(self):
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

class TestReplicaExchangeTen(IMP.test.TestCase):
    def setUp(self):
        nreps = self.nreps = 10
        temps = self.temps = [300.0 + (600.0-300.0)*i/float(nreps-1) for i in 
            xrange(nreps)]
        steps = self.steps = [1.0 for i in xrange(nreps)]
        grid=self.grid=MockGrid.MockGrid(nreps,temps,steps)
        replica=self.replica=IMP.isd.Replica.ReplicaTracker(grid.nreps,
            [1/(MockGrid.kB*t) for t in grid._temps], 
            grid, 123, tune_temps=False)
    
    def tearDown(self):
        if os.path.exists('temps.txt'):
            os.remove('temps.txt')
        if os.path.exists('replicanums.txt'):
            os.remove('replicanums.txt')

    def test_sort_per_state(self):
        #replica numbers as a function of state
        self.replica.replicanums = replist = [1, 3, 5, 7, 0, 2, 4, 6, 8, 9]
        #state numbers as a function of replica
        self.replica.statenums = state = [4, 0, 5, 1, 6, 2, 7, 3, 8, 9]
        self.assertEqual(self.replica.sort_per_state(state), 
                range(self.replica.nreps))

    def test_sort_per_replica(self):
        #replica numbers as a function of state
        self.replica.replicanums = replist = [1, 3, 5, 7, 0, 2, 4, 6, 8, 9]
        #state numbers as a function of replica
        self.replica.statenums = state = [4, 0, 5, 1, 6, 2, 7, 3, 8, 9]
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




if __name__ == '__main__':
    IMP.test.main()

        #tune_data={'rate':100000,'method':'cv','targetAR':0.4,'alpha':0.1,
         #   'CvMethod':'constant'})
        #for i in xrange(50000):
        #    replica.replica_exchange()
        #    replica.write_rex_stats()
