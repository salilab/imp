import StringIO
import IMP
import IMP.test
import IMP.atom
import IMP.display

class PDBReadWriteTest(IMP.test.TestCase):
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
            self.assert_(dist < 1.1*sph.get_radius())
    def _display(self, mp, name, c):
        g= IMP.display.XYZRGeometry(c)
        gp= IMP.display.HierarchyGeometry(mp)
        w= IMP.display.PymolWriter(self.get_tmp_file_name(name))
        w.add_geometry(g)
        w.add_geometry(gp)
    def test_nonrigid(self):
        """Check cover from selection"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        s= IMP.atom.Selection(mp, residue_indexes=[26, 123])
        d= IMP.atom.create_cover(s, "my cover")
        m.update()
        self._display(mp, "before", d)
        self._perturb(mp)
        m.update()
        self._display(mp, "after", d)
        self._check(mp, s, d)
    def test_rigid(self):
        """Check cover from rigid selection"""
        m = IMP.Model()

        #! read PDB
        mp= IMP.atom.read_pdb(self.open_input_file("input.pdb"),
                              m, IMP.atom.NonWaterPDBSelector())
        rb= IMP.atom.create_rigid_body(mp)
        s= IMP.atom.Selection(mp, residue_indexes=[26, 123])
        d= IMP.atom.create_cover(s, "my cover")
        m.update()
        self._display(mp, "before_rigid", d)
        self._perturb(mp, rb)
        m.update()
        self._display(mp, "after_rigid", d)
        self._check(mp, s, d)

if __name__ == '__main__':
    IMP.test.main()
