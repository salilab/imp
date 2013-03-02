import unittest
import IMP.rmf
import IMP.test
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def test_navigation(self):
        """Test loading and saving of rigid bodies"""
        for suffix in RMF.suffixes:
            m= IMP.Model()
            r= IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
            r.set_name("rb")
            rbd= IMP.core.RigidBody.setup_particle(r, IMP.algebra.ReferenceFrame3D())
            for i in range(0,3):
                p = IMP.Particle(m)
                v= IMP.algebra.Vector3D(0,0,0)
                v[i]=1
                d=IMP.core.XYZR.setup_particle(p)
                d.set_coordinates(v)
                d.set_radius(.5)
                IMP.atom.Mass.setup_particle(p, .1)
                r.add_child(IMP.atom.Hierarchy.setup_particle(p))
                rbd.add_member(p)
            fn= self.get_tmp_file_name("rigid."+suffix)
            f= RMF.create_rmf_file(fn)
            IMP.rmf.add_hierarchies(f, [r])
            IMP.rmf.save_frame(f, 0)
            frames=[rbd.get_reference_frame()]
            for i in range(0,10):
                bb= IMP.algebra.get_unit_bounding_box_3d()
                tr= IMP.algebra.Transformation3D(IMP.algebra.get_random_rotation_3d(),
                                                 IMP.algebra.get_random_vector_in(bb))
                rf=IMP.algebra.ReferenceFrame3D(tr)
                rbd.set_reference_frame(rf)
                frames.append(rf)
                m.update()
                IMP.rmf.save_frame(f, i+1)
            del f
            f= RMF.open_rmf_file_read_only(fn)
            IMP.rmf.link_hierarchies(f, [r])
            print frames
            for i in range(0,11):
                IMP.rmf.load_frame(f, i)
                print rbd.get_reference_frame()
                for j, c in enumerate(r.get_children()):
                    oc= IMP.core.XYZ(c).get_coordinates()
                    m.update()
                    nc= IMP.core.XYZ(c).get_coordinates()
                    self.assertAlmostEqual((oc-nc).get_magnitude(), 0,
                                           delta=.1)
                    ic= IMP.core.RigidMember(c).get_internal_coordinates()
                    self.assertAlmostEqual(ic[j], 1, delta=.01)

if __name__ == '__main__':
    unittest.main()
