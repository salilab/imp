import utils
import os
import unittest
import sys

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.model

class Tests(unittest.TestCase):

    def test_sphere(self):
        """Test Sphere class"""
        s = ihm.model.Sphere(asym_unit='foo', seq_id_range=(1,5), x=1.0,
                             y=2.0, z=3.0, radius=4.0)
        self.assertEqual(s.asym_unit, 'foo')
        self.assertEqual(s.seq_id_range, (1,5))

    def test_atom(self):
        """Test Atom class"""
        s = ihm.model.Atom(asym_unit='foo', seq_id=1, atom_id='N',
                           type_symbol='N', x=1.0, y=2.0, z=3.0)
        self.assertEqual(s.asym_unit, 'foo')
        self.assertEqual(s.seq_id, 1)

    def test_model(self):
        """Test Model class"""
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        self.assertEqual(m.name, None)
        self.assertEqual(m.protocol, 'bar')

    def test_model_get_spheres(self):
        """Test Model.get_spheres()"""
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        spheres = ['sphere1', 'sphere2']
        m._spheres = spheres[:]
        new_spheres = [s for s in m.get_spheres()]
        self.assertEqual(new_spheres, spheres)

    def test_model_add_sphere(self):
        """Test Model.add_sphere()"""
        spheres = ['sphere1', 'sphere2']
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m.add_sphere(spheres[0])
        m.add_sphere(spheres[1])
        self.assertEqual(m._spheres, spheres)

    def test_model_get_atoms(self):
        """Test Model.get_atoms()"""
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        atoms = ['atom1', 'atom2']
        m._atoms = atoms[:]
        new_atoms = [a for a in m.get_atoms()]
        self.assertEqual(new_atoms, atoms)

    def test_model_add_atom(self):
        """Test Model.add_atom()"""
        atoms = ['atom1', 'atom2']
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m.add_atom(atoms[0])
        m.add_atom(atoms[1])
        self.assertEqual(m._atoms, atoms)

    def test_model_group(self):
        """Test ModelGroup class"""
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        g = ihm.model.ModelGroup([m], name='foo')
        self.assertEqual(g.name, 'foo')
        self.assertEqual(g[0].protocol, 'bar')

    def test_ensemble(self):
        """Test Ensemble class"""
        e = ihm.model.Ensemble(model_group=['m1', 'm2', 'm3'],
                               num_models=10, name='bar')
        self.assertEqual(e.name, 'bar')
        self.assertEqual(e.precision, None)
        self.assertEqual(e.num_models, 10)
        self.assertEqual(e.num_models_deposited, 3)
        self.assertEqual(e.densities, [])

    def test_density(self):
        """Test LocalizationDensity class"""
        e = ihm.model.LocalizationDensity(file='foo', asym_unit='bar')
        self.assertEqual(e.file, 'foo')
        self.assertEqual(e.asym_unit, 'bar')

    def test_state(self):
        """Test State class"""
        s = ihm.model.State(name='foo')
        self.assertEqual(s.name, 'foo')
        self.assertEqual(s, [])

    def test_state_group(self):
        """Test StateGroup class"""
        s = ihm.model.StateGroup()
        self.assertEqual(s, [])

    def test_ordered_process(self):
        """Test OrderedProcess class"""
        p = ihm.model.OrderedProcess("time steps", "test process")
        self.assertEqual(p.ordered_by, "time steps")
        self.assertEqual(p.description, "test process")
        self.assertEqual(p.steps, [])
        s = ihm.model.ProcessStep(description="foo")
        self.assertEqual(s.description, "foo")
        p.steps.append(s)
        e = ihm.model.ProcessEdge('from', 'to')
        s.append(e)
        self.assertEqual(p.steps[0][0].group_begin, 'from')
        self.assertEqual(p.steps[0][0].group_end, 'to')
        self.assertEqual(p.steps[0][0].description, None)

    def test_dcd_writer(self):
        """Test DCDWriter class"""
        m1 = ihm.model.Model(None, None, None)
        m1._atoms = [ihm.model.Atom(None, None, None, None, x=1,y=2,z=3),
                     ihm.model.Atom(None, None, None, None, x=4,y=5,z=6)]

        with utils.temporary_directory() as tmpdir:
            dcd = os.path.join(tmpdir, 'out.dcd')

            with open(dcd, 'wb') as fh:
                d = ihm.model.DCDWriter(fh)
                d.add_model(m1)
                d.add_model(m1)

            with open(dcd, 'rb') as fh:
                contents = fh.read()
        self.assertEqual(len(contents), 452)

    def test_dcd_writer_framesize_mismatch(self):
        """Test DCDWriter class with framesize mismatch"""
        m1 = ihm.model.Model(None, None, None)
        m1._atoms = [ihm.model.Atom(None, None, None, None, x=1,y=2,z=3),
                     ihm.model.Atom(None, None, None, None, x=4,y=5,z=6)]
        m2 = ihm.model.Model(None, None, None)
        m2._atoms = [ihm.model.Atom(None, None, None, None, x=1,y=2,z=3)]

        with utils.temporary_directory() as tmpdir:
            dcd = os.path.join(tmpdir, 'out.dcd')

            with open(dcd, 'wb') as fh:
                d = ihm.model.DCDWriter(fh)
                d.add_model(m1)
                self.assertRaises(ValueError, d.add_model, m2)


if __name__ == '__main__':
    unittest.main()
