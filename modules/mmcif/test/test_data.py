from __future__ import print_function
import IMP.test
import IMP.atom
import IMP.mmcif.data
import ihm
import os

def add_attrs(r):
    IMP.core.XYZR.setup_particle(
        r, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
    IMP.atom.Mass.setup_particle(r, 1.0)


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

    def test_software_add_hierarchy(self):
        """Test AllSoftware.add_hierarchy"""
        s = ihm.System()
        allsoft = IMP.mmcif.data._AllSoftware(s)
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        prov = IMP.core.SoftwareProvenance.setup_particle(
            IMP.Particle(m), "testname", "testver", "testloc")
        IMP.core.add_provenance(m, top, prov)
        prov = IMP.core.SoftwareProvenance.setup_particle(
            IMP.Particle(m), "testname", "testver", "testloc")
        IMP.core.add_provenance(m, top, prov)
        prov = IMP.core.SoftwareProvenance.setup_particle(
            IMP.Particle(m), "testname", "diffver", "testloc")
        IMP.core.add_provenance(m, top, prov)
        prov = IMP.core.SoftwareProvenance.setup_particle(
            IMP.Particle(m), "diffname", "testver", "testloc")
        IMP.core.add_provenance(m, top, prov)
        allsoft.add_hierarchy(top)
        # Duplicate name-version should be removed
        self.assertEqual([(x.name, x.version) for x in s.software],
                         [('diffname', 'testver'), ('testname', 'diffver'),
                          ('testname', 'testver')])

    def test_software_add_hierarchy_citations(self):
        """Test that AllSoftware.add_hierarchy adds citations"""
        s = ihm.System()
        allsoft = IMP.mmcif.data._AllSoftware(s)
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        prov = IMP.core.SoftwareProvenance.setup_particle(
            IMP.Particle(m), "testname", "testver", "testloc")
        IMP.core.add_provenance(m, top, prov)
        prov = IMP.core.SoftwareProvenance.setup_particle(
            IMP.Particle(m), "IMP PMI module", "testver", "testloc")
        IMP.core.add_provenance(m, top, prov)
        allsoft.add_hierarchy(top)
        pmisoft, testsoft = s.software
        self.assertIsNone(testsoft.citation)
        self.assertEqual(pmisoft.citation.pmid, '31396911')

    def test_external_files_add_hierarchy(self):
        """Test _ExternalFiles.add_hierarchy"""
        s = ihm.System()
        extfiles = IMP.mmcif.data._ExternalFiles(s)
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        # Add paths that we know exist (they are not scripts though)
        prov = IMP.core.ScriptProvenance.setup_particle(
            IMP.Particle(m), self.get_input_file_name("test_1.mrc"))
        IMP.core.add_provenance(m, top, prov)
        prov = IMP.core.ScriptProvenance.setup_particle(
            IMP.Particle(m), self.get_input_file_name("test_2.mrc"))
        IMP.core.add_provenance(m, top, prov)
        prov = IMP.core.ScriptProvenance.setup_particle(
            IMP.Particle(m), self.get_input_file_name("test_1.mrc"))
        IMP.core.add_provenance(m, top, prov)
        extfiles.add_hierarchy(top)
        # Duplicate path should be removed
        self.assertEqual([os.path.basename(x.path) for x in s.locations],
                         ['test_1.mrc', 'test_2.mrc'])

    def test_protocols_add_hierarchy(self):
        """Test _Protocols.add_hierarchy"""
        s = ihm.System()
        software = IMP.mmcif.data._AllSoftware(s)
        protocols = IMP.mmcif.data._Protocols(s)
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        prov = IMP.core.SoftwareProvenance.setup_particle(
            IMP.Particle(m), "testname", "testver", "testloc")
        IMP.core.add_provenance(m, top, prov)
        prov = IMP.core.SampleProvenance.setup_particle(
            IMP.Particle(m), "Monte Carlo", 100, 10, 1)
        IMP.core.add_provenance(m, top, prov)
        protocols._add_hierarchy(top, None, None, software)
        protocol, = s.orphan_protocols
        step, = protocol.steps
        self.assertEqual(step.num_models_begin, 0)
        self.assertEqual(step.num_models_end, 100)
        self.assertEqual(step.software.name, "testname")

    def test_protocols_add_protocol(self):
        """Test _Protocols.add_protocol"""
        s = ihm.System()
        protocols = IMP.mmcif.data._Protocols(s)
        p1 = ihm.protocol.Protocol()
        p1.steps.append(ihm.protocol.Step(
            assembly='foo', dataset_group='bar', method='baz',
            num_models_begin=0, num_models_end=42))

        # New protocol should be returned unchanged
        self.assertIs(protocols._add_protocol(p1), p1)

        # Identical protocol should map to same object
        p2 = ihm.protocol.Protocol()
        p2.steps.append(ihm.protocol.Step(
            assembly='foo', dataset_group='bar', method='baz',
            num_models_begin=0, num_models_end=42))
        self.assertIs(protocols._add_protocol(p2), p1)

        # Different step parameters should map to different object
        p3 = ihm.protocol.Protocol()
        p3.steps.append(ihm.protocol.Step(
            assembly='foo', dataset_group='bar', method='baz',
            num_models_begin=0, num_models_end=99))
        self.assertIs(protocols._add_protocol(p3), p3)

        # Longer protocol should map to different object
        p4 = ihm.protocol.Protocol()
        p4.steps.append(ihm.protocol.Step(
            assembly='foo', dataset_group='bar', method='baz',
            num_models_begin=0, num_models_end=42))
        analysis = ihm.analysis.Analysis()
        analysis.steps.append(ihm.analysis.FilterStep(
            feature='RMSD', num_models_begin=42, num_models_end=5))
        p4.analyses.append(analysis)
        self.assertIs(protocols._add_protocol(p4), p4)

        self.assertEqual(len(s.orphan_protocols), 3)

    def test_model_assemblies(self):
        """Test ModelAsssemblies class"""
        system = ihm.System()
        entity1 = ihm.Entity("ANC")
        asym1 = ihm.AsymUnit(entity1)
        asym2 = ihm.AsymUnit(entity1)
        system.entities.append(entity1)
        system.asym_units.extend((asym1, asym2))
        ma = IMP.mmcif.data._ModelAssemblies(system)
        as1 = ma.add([asym1])
        as1a = ma.add([asym1])
        as2 = ma.add([asym2])
        as2a = ma.add([asym2])
        self.assertIs(as1, as1a)
        self.assertIs(as2, as2a)
        self.assertEqual(len(system.orphan_assemblies), 2)

    def test_coordinate_handler_get_structure_particles(self):
        """Test CoordinateHandler.get_structure_particles()"""
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        frag = IMP.atom.Fragment.setup_particle(IMP.Particle(m), [3, 4])
        IMP.core.XYZR.setup_particle(
            frag, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        IMP.atom.Mass.setup_particle(frag, 1.0)
        top.add_child(frag)
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 1)
        IMP.core.XYZR.setup_particle(
            residue, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(5, 6, 7), 8))
        IMP.atom.Mass.setup_particle(residue, 1.0)
        top.add_child(residue)
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 2)
        atom = IMP.atom.Atom.setup_particle(IMP.Particle(m),
                                            IMP.atom.AT_CA)
        IMP.core.XYZR.setup_particle(
            atom, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(9, 10, 11), 12))
        residue.add_child(atom)
        top.add_child(residue)
        ch = IMP.mmcif.data._CoordinateHandler(None, None)
        ps = ch.get_structure_particles(top)
        self.assertEqual(len(ps), 3)
        self.assertIsInstance(ps[0], IMP.atom.Residue)
        self.assertIsInstance(ps[1], IMP.atom.Atom)
        self.assertIsInstance(ps[2], IMP.atom.Fragment)

        # Non-sequential fragments are not supported
        frag = IMP.atom.Fragment.setup_particle(IMP.Particle(m), [5, 6, 7, 9])
        IMP.core.XYZR.setup_particle(
            frag, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        IMP.atom.Mass.setup_particle(frag, 1.0)
        top.add_child(frag)
        self.assertRaises(ValueError, ch.get_structure_particles, top)

    def test_coordinate_handler_add_chain(self):
        """Test CoordinateHandler.add_chain()"""
        s = ihm.System()
        ent = ihm.Entity('ACGT')
        asym = ihm.AsymUnit(ent)
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        # Two flexible residues
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 1)
        add_attrs(residue)
        top.add_child(residue)
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 2)
        add_attrs(residue)
        top.add_child(residue)
        # One rigid residue
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 3)
        add_attrs(residue)
        rigid1 = IMP.core.RigidBody.setup_particle(IMP.Particle(m), [residue])
        top.add_child(residue)
        # One residue with atomic representation
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 4)
        atom = IMP.atom.Atom.setup_particle(IMP.Particle(m),
                                            IMP.atom.AT_CA)
        IMP.core.XYZR.setup_particle(
            atom, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4))
        residue.add_child(atom)
        top.add_child(residue)
        # Two beads each spanning two residues
        frag = IMP.atom.Fragment.setup_particle(IMP.Particle(m), [5, 6])
        add_attrs(frag)
        top.add_child(frag)
        frag = IMP.atom.Fragment.setup_particle(IMP.Particle(m), [7, 8])
        add_attrs(frag)
        top.add_child(frag)
        ch = IMP.mmcif.data._CoordinateHandler(s, None)
        ps = ch.get_structure_particles(top)
        ch.add_chain(ps, asym)
        r1, r2, r3, r4 = ch._representation
        self.assertIsInstance(r1, ihm.representation.ResidueSegment)
        self.assertEqual(r1.asym_unit.seq_id_range, (1, 2))
        self.assertFalse(r1.rigid)
        self.assertIsInstance(r2, ihm.representation.ResidueSegment)
        self.assertEqual(r2.asym_unit.seq_id_range, (3, 3))
        self.assertTrue(r2.rigid)
        self.assertIsInstance(r3, ihm.representation.AtomicSegment)
        self.assertEqual(r3.asym_unit.seq_id_range, (4, 4))
        self.assertFalse(r3.rigid)
        self.assertIsInstance(r4, ihm.representation.FeatureSegment)
        self.assertEqual(r4.asym_unit.seq_id_range, (5, 8))
        self.assertFalse(r4.rigid)
        self.assertEqual(r4.count, 2)

    def test_coordinate_handler_get_residue_sequence(self):
        """Test CoordinateHandler.get_residue_sequence()"""
        s = ihm.System()
        m = IMP.Model()
        top = IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        # No sequence
        ch = IMP.mmcif.data._CoordinateHandler(s, None)
        ps = ch.get_structure_particles(top)
        seq_id_begin, seq = ch.get_residue_sequence(ps)
        self.assertEqual(seq_id_begin, 1)
        self.assertEqual(seq, [])
        # Sequence with gaps
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.ALA, 2)
        atom = IMP.atom.Atom.setup_particle(IMP.Particle(m),
                                            IMP.atom.AT_CA)
        IMP.core.XYZR.setup_particle(
            atom, IMP.algebra.Sphere3D(IMP.algebra.Vector3D(9, 10, 11), 12))
        residue.add_child(atom)
        top.add_child(residue)
        residue = IMP.atom.Residue.setup_particle(IMP.Particle(m),
                                                  IMP.atom.PRO, 4)
        add_attrs(residue)
        top.add_child(residue)
        frag = IMP.atom.Fragment.setup_particle(IMP.Particle(m), [5, 6, 7])
        add_attrs(frag)
        top.add_child(frag)
        ps = ch.get_structure_particles(top)
        seq_id_begin, seq = ch.get_residue_sequence(ps)
        self.assertEqual(len(seq), 6)
        self.assertEqual(seq_id_begin, 2)
        self.assertEqual(seq[0].id, 'ALA')
        self.assertIsNone(seq[1])
        self.assertEqual(seq[2].id, 'PRO')
        self.assertIsNone(seq[3])
        self.assertIsNone(seq[4])
        self.assertIsNone(seq[5])

    def test_representations(self):
        """Test _Representations class"""
        s = ihm.System()
        ent = ihm.Entity('ACGT')
        asym = ihm.AsymUnit(ent)
        reps = IMP.mmcif.data._Representations(s)

        # New representation should be returned unchanged
        rep1 = ihm.representation.Representation(
            [ihm.representation.ResidueSegment(asym, rigid=False,
                                               primitive='sphere')])
        self.assertIs(reps.add(rep1), rep1)

        # Identical representation should map to same object
        rep2 = ihm.representation.Representation(
            [ihm.representation.ResidueSegment(asym, rigid=False,
                                               primitive='sphere')])
        self.assertIs(reps.add(rep2), rep1)

        # Different representation parameters should map to different object
        rep3 = ihm.representation.Representation(
            [ihm.representation.ResidueSegment(asym, rigid=True,
                                               primitive='sphere')])
        self.assertIs(reps.add(rep3), rep3)

        # Different representation type should map to different object
        rep4 = ihm.representation.Representation(
            [ihm.representation.MultiResidueSegment(asym, rigid=False,
                                                    primitive='sphere')])
        self.assertIs(reps.add(rep4), rep4)

        self.assertEqual(len(s.orphan_representations), 3)


if __name__ == '__main__':
    IMP.test.main()
