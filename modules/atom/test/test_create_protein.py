import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.display
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    """Test molecular dynamics optimizer"""
    def broken_until_swig_hierarchies_gets_fixed_test_cp(self):
        """Testing create_protein"""
        m= IMP.Model()
        rp= IMP.Particle(m)
        r= IMP.atom.create_protein(rp, "hi", 10.0, 150)
        print "back"
        r.show()
        m.add_restraint(r)
        p= IMP.atom.Hierarchy(rp)
        print "printing"
        print p.get_number_of_children()
        print p.get_children()
        print p.get_children().size()
        for c in p.get_children():
            d= IMP.core.XYZ(c.get_particle())
            d.set_coordinates(get_random_vector_in(Vector3D(0,0,0),
                                                   Vector3D(300, 300, 300)))
        o= IMP.core.SteepestDescent()
        o.set_model(m)
        score=o.optimize(1000)
        print score
        w= IMP.display.ChimeraWriter(self.get_tmp_file_name("proteinconf.py"))
        for c in p.get_children():
            d= IMP.core.XYZR(c.get_particle())
            w.add_geometry(IMP.core.XYZRGeometry(d))
        self.assertLess(score, 1)

if __name__ == '__main__':
    IMP.test.main()
