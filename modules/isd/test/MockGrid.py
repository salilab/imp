#!/usr/bin/env python

import numpy.random as random
kB = 1.3806503 * 6.0221415 / 4184.0  # Boltzmann constant in kcal/mol/K
example_nreps = 10
example_temps = [300.0 + (600.0 - 300.0) * i / float(example_nreps - 1) for i in
                 xrange(example_nreps)]
example_steps = [1.0 for i in xrange(example_nreps)]


class MockModel:

    """fake model for a slave"""

    def __init__(self, temp):
        self.temp = temp

    def evaluate(self, flag):
        """return random deviate from gaussian energy with mean kT and standard
        deviation kT
        """
        if flag:
            raise NotImplementedError
        return random.normal(30 * kB * self.temp, kB * self.temp)

    def set_temp(self, temp):
        self.temp = temp


class MockSlave:

    """fake slave for the grid"""

    def __init__(self, temp, mc_stepsize):
        self.temp = temp
        self.mc_stepsize = mc_stepsize
        self._m = MockModel(temp)

    def m(self, funcname, *args, **kwargs):
        try:
            func = getattr(self._m, funcname)
        except AttributeError:
            raise NotImplementedError
        return func(*args, **kwargs)

    def get_temp(self):
        return self.temp

    def set_temp(self, temp):
        self.temp = temp
        self._m.set_temp(temp)

    def get_mc_stepsize(self):
        return self.mc_stepsize

    def set_mc_stepsize(self, mc):
        self.mc_stepsize = mc

    def get_state(self):
        return {'inv_temp': 1 / (kB * self.get_temp()),
                'mcstep': self.get_mc_stepsize()}

    def set_state(self, state):
        self.set_temp(1 / (kB * state['inv_temp']))
        self.set_mc_stepsize(state['mcstep'])


class MockResult:

    """fake result object for the grid"""

    def __init__(self, value):
        self.value = value

    def get(self):
        return self.value


class MockGrid:

    """fake grid to test Replica.py"""

    def __init__(self, nreps=example_nreps,
                 temps=example_temps, stepsizes=example_steps):
        if len(temps) != nreps or len(stepsizes) != nreps:
            raise ValueError("adjust temps and stepsizes to nreps!")
        self.nreps = nreps
        self._temps = temps
        self._slaves = [MockSlave(temp, mc)
                        for temp, mc in zip(temps, stepsizes)]
        self._replay = None

    def set_replay(self, replay):
        "override output of gather function"
        if len(replay) == self.nreps:
            self._replay = replay
        else:
            raise ValueError("replay should be length of nrep")

    def broadcast(self, id, call, *args, **kwargs):
        return [MockResult(getattr(sl, call)(*args, **kwargs))
                for sl in self._slaves]

    def scatter(self, id, funcname, *arglist, **kwlist):
        results = []
        if kwlist is None:
            kwlist = [{} for i in xrange(len(arglist))]
        if not hasattr(arglist[0], '__iter__'):
            arglist = [[i] for i in arglist]
        for prox, args, kw in zip(self._slaves, arglist, kwlist):
            func = getattr(prox, funcname)
            results.append(MockResult(func(*args, **kw)))
        return results

    def gather(self, results):
        if self._replay is not None:
            return self._replay
        else:
            return [res.get() for res in results]
