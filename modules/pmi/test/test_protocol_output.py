from __future__ import print_function
import IMP.test
import IMP.pmi.output
import IMP.pmi.representation

class TestPO(IMP.pmi.output.ProtocolOutput):
    _file_datasets = []
    _each_metadata = []
    def _add_state(self, obj):
        return self

class Tests(IMP.test.TestCase):

    def test_prot_add(self):
        """Test Representation.add_protocol_output()"""
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        po = TestPO()
        s.add_protocol_output(po)

if __name__ == '__main__':
    IMP.test.main()
