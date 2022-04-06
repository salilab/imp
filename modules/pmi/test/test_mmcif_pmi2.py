from __future__ import print_function

import ihm.format
import IMP.test
import IMP.pmi.topology
import IMP.pmi.mmcif
import sys

if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

class MockMsgPack(object):
    @staticmethod
    def pack(data, fh, use_bin_type=True):
        fh.data = data


class MockFh(object):
    pass


class Tests(IMP.test.TestCase):

    def assign_entity_asym_ids(self, system):
        """Assign IDs to all Entities and AsymUnits in the system"""
        d = ihm.dumper._EntityDumper()
        d.finalize(system)
        d = ihm.dumper._StructAsymDumper()
        d.finalize(system)

    def assign_range_ids(self, system):
        """Assign IDs to all Entity/AsymUnit segments in the system"""
        d = ihm.dumper._EntityPolySegmentDumper()
        d.finalize(system)

    def test_component_mapper(self):
        """Test ComponentMapper with PMI2 topology"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        # Should be OK with a multi-character chain ID
        nup84 = state.create_molecule("Nup84", "MELS", "AA")
        nup84.add_representation(resolutions=[1])
        hier = s.build()
        c = IMP.pmi.mmcif._ComponentMapper(hier)
        r = IMP.atom.get_by_type(hier, IMP.atom.RESIDUE_TYPE)[1]
        self.assertEqual(c[r], 'Nup84.0')

    def test_hier_system_mapping(self):
        """Test mapping from Hierarchy back to System"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_representation(resolutions=[1])
        hier = s.build()

        # Check mapping from top-level Hierarchy back to System
        self.assertEqual(IMP.pmi.tools._get_system_for_hier(hier), s)
        # Invalid particle
        self.assertEqual(IMP.pmi.tools._get_system_for_hier(None), None)
        # Particle not set up by System
        p = IMP.Particle(m)
        self.assertEqual(IMP.pmi.tools._get_system_for_hier(p), None)
        h = IMP.atom.Hierarchy.setup_particle(p)
        self.assertEqual(IMP.pmi.tools._get_system_for_hier(h), None)
        # Child particles should be OK
        child = hier.get_child(0).get_child(0).get_child(0).get_child(0)
        self.assertEqual(IMP.pmi.tools._get_system_for_hier(child), s)
        child = child.get_child(3)
        self.assertEqual(IMP.pmi.tools._get_system_for_hier(child), s)

        # Check mapping from Hierarchy to ProtocolOutput
        pos = list(IMP.pmi.tools._all_protocol_outputs(hier))
        # Should be a list of (ProtocolOuput, State) tuples
        self.assertEqual(len(pos), 1)
        self.assertEqual(len(pos[0]), 2)
        self.assertEqual(pos[0][0], po)

    def test_finalize_write_mmcif(self):
        """Test ProtocolOutput.finalize() and mmCIF output"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        po.finalize()
        fh = StringIO()
        ihm.dumper.write(fh, [po.system])
        val = fh.getvalue()
        # Work with both latest stable ihm and that bundled with IMP
        if '_struct.pdbx_model_details' in val:
            self.assertEqual(
                val.split('\n')[:5],
                ['data_model', '_entry.id model', '_struct.entry_id model',
                 '_struct.pdbx_model_details .',
                 '_struct.pdbx_structure_determination_methodology '
                 'integrative'])
        else:
            self.assertEqual(
                val.split('\n')[:5],
                ['data_model', '_entry.id model', '_struct.entry_id model',
                 '_struct.pdbx_structure_determination_methodology '
                 'integrative',
                 '_struct.title .'])

    def test_finalize_write_bcif(self):
        """Test ProtocolOutput.finalize() and BinaryCIF output"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        sys.modules['msgpack'] = MockMsgPack
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        po.finalize()
        fh = MockFh()
        ihm.dumper.write(fh, [po.system], format='BCIF')
        self.assertEqual(fh.data['dataBlocks'][0]['categories'][0]['name'],
                         '_entry')

    def test_entity(self):
        """Test EntityDump with PMI2-style init"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_representation(resolutions=[1])
        hier = s.build()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        d = ihm.dumper._EntityDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_entity.id
_entity.type
_entity.src_method
_entity.pdbx_description
_entity.formula_weight
_entity.pdbx_number_of_molecules
_entity.details
1 polymer man Nup84 532.606 1 .
#
""")

    def test_model_representation(self):
        """Test ModelRepresentationDumper with PMI2-style init"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = IMP.pmi.mmcif.ProtocolOutput()
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(resolutions=[1])
        hier = s.build()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        self.assign_range_ids(po.system)
        # Assign starting model IDs
        d = ihm.dumper._StartingModelDumper()
        d.finalize(po.system)
        d = ihm.dumper._ModelRepresentationDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        r, = po.system.orphan_representations
        self.assertEqual([f.asym_unit.seq_id_range for f in r], [(1,2), (3,4)])
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_representation.id
_ihm_model_representation.name
_ihm_model_representation.details
1 'Default representation' .
#
#
loop_
_ihm_model_representation_details.id
_ihm_model_representation_details.representation_id
_ihm_model_representation_details.entity_id
_ihm_model_representation_details.entity_description
_ihm_model_representation_details.entity_asym_id
_ihm_model_representation_details.entity_poly_segment_id
_ihm_model_representation_details.model_object_primitive
_ihm_model_representation_details.starting_model_id
_ihm_model_representation_details.model_mode
_ihm_model_representation_details.model_granularity
_ihm_model_representation_details.model_object_count
_ihm_model_representation_details.description
1 1 1 Nup84 A 1 sphere 1 flexible by-residue . .
2 1 1 Nup84 A 2 sphere . flexible by-feature 2 .
#
""")

if __name__ == '__main__':
    IMP.test.main()
