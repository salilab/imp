from __future__ import print_function
import IMP.test
import IMP.mmcif
import RMF
import IMP.rmf
import ihm
import ihm.reader
import os
import sys
from test_restraints import MockGaussianEMRestraint
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO


class Tests(IMP.test.TestCase):
    def add_state(self, m, top, state_index, name):
        h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state = IMP.atom.State.setup_particle(h, state_index)
        state.set_name(name)
        top.add_child(state)
        return state

    def add_chains(self, m, top, chains=None):
        if chains is None:
            chains = (('foo', 'ACGT', 'X'), ('bar', 'ACGT', 'Y'),
                      ('baz', 'ACC', 'Z'))
        for name, seq, cid in chains:
            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            mol = IMP.atom.Molecule.setup_particle(h)
            mol.set_name(name)
            top.add_child(mol)

            h = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            chain = IMP.atom.Chain.setup_particle(h, cid)
            chain.set_sequence(seq)
            mol.add_child(chain)

    def add_protocol(self, m, top, sampcon=False):
        prov = IMP.core.SampleProvenance.setup_particle(
            m, IMP.Particle(m), "Monte Carlo", 100, 5)
        IMP.core.add_provenance(m, top, prov)

        prov = IMP.core.CombineProvenance.setup_particle(m, IMP.Particle(m),
                                                         5, 500)
        IMP.core.add_provenance(m, top, prov)

        prov = IMP.core.FilterProvenance.setup_particle(
            m, IMP.Particle(m), "Total score", 100.5, 400)
        IMP.core.add_provenance(m, top, prov)

        prov = IMP.core.SampleProvenance.setup_particle(
            m, IMP.Particle(m), "Molecular Dynamics", 2000, 5, 16)
        IMP.core.add_provenance(m, top, prov)

        prov = IMP.core.ClusterProvenance.setup_particle(m, IMP.Particle(m), 10)
        if sampcon:
            prov.set_name("cluster.0")
            prov.set_precision(42.0)
            prov.set_density(self.get_input_file_name("sampcon.json"))
        IMP.core.add_provenance(m, top, prov)

    def add_structured_residue(self, m, top, ind):
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, ind)
        IMP.core.XYZR.setup_particle(
            residue, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        IMP.atom.Mass.setup_particle(residue, 1.0)
        top.add_child(residue)
        return residue

    def test_no_chains(self):
        """Trying to add a Hierarchy with no chains should give an error"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        c = IMP.mmcif.Convert()
        self.assertIsNone(c.system.title)
        self.assertRaises(ValueError, c.add_model, [top], [])

    def test_no_state_node(self):
        """Test reading a hierarchy containing no State nodes"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        top.set_name("Top node")
        self.add_chains(m, top)
        c = IMP.mmcif.Convert()
        c.add_model([top], [])
        self.assertEqual([x.description for x in c.system.entities],
                         ['foo', 'baz'])
        self.assertEqual([x.details for x in c.system.asym_units],
                         ['foo', 'bar', 'baz'])
        self.assertEqual([x.id for x in c.system.asym_units],
                         ['X', 'Y', 'Z'])
        self.assertEqual(c.system.title, "Top node")
        # Should be a single unnamed State
        self.assertEqual(len(c.system.state_groups), 1)
        state, = c.system.state_groups[0]
        self.assertIsNone(state.name)
        # Single ensemble
        self.assertEqual(len(c.system.ensembles), 1)

    def test_add_rmf(self):
        """Test add_rmf() convenience method"""
        def make_rmf(fname):
            m = IMP.Model()
            top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            self.add_chains(m, top)
            # Need to add coordinates/mass to leaves to satisfy RMF
            for molecule in top.get_children():
                chain = molecule.get_child(0)
                leaf = IMP.core.XYZR.setup_particle(
                    IMP.Particle(m),
                    IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 1.0))
                IMP.atom.Mass.setup_particle(leaf, 1.0)
                chain.add_child(leaf)
            top.set_name("Top node")
            f = RMF.create_rmf_file(fname)
            IMP.rmf.add_hierarchy(f, top)
            IMP.rmf.save_frame(f, "0")
            f.close()

        fname = self.get_tmp_file_name("test_add_rmf.rmf3")
        make_rmf(fname)

        c = IMP.mmcif.Convert()
        c.add_rmf(fname)
        self.assertEqual(len(c.system.entities), 2)
        self.assertEqual(len(c.system.asym_units), 3)
        self.assertEqual(c.system.title, "Top node")
        os.unlink(fname)

    def test_two_states(self):
        """Test reading a simple hierarchy containing two States"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top, 0, "State_0")
        self.add_chains(m, state0)
        state1 = self.add_state(m, top, 1, "State_1")
        self.add_chains(m, state1)
        c = IMP.mmcif.Convert()
        c.add_model([top], [])
        # Entities/asyms should not be duplicated
        self.assertEqual([x.description for x in c.system.entities],
                         ['foo', 'baz'])
        self.assertEqual([x.details for x in c.system.asym_units],
                         ['foo', 'bar', 'baz'])
        self.assertEqual([x.id for x in c.system.asym_units],
                         ['X', 'Y', 'Z'])
        # Should be two states
        self.assertEqual(len(c.system.state_groups), 1)
        self.assertEqual([x.name for x in c.system.state_groups[0]],
                         ["State_0", "State_1"])
        # Ensemble for each state
        self.assertEqual(len(c.system.ensembles), 2)

    def test_filter_states(self):
        """Test filtering with `states` argument"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top, 0, "State_0")
        self.add_chains(m, state0)
        state1 = self.add_state(m, top, 1, "State_1")
        self.add_chains(m, state1)

        # No filtering -> two states
        c = IMP.mmcif.Convert()
        c.add_model([top], [])
        self.assertEqual(len(c.system.state_groups[0]), 2)

        c = IMP.mmcif.Convert()
        c.add_model([top], [], states=["State_0"])
        self.assertEqual(len(c.system.state_groups[0]), 1)

        # No states selected (and so no groups either)
        c = IMP.mmcif.Convert()
        c.add_model([top], [], states=["State_99"])
        self.assertEqual(len(c.system.state_groups), 0)

    def test_group_ensembles(self):
        """Test grouping models into ensembles"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top)

        c = IMP.mmcif.Convert()
        c.add_model([top], [])
        c.add_model([top], [])
        # No grouping -> ensemble for each model
        self.assertEqual(len(c.system.ensembles), 2)

        # Group into single ensemble
        c = IMP.mmcif.Convert()
        ens = c.add_model([top], [])
        c.add_model([top], [], ensembles=ens)
        self.assertEqual(len(c.system.ensembles), 1)

    def test_write(self):
        """Test Convert.write() method"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top)
        c = IMP.mmcif.Convert()
        c.add_model([top], [])
        fname = 'test_write.cif'
        c.write(fname)
        with open(fname) as fh:
            s, = ihm.reader.read(fh)
        self.assertEqual(len(s.entities), 2)
        self.assertEqual(len(s.asym_units), 3)
        os.unlink(fname)

    def test_report(self):
        """Test Convert.report() method"""
        sio = StringIO()
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top)
        c = IMP.mmcif.Convert()
        c.add_model([top], [])
        c.report(sio)

    def test_sampcon_ensemble(self):
        """Test ensemble information from IMP.sampcon output"""
        m = IMP.Model()

        # No state node, clustering info on top node
        top1 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top1)
        self.add_protocol(m, top1, sampcon=True)

        # State node, clustering info on top node
        top2 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top2, 0, "State_0")
        self.add_chains(m, state0)
        self.add_protocol(m, top2, sampcon=True)

        # State node, clustering info on state node
        top3 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top3, 0, "State_0")
        self.add_chains(m, state0)
        self.add_protocol(m, state0, sampcon=True)

        for top in (top1, top2, top3):
            c = IMP.mmcif.Convert()
            c.add_model([top], [])

            e, = c.system.ensembles
            # Name and precision should be assigned based on sampcon output
            self.assertEqual(e.name, "cluster.0")
            self.assertAlmostEqual(e.precision, 42.0, delta=1e-4)

            den1, den2 = e.densities
            self.assertEqual(den1.asym_unit.details, 'foo')
            self.assertEqual(den1.asym_unit.seq_id_range, (1, 4))
            self.assertEqual(os.path.basename(den1.file.path), 'test_1.mrc')
            self.assertEqual(den2.asym_unit.details, 'bar')
            self.assertEqual(den2.asym_unit.seq_id_range, (2, 3))
            self.assertEqual(os.path.basename(den2.file.path), 'test_2.mrc')

    def test_software(self):
        """Test that software information is collected"""
        def add_software(m, top):
            prov = IMP.core.SoftwareProvenance.setup_particle(
                IMP.Particle(m), "testname", "testver", "testloc")
            IMP.core.add_provenance(m, top, prov)

        m = IMP.Model()

        # No state node, software info on top node
        top1 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top1)
        add_software(m, top1)

        # State node, software info on top node
        top2 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top2, 0, "State_0")
        self.add_chains(m, state0)
        add_software(m, top2)

        # State node, software info on state node
        top3 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top3, 0, "State_0")
        self.add_chains(m, state0)
        add_software(m, state0)

        for top in (top1, top2, top3):
            c = IMP.mmcif.Convert()
            c.add_model([top], [])

            soft, = c.system.software
            self.assertEqual(soft.name, "testname")
            self.assertEqual(soft.version, "testver")
            self.assertEqual(soft.location, "testloc")

    def test_external_files(self):
        """Test that Python script info is collected"""
        def add_script(m, top):
            prov = IMP.core.ScriptProvenance.setup_particle(
                IMP.Particle(m), self.get_input_file_name("test_1.mrc"))
            IMP.core.add_provenance(m, top, prov)

        m = IMP.Model()

        # No state node, script info on top node
        top1 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top1)
        add_script(m, top1)

        # State node, script info on top node
        top2 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top2, 0, "State_0")
        self.add_chains(m, state0)
        add_script(m, top2)

        # State node, script info on state node
        top3 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top3, 0, "State_0")
        self.add_chains(m, state0)
        add_script(m, state0)

        for top in (top1, top2, top3):
            c = IMP.mmcif.Convert()
            c.add_model([top], [])

            loc, = c.system.locations
            self.assertEqual(os.path.basename(loc.path), 'test_1.mrc')

    def test_same_assembly(self):
        """Test reading multiple models with the same assembly"""
        m = IMP.Model()
        top1 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top1)

        top2 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top2)

        c = IMP.mmcif.Convert()
        c.add_model([top1, top2], [])
        self.assertEqual(len(c.system.orphan_assemblies), 1)
        self.assertEqual(len(c.system.orphan_assemblies[0]), 3)

    def test_different_assembly(self):
        """Test reading multiple models with different assemblies"""
        m = IMP.Model()
        top1 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top1)

        top2 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top2,
                        chains = (('foo', 'ACGT', 'X'), ('bar', 'ACGT', 'Y')))

        c = IMP.mmcif.Convert()
        c.add_model([top1, top2], [])
        self.assertEqual(len(c.system.orphan_assemblies), 2)
        self.assertEqual(len(c.system.orphan_assemblies[0]), 3)
        self.assertEqual(len(c.system.orphan_assemblies[1]), 2)

    def test_model_creation(self):
        """Test creation of ihm Model objects"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top)
        c = IMP.mmcif.Convert()
        chain0 = top.get_child(0).get_child(0)
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 1)
        IMP.core.XYZR.setup_particle(
            residue, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        IMP.atom.Mass.setup_particle(residue, 1.0)
        chain0.add_child(residue)
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.HIS, 2)
        atom = IMP.atom.Atom.setup_particle(IMP.Particle(m),
                                            IMP.atom.AT_CA)
        IMP.core.XYZR.setup_particle(
            atom, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5, 6, 7), 8))
        residue.add_child(atom)
        chain0.add_child(residue)
        frag = IMP.atom.Fragment.setup_particle(IMP.Particle(m), [3, 4])
        chain0.add_child(frag)
        IMP.core.XYZR.setup_particle(
            frag, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(9, 10, 11), 12))
        IMP.atom.Mass.setup_particle(frag, 1.0)
        c.add_model([top], [])
        model, = c.system.state_groups[0][0][0]
        # Representation should contain residue, atom, fragment
        r1, r2, r3 = model.representation
        self.assertIsInstance(r1, ihm.representation.ResidueSegment)
        self.assertIsInstance(r2, ihm.representation.AtomicSegment)
        self.assertIsInstance(r3, ihm.representation.FeatureSegment)
        # Coordinates should contain one atom, two spheres
        a1, = model._atoms
        self.assertEqual(a1.seq_id, 2)
        self.assertEqual(a1.atom_id, 'CA')
        self.assertEqual(a1.type_symbol, 'C')
        self.assertFalse(a1.het)
        s1, s2 = model._spheres
        self.assertEqual(s1.seq_id_range, (1, 1))
        self.assertAlmostEqual(s1.radius, 4.0, delta=1e-2)
        self.assertEqual(s2.seq_id_range, (3, 4))
        self.assertAlmostEqual(s2.radius, 12.0, delta=1e-2)

    def test_starting_models(self):
        """Test handling of starting models"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        state0 = self.add_state(m, top, 0, "State_0")
        self.add_chains(m, state0)
        state1 = self.add_state(m, top, 1, "State_1")
        self.add_chains(m, state1)

        chain0 = state0.get_child(0).get_child(0)
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 1)
        IMP.core.XYZR.setup_particle(
            residue, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        IMP.atom.Mass.setup_particle(residue, 1.0)

        prov = IMP.core.StructureProvenance.setup_particle(
            m, IMP.Particle(m), self.get_input_file_name("test.nup84.pdb"),
            "X")
        IMP.core.add_provenance(m, residue, prov)

        chain0.add_child(residue)

        chain1 = state1.get_child(0).get_child(0)
        frag = IMP.atom.Fragment.setup_particle(IMP.Particle(m), [1])

        # Provenance here is attached to the parent fragment, not the
        # residue directly
        prov = IMP.core.StructureProvenance.setup_particle(
            m, IMP.Particle(m), self.get_input_file_name("test.nup84.pdb"),
            "X")
        IMP.core.add_provenance(m, frag, prov)

        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 1)
        IMP.core.XYZR.setup_particle(
            residue, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        IMP.atom.Mass.setup_particle(residue, 1.0)
        frag.add_child(residue)
        chain1.add_child(frag)

        c = IMP.mmcif.Convert()
        c.add_model([top], [])
        # Both models should have same representation, with same starting model
        state0_model, = c.system.state_groups[0][0][0]
        state1_model, = c.system.state_groups[0][1][0]
        self.assertIs(state0_model.representation, state1_model.representation)
        s1, = state0_model.representation
        self.assertEqual(s1.starting_model.asym_id, 'X')

    def test_em3d_restraint(self):
        """Test handling of 3D EM restraints"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top)
        c = IMP.mmcif.Convert()
        chain0 = top.get_child(0).get_child(0)
        r1 = self.add_structured_residue(m, chain0, 1)
        self.add_structured_residue(m, chain0, 2)
        r3 = chain1 = top.get_child(1).get_child(0)
        self.add_structured_residue(m, chain1, 1)

        em_filename = self.get_input_file_name('test.gmm.txt')
        em1 = MockGaussianEMRestraint(m, em_filename, [r1.get_particle()])
        em2 = MockGaussianEMRestraint(m, em_filename, [r1.get_particle()])
        em3 = MockGaussianEMRestraint(m, em_filename, [r1.get_particle(),
                                                       r3.get_particle()])

        c.add_model([top], [em1, em2, em3])
        # Duplicate restraint em2 should be consolidated with em1
        cr1, cr2 = c.system.restraints
        self.assertEqual([asym.id for asym in cr1.assembly], ['X'])
        self.assertEqual(os.path.basename(cr1.dataset.location.path),
                         'test.gmm.txt')
        self.assertEqual([asym.id for asym in cr2.assembly], ['X', 'Y'])
        self.assertEqual(os.path.basename(cr2.dataset.location.path),
                         'test.gmm.txt')



if __name__ == '__main__':
    IMP.test.main()
