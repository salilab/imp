import utils
import os
import unittest
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm
import ihm.location

class Tests(unittest.TestCase):
    def test_system(self):
        """Test System class"""
        s = ihm.System('test system')
        self.assertEqual(s.name, 'test system')

    def test_entity(self):
        """Test Entity class"""
        e1 = ihm.Entity('ABCD', description='foo')
        # Should compare identical if sequences are the same
        e2 = ihm.Entity('ABCD', description='bar')
        e3 = ihm.Entity('ABCDE', description='foo')
        self.assertEqual(e1, e2)
        self.assertNotEqual(e1, e3)

    def test_software(self):
        """Test Software class"""
        s1 = ihm.Software(name='foo', version='1.0',
                          classification='1', description='2', location='3')
        s2 = ihm.Software(name='foo', version='2.0',
                          classification='4', description='5', location='6')
        s3 = ihm.Software(name='foo', version='1.0',
                          classification='7', description='8', location='9')
        s4 = ihm.Software(name='bar', version='1.0',
                          classification='a', description='b', location='c')
        # Should compare equal iff name and version both match
        self.assertEqual(s1, s3)
        self.assertEqual(hash(s1), hash(s3))
        self.assertNotEqual(s1, s2)
        self.assertNotEqual(s1, s4)

    def test_asym_range(self):
        """Test AsymUnitRange class"""
        e = ihm.Entity('ABCDAB')
        a = ihm.AsymUnit(e)
        a._id = 42
        self.assertEqual(a.seq_id_range, (1,6))
        r = a(3,4)
        self.assertEqual(r.seq_id_range, (3,4))
        self.assertEqual(r._id, 42)
        self.assertEqual(r.entity, e)

    def test_assembly_component_entity(self):
        """Test AssemblyComponent created from an entity"""
        e = ihm.Entity('ABCD')
        c = ihm.AssemblyComponent(e)
        self.assertEqual(c.entity, e)
        self.assertEqual(c.asym, None)

    def test_assembly_component_asym(self):
        """Test AssemblyComponent created from an asym unit"""
        e = ihm.Entity('ABCD')
        a = ihm.AsymUnit(e)
        c = ihm.AssemblyComponent(a)
        self.assertEqual(c.entity, e)
        self.assertEqual(c.asym, a)

    def test_assembly_component_seqrange_entity(self):
        """Test AssemblyComponent default seq range from an entity"""
        e = ihm.Entity('ABCD')
        c = ihm.AssemblyComponent(e)
        self.assertEqual(c.seq_id_range, (1, 4))

    def test_assembly_component_seqrange_asym_unit(self):
        """Test AssemblyComponent default seq range from an asym unit"""
        e = ihm.Entity('ABCD')
        a = ihm.AsymUnit(e)
        c = ihm.AssemblyComponent(a)
        self.assertEqual(c.seq_id_range, (1, 4))

    def test_assembly_component_given_seqrange(self):
        """Test AssemblyComponent with a seq range"""
        e = ihm.Entity('ABCD')
        c = ihm.AssemblyComponent(e, (2,3))
        self.assertEqual(c.seq_id_range, (2, 3))

    def test_assembly(self):
        """Test Assembly class"""
        e1 = ihm.Entity('ABCD')
        e2 = ihm.Entity('ABC')
        c = ihm.AssemblyComponent(e1)
        a = ihm.Assembly([c, e2], name='foo', description='bar')
        self.assertEqual(a.name, 'foo')
        self.assertEqual(a.description, 'bar')

    def test_remove_identical(self):
        """Test remove_identical function"""
        x = {}
        y = {}
        all_objs = ihm._remove_identical([x, x, y])
        # Order should be preserved, but only one x should be returned
        self.assertEqual(list(all_objs), [x, y])

    def test_all_model_groups(self):
        """Test _all_model_groups() method"""
        model_group1 = []
        model_group2 = []
        state1 = [model_group1, model_group2]
        state2 = [model_group2, model_group2]
        s = ihm.System()
        s.state_groups.append([state1, state2])
        mg = s._all_model_groups()
        # List may contain duplicates
        self.assertEqual(list(mg), [model_group1, model_group2,
                                    model_group2, model_group2])

    def test_all_models(self):
        """Test _all_models() method"""
        class MockModel(object):
            pass
        model1 = MockModel()
        model2 = MockModel()
        model_group1 = [model1, model2]
        model_group2 = [model1, model1]
        s = ihm.System()
        s.state_groups.append([[model_group1, model_group2]])
        ms = s._all_models()
        models = [model for group, model in ms]
        # duplicates should be filtered within groups, but not between groups
        self.assertEqual(models, [model1, model2, model1])

    def test_all_protocols(self):
        """Test _all_protocols() method"""
        class MockObject(object):
            pass
        model1 = MockObject()
        model2 = MockObject()
        model3 = MockObject()
        model_group1 = [model1, model2, model3]
        s = ihm.System()
        s.state_groups.append([[model_group1]])
        p1 = MockObject()
        p2 = MockObject()
        s.orphan_protocols.append(p1)
        model1.protocol = None
        model2.protocol = p2
        model3.protocol = p1
        # duplicates should be filtered globally
        self.assertEqual(list(s._all_protocols()), [p1, p2])

    def test_all_dataset_groups(self):
        """Test _all_dataset_groups() method"""
        class MockObject(object):
            pass
        dg1 = MockObject()
        dg2 = MockObject()
        s = ihm.System()
        s.orphan_dataset_groups.append(dg1)
        step1 = MockObject()
        step2 = MockObject()
        step3 = MockObject()
        step1.dataset_group = None
        step2.dataset_group = dg2
        step3.dataset_group = dg1
        protocol1 = MockObject()
        protocol1.steps = [step1, step2, step3]
        s.orphan_protocols.append(protocol1)
        # duplicates should not be filtered
        self.assertEqual(list(s._all_dataset_groups()), [dg1, dg2, dg1])

    def test_all_locations(self):
        """Test _all_locations() method"""
        class MockObject(object):
            pass
        loc1 = MockObject()
        loc2 = MockObject()
        loc3 = MockObject()

        s = ihm.System()
        dataset1 = MockObject()
        dataset2 = MockObject()
        dataset2.location = None
        dataset3 = MockObject()
        dataset3.location = loc1

        s.locations.append(loc1)
        s.datasets.extend((dataset1, dataset2, dataset3))

        ensemble = MockObject()
        ensemble.file = loc2
        density = MockObject()
        density.file = loc1
        ensemble.densities = [density]
        s.ensembles.append(ensemble)

        start_model = MockObject()
        template = MockObject()
        template.alignment_file = loc3
        start_model.templates = [template]
        s.starting_models.append(start_model)

        # duplicates should not be filtered
        self.assertEqual(list(s._all_locations()), [loc1, loc1, loc2,
                                                    loc1, loc3])

    def test_update_locations_in_repositories(self):
        """Test update_locations_in_repositories() method"""
        s = ihm.System()
        loc = ihm.location.InputFileLocation(path='foo', repo='bar')
        s.locations.append(loc)
        r = ihm.location.Repository(doi='foo', root='..')
        s.update_locations_in_repositories([r])


if __name__ == '__main__':
    unittest.main()
