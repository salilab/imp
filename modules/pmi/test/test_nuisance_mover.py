from __future__ import print_function
import IMP
import IMP.core
import IMP.test
import IMP.algebra
import IMP.atom
import IMP.container

import IMP.pmi.tools as tools
import IMP.pmi.samplers as samplers


class XTransRestraint(IMP.Restraint):

    def __init__(self, m):
        IMP.Restraint.__init__(self, m, "XTransRestraint %1%")

    def unprotected_evaluate(self, accum):
        e = 0
        return e

    def get_version_info(self):
        return IMP.VersionInfo("", "")

    def do_show(self, fh):
        fh.write("Test restraint")

    def do_get_inputs(self):
        m = self.get_model()
        return IMP.get_particles(m, m.get_particle_indexes())

class Tests(IMP.test.TestCase):
    def test_nuisance_mover(self):
        m = IMP.Model()
        sigma = tools.SetupNuisance(m, 0, 0, 10, True).get_particle()
        mv = IMP.core.NormalMover(
            [sigma], [IMP.FloatKey("nuisance")], 1.0)

        rs = XTransRestraint(m)
        IMP.pmi.tools.add_restraint_to_model(m, rs)

        mc = samplers.MonteCarlo(m, [mv], 1.0)
        for i in range(100):
            mc.optimize(1)
            print(sigma.get_scale())

if __name__ == '__main__':
    IMP.test.main()
