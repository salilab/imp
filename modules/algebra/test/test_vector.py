import IMP
import IMP.test
import IMP.algebra
import math
import io

class Tests(IMP.test.TestCase):

    def test_vector_nd(self):
        """Test VectorD<N> operations for unusual N"""
        for N in (-1,1,2,4,5,6):
            if N == -1:
                clsdim = 'K'
                dim = 5
            else:
                clsdim = '%d' % N
                dim = N
            V = getattr(IMP.algebra, "Vector%sD" % clsdim)
            init = [0] * dim
            v = V(init)
            v2 = V(*init)
            self.assertLess(IMP.algebra.get_distance(v, v2), 1e-4)
            self.assertLess(IMP.algebra.get_squared_distance(v, v), 1e-4)
            if N == -1:
                us = IMP.algebra.get_ones_vector_kd(5)
                us = IMP.algebra.get_basis_vector_kd(5, 0)
            else:
                us = getattr(IMP.algebra, "get_ones_vector_%sd" % clsdim)()
                us = getattr(IMP.algebra, "get_basis_vector_%sd" % clsdim)(0)
            v += V([1] * dim)
            v /= 2.
            v -= V([2] * dim)
            v2 = v + V([0] * dim)
            v2 = v - V([0] * dim)
            v2 = v * v
            sio = io.BytesIO()
            v.show(sio)

if __name__ == '__main__':
    IMP.test.main()
