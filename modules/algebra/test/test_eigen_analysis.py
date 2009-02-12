import unittest
import IMP
import IMP.utils
import IMP.test
import IMP.algebra

class EigenAnalysisTests(IMP.test.TestCase):
    """Test eigen analysis"""

    def _produce_point_set(self):
        vs= []
        vs.append(IMP.algebra.Vector3D(2.5,2.4,0.0))
        vs.append(IMP.algebra.Vector3D(0.5,0.7,0.0))
        vs.append(IMP.algebra.Vector3D(2.2,2.9,0.0))
        vs.append(IMP.algebra.Vector3D(1.9,2.2,0.0))
        vs.append(IMP.algebra.Vector3D(3.1,3.0,0.0))
        vs.append(IMP.algebra.Vector3D(2.3,2.7,0.0))
        vs.append(IMP.algebra.Vector3D(2.0,1.6,0.0))
        vs.append(IMP.algebra.Vector3D(1.0,1.1,0.0))
        vs.append(IMP.algebra.Vector3D(1.5,1.6,0.0))
        vs.append(IMP.algebra.Vector3D(1.1,0.9,0.0))

        return vs

    def test_pca(self):
        """Testing eigen analysis """
        vs = self._produce_point_set()
        ed = IMP.algebra.principle_components(vs)
        self.assertInTolerance(ed.eigen_values_[0],1.28,.1)
        self.assertInTolerance(ed.eigen_values_[1],0.049,.1)
        ed.show()
if __name__ == '__main__':
    unittest.main()
