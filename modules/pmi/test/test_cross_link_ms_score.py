import ihm.cross_linkers
import IMP.pmi.topology
import IMP.pmi.dof
import IMP.pmi.io.crosslink
import IMP.pmi.restraints.crosslinking
import IMP.test
try:
    import jax
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):
    def make_repr(self, m):
        s = IMP.pmi.topology.System(m)
        st = s.create_state()
        protA = st.create_molecule("ProtA",sequence='A'*30,chain_id='A')
        protA.add_representation(protA[0:10],
                                 resolutions=[1], bead_default_coord=[0,0,0])
        protA.add_representation(protA[10:20],
                                 resolutions=[1], bead_default_coord=[10,0,0])
        protA.add_representation(protA[20:30],
                                 resolutions=[1], bead_default_coord=[20,0,0])
        protB = st.create_molecule("ProtB",
                                   sequence='A'*30, chain_id='B')
        protB.add_representation(protB[0:10],
                                 resolutions=[1], bead_default_coord=[0,0,0])
        protB.add_representation(protB[10:20],
                                 resolutions=[1], bead_default_coord=[10,0,0])
        protB.add_representation(protB[20:30],
                                 resolutions=[1], bead_default_coord=[20,0,0])
        hier = s.build()
        dof = IMP.pmi.dof.DegreesOfFreedom(m)
        dof.create_flexible_beads(protA)
        dof.create_flexible_beads(protB)
        return hier, dof

    def setup_crosslinks_beads(self, root_hier):
        cldbkc = IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_unique_id_key("Unique ID")
        cldbkc.set_protein1_key("Protein 1")
        cldbkc.set_protein2_key("Protein 2")
        cldbkc.set_residue1_key("Residue 1")
        cldbkc.set_residue2_key("Residue 2")
        cldbkc.set_id_score_key("ID Score")
        cldb = IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(
            self.get_input_file_name("expensive_test_new_cross_link_ms_restraint.csv"))

        xl = IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint(
            root_hier=root_hier, database=cldb, length=21, label="XL",
            resolution=1, linker=ihm.cross_linkers.dss, slope=0.01)

        return xl, cldb

    def test_score(self):
        """Test score of crosslinking restraint"""
        m = IMP.Model()
        hier, dof = self.make_repr(m)
        xl, cldb = self.setup_crosslinks_beads(hier)
        xl.add_to_model()

        rs = IMP.pmi.tools.get_restraint_set(m)
        exp_score = -0.13949
        imp_score = rs.evaluate(False)
        self.assertAlmostEqual(imp_score, exp_score, delta=1e-4)

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_score_jax(self):
        """Test JAX score of crosslinking restraint"""
        m = IMP.Model()
        hier, dof = self.make_repr(m)
        xl, cldb = self.setup_crosslinks_beads(hier)
        xl.add_to_model()

        rs = IMP.pmi.tools.get_restraint_set(m)
        exp_score = -0.13949
        ji = rs._get_jax()
        jm = ji.get_jax_model()
        score_f = jax.jit(ji.score_func)
        jax_score = score_f(jm)
        self.assertAlmostEqual(jax_score, exp_score, delta=1e-4)
        # JAX score should also match the IMP (C++) score
        imp_score = rs.evaluate(False)
        self.assertAlmostEqual(jax_score, imp_score, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
