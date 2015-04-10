#!/usr/bin/env python
import IMP
import IMP.core
import IMP.base
import IMP.algebra
import IMP.atom
import IMP.container
import IMP.isd


class WeightRestraint(object):

    def __init__(self, weight, lower, upper, kappa):

        self.weight = weight
        self.m = self.weight.get_model()
        self.label = "None"
        self.rs = IMP.RestraintSet(self.m, 'weight_restraint')
        self.lower = lower
        self.upper = upper
        self.kappa = kappa
        self.rs.add_restraint(
            IMP.isd.WeightRestraint(
                self.weight,
                self.lower,
                self.upper,
                self.kappa))

    def get_restraint(self, label):
        return self.rs

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def set_label(self, label):
        self.label = label

    def get_output(self):
        self.m.update()
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["WeightRestraint_" + self.label] = str(score)
        return output


class JeffreysPrior(object):

    def __init__(self, nuisance):

        self.m = nuisance.get_model()
        self.label = "None"
        self.rs = IMP.RestraintSet(self.m, 'jeffrey_prior')
        jp = IMP.isd.JeffreysRestraint(self.m, nuisance)
        self.rs.add_restraint(jp)

    def add_to_model(self):
        self.m.add_restraint(self.rs)

    def set_label(self, label):
        self.label = label

    def get_output(self):
        output = {}
        score = self.rs.unprotected_evaluate(None)
        output["_TotalScore"] = str(score)
        output["JeffreyPrior_" + self.label] = str(score)
        return output

#
