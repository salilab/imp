import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
import sys
sys.path.append("../pyext")
from load_anchor_graph import *

class AnchorGraphTest(IMP.test.TestCase):
    """Class to test anchor graph functionalities"""

    def load_data(self):
        self.anchor_graph=load_anchor_graph(self.open_input_file("1z5s_4_anchors.cmm"))
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
        self.load_data()
    def test_anchor_graph_formation(self):
        self.assertEqual(4,self.anchor_graph.get_number_of_anchors())
        self.assertEqual(4,self.anchor_graph.get_number_of_edges())

if __name__ == '__main__':
    IMP.test.main()
