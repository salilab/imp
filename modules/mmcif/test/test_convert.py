from __future__ import print_function
import IMP.test
import IMP.mmcif
import RMF
import IMP.rmf
import ihm
import ihm.reader
import os
import sys
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
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        self.add_chains(m, top)
        self.add_protocol(m, top, sampcon=True)
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


if __name__ == '__main__':
    IMP.test.main()
