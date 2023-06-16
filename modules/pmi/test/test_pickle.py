import IMP.test
import IMP.pmi.dof
import IMP.pmi.io.crosslink
import IMP.pmi.topology
import IMP.pmi.macros
import IMP.pmi.restraints.stereochemistry
import IMP.pmi.restraints.crosslinking
import ihm.cross_linkers
import pickle
import shutil
import os

def make_system():
    m = IMP.Model()
    s = IMP.pmi.topology.System(m)
    st1 = s.create_state()
    mol = st1.create_molecule("Rpb1", "MVGQQMVGQQMVGQQMVGQQ")
    mol.add_representation(resolutions=[1])
    root_hier = s.build()

    dof = IMP.pmi.dof.DegreesOfFreedom(mol)
    dof.create_flexible_beads(mol, max_trans=3.0, resolution=1)

    mols = IMP.pmi.tools.get_molecules(root_hier)
    for mol in mols:
        molname = mol.get_name()
        cr = IMP.pmi.restraints.stereochemistry.ConnectivityRestraint(
            mol, scale=2.0, label=molname)
        cr.add_to_model()

    ev = IMP.pmi.restraints.stereochemistry.ExcludedVolumeSphere(
        included_objects=root_hier, resolution=1)
    ev.add_to_model()

    xldbkwc = IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
    xldbkwc.set_protein1_key("prot1")
    xldbkwc.set_protein2_key("prot2")
    xldbkwc.set_residue1_key("res1")
    xldbkwc.set_residue2_key("res2")

    xl2db = IMP.pmi.io.crosslink.CrossLinkDataBase(xldbkwc)
    with open('test_pickle.csv', 'w') as fh:
        fh.write("prot1,res1,prot2,res2\nRpb1,1,Rpb1,18\n")
    xl2db.create_set_from_file('test_pickle.csv')
    os.unlink('test_pickle.csv')

    xl2 = IMP.pmi.restraints.crosslinking.CrossLinkingMassSpectrometryRestraint(
        root_hier=root_hier, database=xl2db, length=21.0, slope=0.01,
        resolution=1.0, label="Chen", weight=1., linker=ihm.cross_linkers.dss)
    xl2.add_to_model()

    mc1 = IMP.pmi.macros.ReplicaExchange(
        m, root_hier=root_hier, monte_carlo_sample_objects=dof.get_movers(),
        monte_carlo_temperature=1.0, simulated_annealing=True,
        simulated_annealing_minimum_temperature=1.0,
        simulated_annealing_maximum_temperature=2.5,
        simulated_annealing_minimum_temperature_nframes=200,
        simulated_annealing_maximum_temperature_nframes=20,
        replica_exchange_minimum_temperature=1.0,
        replica_exchange_maximum_temperature=2.5,
        number_of_best_scoring_models=10, monte_carlo_steps=5,
        number_of_frames=5, global_output_directory="test_pickle_output")
    return mc1


class Tests(IMP.test.TestCase):
    def test_pickle(self):
        """Test that pickled ReplicaExchange objects work"""
        mc1 = make_system()
        dump = pickle.dumps((mc1.model, mc1))

        # Run the original ReplicaExchange and get the final score
        IMP.random_number_generator.seed(99)
        mc1.execute_macro()
        rs = IMP.pmi.tools.get_restraint_set(mc1.model)
        original_score = rs.evaluate(False)
        del mc1, rs

        # With the same random seed, we should get the exact same trajectory
        # with the pickled object
        newm, newmc1 = pickle.loads(dump)
        IMP.random_number_generator.seed(99)
        newmc1.execute_macro()
        rs = IMP.pmi.tools.get_restraint_set(newmc1.model)
        new_score = rs.evaluate(False)
        self.assertAlmostEqual(original_score, new_score, delta=1e-4)
        shutil.rmtree('test_pickle_output')


if __name__ == '__main__':
    IMP.test.main()
