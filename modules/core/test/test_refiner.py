import IMP
import IMP.test
import IMP.core
import IMP.atom

rk = IMP.FloatKey("radius")

class Test(IMP.test.TestCase):
    """Tests for bond refiner"""


    def test_rops(self):
        """Checking refiner"""
        IMP.set_log_level(IMP.VERBOSE)
        m= IMP.Model()
        p0= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"), m)
        rb0= IMP.atom.create_rigid_body(p0)
        refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
        print "size:",refiner.get_refined(rb0)
        self.assertGreater(refiner.get_refined(rb0),5)


if __name__ == '__main__':
    IMP.test.main()
