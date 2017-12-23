from __future__ import print_function
import IMP.test
import IMP.atom
import IMP.mmcif.data

class Tests(IMP.test.TestCase):
    def test_get_molecule(self):
        """Test get_molecule()"""
        m = IMP.Model()
        h1 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        h2 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        h3 = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        h1.add_child(h2)
        h2.add_child(h3)
        IMP.atom.Molecule.setup_particle(h2)
        self.assertEqual(IMP.mmcif.data.get_molecule(h3), h2)
        self.assertEqual(IMP.mmcif.data.get_molecule(h2), h2)
        self.assertIsNone(IMP.mmcif.data.get_molecule(h1))

    def test_assign_id(self):
        """Test _assign_id utility function"""
        class DummyObj(object):
            def __init__(self, hashval):
                self.hashval = hashval
            def __eq__(self, other):
                return self.hashval == other.hashval
            def __hash__(self):
                return self.hashval
        seen_objs = {}
        obj_by_id = []
        obj1a = DummyObj(42) # obj1a and 1b are identical
        obj1b = DummyObj(42)
        obj2 = DummyObj(34)
        obj3 = DummyObj(23) # obj3 already has an id
        obj3.id = 'foo'
        for obj in (obj1a, obj1b, obj2, obj3):
            IMP.mmcif.data._assign_id(obj, seen_objs, obj_by_id)
        self.assertEqual(obj1a.id, 1)
        self.assertEqual(obj1b.id, 1)
        self.assertEqual(obj2.id, 2)
        self.assertEqual(obj3.id, 'foo')
        self.assertEqual(obj_by_id, [obj1a, obj2])

    def test_component_mapper_same_id_chain(self):
        """Test ComponentMapper given two chains with same ID"""
        class MockChain(object):
            def __init__(self, name):
                self.name = name
        cm = IMP.mmcif.data._ComponentMapper()
        entity1 = IMP.mmcif.data._Entity("ABC")
        entity2 = IMP.mmcif.data._Entity("DEF")
        chain1 = MockChain("A")
        chain2 = MockChain("A")
        comp1 = cm.add(chain1, entity1)
        self.assertEqual(cm[chain1], comp1)
        # Cannot add two chains with the same ID but different sequences
        self.assertRaises(ValueError, cm.add, chain2, entity2)

    def test_component_mapper_get_all(self):
        """Test ComponentMapper get_all()"""
        class MockChain(object):
            def __init__(self, name):
                self.name = name
        cm = IMP.mmcif.data._ComponentMapper()
        entity1 = IMP.mmcif.data._Entity("ABC")
        entity2 = IMP.mmcif.data._Entity("DEF")
        chain1 = MockChain("A")
        chain2 = MockChain("B")
        comp1 = cm.add(chain1, entity1)
        comp2 = cm.add(chain2, entity2)
        allc = cm.get_all()
        self.assertEqual(allc, [comp1, comp2])
        self.assertEqual(cm.get_all_modeled(), [])

    def test_representation_same_rigid_body(self):
        """Test Representation._same_rigid_body()"""
        m = IMP.Model()
        xyz1 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                           IMP.algebra.Vector3D(1,1,1))
        xyz2 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                           IMP.algebra.Vector3D(2,2,2))
        rigid1 = IMP.core.RigidBody.setup_particle(IMP.Particle(m), [xyz1])
        r = IMP.mmcif.data._Representation()
        r.rigid_body = None
        self.assertTrue(r._same_rigid_body(None))
        self.assertFalse(r._same_rigid_body(rigid1))
        r.rigid_body = rigid1
        self.assertFalse(r._same_rigid_body(None))
        self.assertTrue(r._same_rigid_body(rigid1))


if __name__ == '__main__':
    IMP.test.main()
