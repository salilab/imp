import modeller
import IMP
import IMP.test
import IMP.core
import IMP.modeller
from test_rsr_file_read import assertSimilarModellerIMPScores

class Tests(IMP.test.TestCase):
    """Check using Modeller restraints in IMP"""

    def test_modeller_restraints(self):
        """Check using Modeller restraints in IMP"""
        e = modeller.environ()
        e.edat.dynamic_sphere = False
        e.libs.topology.read('${LIB}/top_heav.lib')
        e.libs.parameters.read('${LIB}/par.lib')
        modmodel = modeller.model(e)
        modmodel.build_sequence('GGCC')
        feat = modeller.features.distance(modmodel.atoms[0], modmodel.atoms[-1])
        r = modeller.forms.gaussian(feature=feat, mean=10.0, stdev=1.0,
                                    group=modeller.physical.xy_distance)
        modmodel.restraints.add(r)

        m = IMP.Model()
        protein = IMP.modeller.ModelLoader(modmodel).load_atoms(m)
        atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
        m.add_restraint(IMP.modeller.ModellerRestraints(m, modmodel,
                                                        atoms))

        assertSimilarModellerIMPScores(self, modmodel, protein)
        self.assertAlmostEqual(m.evaluate(False), 5.7837, delta=1e-3)

if __name__ == '__main__':
    IMP.test.main()
