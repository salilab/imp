import IMP
import IMP.test
import IMP.core
import IMP.display
import IMP.container
import StringIO
import math

class Tests(IMP.test.TestCase):
    def test_score(self):
        """Check that code to display blame runs"""
        m= IMP.Model()
        IMP.base.set_log_level(IMP.base.SILENT)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(100, 100, 100))
        ps= [IMP.core.XYZR.setup_particle(IMP.Particle(m),
                                          IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_in(bb), 10))
             for i in range(0,20)]
        apc= IMP.container.AllPairContainer(ps)
        r= IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), apc)
        g= IMP.core.create_blame_geometries([r], ps)
        w= IMP.display.PymolWriter(self.get_tmp_file_name("blame.pym"))
        w.add_geometry(g)


if __name__ == '__main__':
    IMP.test.main()
