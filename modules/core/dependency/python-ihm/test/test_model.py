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
        s = ihm.model.Atom(asym_unit='foo', seq_id=1, atom_id='N', x=1.0,
                             y=2.0, z=3.0)
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

    def test_model_set_spheres(self):
        """Test Model.set_spheres()"""
        spheres = ['sphere1', 'sphere2']
        def spheregen():
            for s in spheres:
                yield s
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m.set_spheres(spheregen())
        self.assertEqual(m._spheres, spheres)

    def test_model_get_atoms(self):
        """Test Model.get_atoms()"""
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        atoms = ['atom1', 'atom2']
        m._atoms = atoms[:]
        new_atoms = [a for a in m.get_atoms()]
        self.assertEqual(new_atoms, atoms)

    def test_model_set_atoms(self):
        """Test Model.set_atoms()"""
        atoms = ['atom1', 'atom2']
        def atomgen():
            for a in atoms:
                yield a
        m = ihm.model.Model(assembly='foo', protocol='bar',
                            representation='baz')
        m.set_atoms(atomgen())
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


if __name__ == '__main__':
    unittest.main()
