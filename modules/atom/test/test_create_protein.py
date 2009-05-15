import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.display
from IMP.algebra import *

class MolecularDynamicsTests(IMP.test.TestCase):
    """Test molecular dynamics optimizer"""
    def test_cp(self):
        """Testing create_protein"""
        m= IMP.Model()
        rp= IMP.Particle(m)
        r= IMP.atom.create_protein(rp, 10.0, 150)
        print "back"
        r.show()
        m.add_restraint(r)
        p= IMP.atom.MolecularHierarchyDecorator(rp)
        print p.get_number_of_children()
        for c in p.get_child_particles():
            d= IMP.core.XYZDecorator(c)
            d.set_coordinates(random_vector_in_box(Vector3D(0,0,0),
                                                   Vector3D(300, 300, 300)))
        o= IMP.core.SteepestDescent()
        o.set_model(m)
        score=o.optimize(1000)
        print score
        w= IMP.display.ChimeraWriter(self.get_tmp_file_name("proteinconf.py"))
        for c in p.get_child_particles():
            d= IMP.core.XYZRDecorator(c)
            w.add_geometry(IMP.display.XYZRGeometry(d))
        self.assert_(score < 1)

if __name__ == '__main__':
    unittest.main()
