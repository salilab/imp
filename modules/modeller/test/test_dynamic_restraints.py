import os
from modeller import *
import IMP
import IMP.test
import IMP.core
import IMP.modeller

class Tests(IMP.test.TestCase):

    def test_soft_sphere(self):
        """Check loading of Modeller soft sphere restraints"""
        e = environ()
        e.edat.dynamic_sphere = True
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = model(e)
        modmodel.build_sequence('GGCC')

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)

        for r in loader.load_dynamic_restraints():
            m.add_restraint(r)

        modenergy = selection(modmodel).energy()[0]
        imp_score = m.evaluate(True)
        self.assertAlmostEqual(imp_score, modenergy, delta=1e-3)

    def test_lennard_jones(self):
        """Check loading of Modeller Lennard-Jones restraints"""
        e = environ()
        e.edat.dynamic_sphere = False
        e.edat.dynamic_lennard = True
        e.edat.contact_shell = 8.0
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = model(e)
        modmodel.build_sequence('GGCC')

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)

        for r in loader.load_dynamic_restraints():
            m.add_restraint(r)

        modenergy = selection(modmodel).energy()[0]
        imp_score = m.evaluate(True)
        self.assertAlmostEqual(imp_score, modenergy, delta=1e-3)

    def test_coulomb(self):
        """Check loading of Modeller electrostatic restraints"""
        e = environ()
        e.edat.dynamic_sphere = False
        e.edat.dynamic_coulomb = True
        e.edat.contact_shell = 8.0
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = model(e)
        modmodel.build_sequence('GGC')

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)

        for r in loader.load_dynamic_restraints():
            m.add_restraint(r)

        modenergy = selection(modmodel).energy()[0]
        imp_score = m.evaluate(True)
        self.assertAlmostEqual(imp_score, modenergy, delta=1e-3)

if __name__ == '__main__':
    IMP.test.main()
