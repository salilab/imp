from __future__ import print_function
import IMP.test
import IMP.pmi.output
import IMP.pmi.representation

class Tests(IMP.test.TestCase):

    def test_prot_add(self):
        """Test Representation.add_protocol_output()"""
        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        po = IMP.pmi.output.ProtocolOutput()
        r.add_protocol_output(po)

if __name__ == '__main__':
    IMP.test.main()
