import IMP
import IMP.atom
import IMP.pmi
import IMP.pmi.topology
import IMP.test
import IMP.pmi.restraints.basic
import math


class Tests(IMP.test.TestCase):


    def test_functionalities(self):
        mdl = IMP.Model()
        s = IMP.pmi.topology.System(mdl)
        st1 = s.create_state()
        seqs = IMP.pmi.topology.Sequences(
            IMP.pmi.get_example_path('data/gcp2.fasta'))
        mol = st1.create_molecule(
            "GCP2", sequence=seqs["GCP2_YEAST"][:100], chain_id='A')
        a1 = mol.add_structure(IMP.pmi.get_example_path('data/gcp2.pdb'),
                               res_range=(1, 100),
                               chain_id='A')
        gmm_prefix = self.get_input_file_name('gcp2_gmm.txt').strip('.txt')
        mol.add_representation(a1,
                               resolutions=[10, 100],
                               density_prefix=gmm_prefix,
                               density_residues_per_component=20,
                               density_voxel_size=3.0)
        mol.add_representation(mol.get_non_atomic_residues(),
                               resolutions=[10],
                               setup_particles_as_densities=True)
        hier = s.build()

        cr=IMP.pmi.restraints.basic.CylinderRestraint(mdl,mol[:100],10,20)
        cr.set_was_used(True)
        self.assertAlmostEqual(cr.unprotected_evaluate(None),
                               97.8378602841374, delta=1e-5)
        cr.add_to_model()
        self.assertEqual(len(cr.do_get_inputs()),12)
        cr.get_output()

    def test_values(self):
        mdl = IMP.Model()
        p=IMP.Particle(mdl)
        d=IMP.core.XYZR.setup_particle(p)
        d.set_coordinates((0,0,0))
        d.set_radius(1.0)
        IMP.atom.Mass.setup_particle(p,1.0)
        h=IMP.atom.Hierarchy.setup_particle(p)
        cr=IMP.pmi.restraints.basic.CylinderRestraint(mdl,[h],10,20)
        cr.set_was_used(True)
        for r in range(100):
            d.set_coordinates((r,0,0))
            cr.unprotected_evaluate(None)


    def test_angles(self):
        mdl = IMP.Model()
        p=IMP.Particle(mdl)
        d=IMP.core.XYZR.setup_particle(p)
        d.set_coordinates((0,0,0))
        d.set_radius(1.0)
        IMP.atom.Mass.setup_particle(p,1.0)
        h=IMP.atom.Hierarchy.setup_particle(p)
        cr=IMP.pmi.restraints.basic.CylinderRestraint(mdl,[h],10,20,-72,72)
        cr.set_was_used(True)
        for angle in range(360):
            anglerad=float(angle)/180.0*math.pi
            d.set_coordinates((math.cos(anglerad),math.sin(anglerad),0))

if __name__ == '__main__':
    IMP.test.main()
