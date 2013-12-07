"""Interface between XML Representation and IMP Model."""

import IMP
import IMP.atom
import random
import math


class Optimization(object):

    def __init__(self):
        self._children = list()

    def run(self, restraint, log):
        for child in self._children:
            child.run(restraint, log)


class _OptimizationNode(object):
    counter = 0

    def __init__(self, attributes, text):
        id = attributes.get('id')
        if id:
            self.id = id
        else:
            self.id = 'object_%d' % _OptimizationNode.counter
            _OptimizationNode.counter += 1
        self._children = list()


class _OptimizationConjugateGradients(_OptimizationNode):

    def __init__(self, attributes, text):
        _OptimizationNode.__init__(self, attributes, text)
        self.threshold = float(attributes.get('threshold', 0))
        self.steps = int(attributes.get('steps', 1))

    def run(self, restraint, log):
        restraint.clear()
        restraint.set_include(include=True)
        for child in self._children:
            child.run(restraint, log)
        restraint.add_to_model()
        o = IMP.core.ConjugateGradients()
        o.set_model(restraint._model)
        if log:
            o.add_optimizer_state(log)
        o.optimize(self.steps)


class _OptimizationRestraint(_OptimizationNode):

    def __init__(self, attributes, text):
        _OptimizationNode.__init__(self, attributes, text)
        self.name = attributes['name']
        self.weight = float(attributes.get('weight', 1.0))

    def run(self, restraint, log):
        restraint.set_include(name=self.name, include=True,
                              weight=self.weight)
