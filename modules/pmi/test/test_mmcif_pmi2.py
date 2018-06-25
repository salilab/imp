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

if __name__ == '__main__':
    IMP.test.main()
