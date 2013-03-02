import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
from load_anchor_graph import *

class Tests(IMP.test.TestCase):
    """Class to test anchor graph functionalities"""

    def load_data(self):
        self.anchor_graph=load_anchor_graph(self.open_input_file("1z5s_4_anchors.cmm"))
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        self.imp_model = IMP.Model()
        self.load_data()
    def test_anchor_graph_formation(self):
        self.assertEqual(4,self.anchor_graph.get_number_of_anchors())
        self.assertEqual(4,self.anchor_graph.get_number_of_edges())
    def test_probability_calculation(self):
        p=IMP.Particle(self.imp_model)
        IMP.core.XYZ.setup_particle(p,IMP.algebra.Vector3D(0,0,0))
        sols=IMP.multifit.read_fitting_solutions(
            self.get_input_file_name("1z5s_A_fitting_solutions.txt"))
        print sols[0].get_fit_transformation()
        self.anchor_graph.set_particle_probabilities_on_anchors(p,sols)
        probs=self.anchor_graph.get_particle_probabilities(p)
        print probs
        self.assertAlmostEqual(probs[0],0,1)
        self.assertAlmostEqual(probs[2],0.92,1)
        self.anchor_graph.show()

if __name__ == '__main__':
    IMP.test.main()
