from __future__ import print_function
import IMP.test
import IMP.atom
import IMP.mmcif.data
import ihm

class MockChain(object):
    def __init__(self, name, sequence=''):
        self.name = name
        self.sequence = sequence
    def get_sequence(self):
        return self.sequence

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

    def test_entity_mapper_add(self):
        """Test EntityMapper.add()"""
        system = ihm.System()
        e = IMP.mmcif.data._EntityMapper(system)
        self.assertEqual(len(e.get_all()), 0)
        chain1 = MockChain("A", sequence='ANC')
        chain2 = MockChain("B", sequence='ANC')
        e.add(chain1)
        e.add(chain2)
        # Identical sequences, so only one entity
        self.assertEqual(len(e.get_all()), 1)
        # Different sequences, so two entities
        chain3 = MockChain("C", sequence='ANCD')
        e.add(chain3)
        self.assertEqual(len(e.get_all()), 2)
        # Cannot add chains with no sequence
        chain4 = MockChain("D", sequence='')
        self.assertRaises(ValueError, e.add, chain4)

    def test_entity_naming(self):
        """Test naming of Entities"""
        system = ihm.System()
        cm = IMP.mmcif.data._ComponentMapper(system)
        entity1 = ihm.Entity("ANC")
        chain1 = MockChain("A.1@12")
        chain2 = MockChain("A.2@12")
        comp1 = cm.add(chain1, entity1)
        comp2 = cm.add(chain2, entity1)
        self.assertEqual(chain1.name, "A.1@12")
        self.assertEqual(chain2.name, "A.2@12")
        self.assertEqual(entity1.description, 'A')

    def test_component_mapper_same_id_chain(self):
        """Test ComponentMapper given two chains with same ID"""
        system = ihm.System()
        cm = IMP.mmcif.data._ComponentMapper(system)
        entity1 = ihm.Entity("ANC")
        entity2 = ihm.Entity("DEF")
        chain1 = MockChain("A")
        chain2 = MockChain("A")
        comp1 = cm.add(chain1, entity1)
        self.assertEqual(cm[chain1], comp1)
        # Cannot add two chains with the same ID but different sequences
        self.assertRaises(ValueError, cm.add, chain2, entity2)

    def test_component_mapper_get_all(self):
        """Test ComponentMapper get_all()"""
        system = ihm.System()
        cm = IMP.mmcif.data._ComponentMapper(system)
        entity1 = ihm.Entity("ANC")
        entity2 = ihm.Entity("DEF")
        chain1 = MockChain("A")
        chain2 = MockChain("B")
        comp1 = cm.add(chain1, entity1)
        comp2 = cm.add(chain2, entity2)
        allc = cm.get_all()
        self.assertEqual(allc, [comp1, comp2])
        self.assertEqual(cm.get_all_modeled(), [])

    def test_representation_same_rigid_body(self):
        """Test RepSegmentFactory._same_rigid_body()"""
        m = IMP.Model()
        xyz1 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                           IMP.algebra.Vector3D(1,1,1))
        xyz2 = IMP.core.XYZ.setup_particle(IMP.Particle(m),
                                           IMP.algebra.Vector3D(2,2,2))
        rigid1 = IMP.core.RigidBody.setup_particle(IMP.Particle(m), [xyz1])
        r = IMP.mmcif.data._RepSegmentFactory('mockcomp')
        r.rigid_body = None
        self.assertTrue(r._same_rigid_body(None))
        self.assertFalse(r._same_rigid_body(rigid1))
        r.rigid_body = rigid1
        self.assertFalse(r._same_rigid_body(None))
        self.assertTrue(r._same_rigid_body(rigid1))


if __name__ == '__main__':
    IMP.test.main()
