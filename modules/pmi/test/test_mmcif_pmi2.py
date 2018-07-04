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

class DummyPO(IMP.pmi.mmcif.ProtocolOutput):
    def flush(self):
        pass

class Tests(IMP.test.TestCase):

    def assign_entity_asym_ids(self, system):
        """Assign IDs to all Entities and AsymUnits in the system"""
        d = ihm.dumper._EntityDumper()
        d.finalize(system)
        d = ihm.dumper._StructAsymDumper()
        d.finalize(system)

    def test_entity(self):
        """Test EntityDump with PMI2-style init"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = DummyPO(None)
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
1 polymer man Nup84 ? 1 .
#
""")

    def test_model_representation(self):
        """Test ModelRepresentationDumper with PMI2-style init"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = DummyPO(None)
        s.add_protocol_output(po)
        state = s.create_state()
        nup84 = state.create_molecule("Nup84", "MELS", "A")
        nup84.add_structure(self.get_input_file_name('test.nup84.pdb'), 'A')
        nup84.add_representation(resolutions=[1])
        hier = s.build()
        fh = StringIO()
        w = ihm.format.CifWriter(fh)
        self.assign_entity_asym_ids(po.system)
        # Assign starting model IDs
        d = ihm.dumper._StartingModelDumper()
        d.finalize(po.system)
        d = ihm.dumper._ModelRepresentationDumper()
        d.finalize(po.system)
        d.dump(po.system, w)
        out = fh.getvalue()
        self.assertEqual(out, """#
loop_
_ihm_model_representation.ordinal_id
_ihm_model_representation.representation_id
_ihm_model_representation.segment_id
_ihm_model_representation.entity_id
_ihm_model_representation.entity_description
_ihm_model_representation.entity_asym_id
_ihm_model_representation.seq_id_begin
_ihm_model_representation.seq_id_end
_ihm_model_representation.model_object_primitive
_ihm_model_representation.starting_model_id
_ihm_model_representation.model_mode
_ihm_model_representation.model_granularity
_ihm_model_representation.model_object_count
1 1 1 1 Nup84 A 1 2 sphere 1 flexible by-residue .
2 1 2 1 Nup84 A 3 4 sphere . flexible by-feature 2
#
""")

if __name__ == '__main__':
    IMP.test.main()
