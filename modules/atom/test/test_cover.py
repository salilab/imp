import StringIO
import IMP
import IMP.test
import IMP.atom
import IMP.display

class Tests(IMP.test.TestCase):
    def _perturb(self, mp, rb=None):
        rot = IMP.algebra.get_random_rotation_3d()
        bx= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(100,100,100))
        tr= IMP.algebra.Transformation3D(rot, IMP.algebra.get_random_vector_in(bx))
        if not rb:
            IMP.atom.transform(mp, tr)
        else:
            rb.set_reference_frame(IMP.algebra.ReferenceFrame3D(tr))
    def _check(self, mp, s, c):
        sph= IMP.core.XYZR(c).get_sphere()
        for p in s.get_selected_particles():
            d= IMP.core.XYZ(p)
            dist= IMP.algebra.get_distance(d.get_coordinates(),
                                           sph.get_center())
            self.assertLess(dist, 1.1*sph.get_radius())
        self.assertLess(c.get_radius(), 8)
    def _display(self, mp, name, c, s):
        g= IMP.core.XYZRGeometry(c)
        gp= IMP.atom.HierarchyGeometry(mp)
        gs=IMP.atom.SelectionGeometry(s)
        gs.set_color(IMP.display.Color(1,0,0))
        w= IMP.display.PymolWriter(self.get_tmp_file_name(name)+".pym")
        w.add_geometry(g)
        w.add_geometry(gp)
        w.add_geometry(gs)
    def test_nonrigid(self):
        """Check cover from selection"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.CAlphaPDBSelector())
        s= IMP.atom.Selection(mp, residue_indexes=[26, 30])
        d= IMP.atom.create_cover(s, "my cover")
        m.update()
        print d.get_radius()
        self._display(mp, "before", d, s)
        self._perturb(mp)
        m.update()
        self._display(mp, "after", d, s)
        self._check(mp, s, d)
    def test_rigid(self):
        """Check cover from rigid selection"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.CAlphaPDBSelector())
        rb= IMP.atom.create_rigid_body(mp)
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        #IMP.atom.show_molecular_hierarchy(mp)
        s= IMP.atom.Selection(mp, residue_indexes=[26, 30])
        print "selected:"
        for p in s.get_selected_particles():
            print p.get_name()
        print "onward"
        d= IMP.atom.create_cover(s, "my cover")
        m.update()
        print d.get_radius()
        self._display(mp, "before_rigid", d, s)
        self._perturb(mp, rb)
        m.update()
        self._display(mp, "after_rigid", d, s)
        self._check(mp, s, d)

if __name__ == '__main__':
    IMP.test.main()
