import os
import modeller
import IMP
import IMP.test
import IMP.core
import IMP.modeller


class Tests(IMP.test.TestCase):

    def test_soft_sphere(self):
        """Check loading of Modeller soft sphere restraints"""
        e = modeller.Environ()
        e.edat.dynamic_sphere = True
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = modeller.Model(e)
        modmodel.build_sequence('GGCC')

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)

        sf = IMP.core.RestraintsScoringFunction(list(
                                       loader.load_dynamic_restraints()))

        modenergy = modeller.Selection(modmodel).energy()[0]
        imp_score = sf.evaluate(True)
        self.assertAlmostEqual(imp_score, modenergy, delta=1e-3)

    def test_lennard_jones(self):
        """Check loading of Modeller Lennard-Jones restraints"""
        e = modeller.Environ()
        e.edat.dynamic_sphere = False
        e.edat.dynamic_lennard = True
        e.edat.contact_shell = 8.0
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = modeller.Model(e)
        modmodel.build_sequence('GGCC')

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)

        sf = IMP.core.RestraintsScoringFunction(list(
                                       loader.load_dynamic_restraints()))

        modenergy = modeller.Selection(modmodel).energy()[0]
        imp_score = sf.evaluate(True)
        self.assertAlmostEqual(imp_score, modenergy, delta=1e-3)

    def test_coulomb(self):
        """Check loading of Modeller electrostatic restraints"""
        e = modeller.Environ()
        e.edat.dynamic_sphere = False
        e.edat.dynamic_coulomb = True
        e.edat.contact_shell = 8.0
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = modeller.Model(e)
        modmodel.build_sequence('GGC')

        m = IMP.Model()
        loader = IMP.modeller.ModelLoader(modmodel)
        protein = loader.load_atoms(m)

        sf = IMP.core.RestraintsScoringFunction(list(
                                       loader.load_dynamic_restraints()))

        modenergy = modeller.Selection(modmodel).energy()[0]
        imp_score = sf.evaluate(True)
        self.assertAlmostEqual(imp_score, modenergy, delta=1e-3)

if __name__ == '__main__':
    IMP.test.main()
