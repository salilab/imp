import modeller
import IMP
import IMP.test
import IMP.core
import IMP.modeller
from test_rsr_file_read import assertSimilarModellerIMPScores


class Tests(IMP.test.TestCase):

    """Check using IMP restraints in Modeller"""

    def test_imp_restraints(self):
        """Check using IMP restraints in Modeller"""
        e = modeller.environ()
        e.edat.dynamic_sphere = False
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = modeller.model(e)
        modmodel.build_sequence('GGCC')

        m = IMP.Model()
        protein = IMP.modeller.ModelLoader(modmodel).load_atoms(m)
        atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
        r = IMP.core.DistanceRestraint(m, IMP.core.Harmonic(10.0, 1.0),
                                       atoms[0], atoms[-1])
        sf = IMP.core.RestraintsScoringFunction([r])

        t = modmodel.env.edat.energy_terms
        t.append(IMP.modeller.IMPRestraints(atoms, sf))
        assertSimilarModellerIMPScores(self, sf, modmodel, protein)
        self.assertAlmostEqual(sf.evaluate(False), 9.80, delta=1e-2)

if __name__ == '__main__':
    IMP.test.main()
