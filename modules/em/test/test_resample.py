import IMP.test
import IMP.em
import os
import random

class Tests(IMP.test.TestCase):

    def _get_min_max(self, m):
        min= m.get_value(0)
        max= m.get_value(0)
        for i in range(1, m.get_number_of_voxels()):
            v= m.get_value(i)
            if v< min:
                min=v
            if v > max:
                max=v
        return (min, max)
    def _test_min_max(self, ma, mb):
        (mina, maxa) = self._get_min_max(ma)
        (minb, maxb) = self._get_min_max(mb)
        self.assertAlmostEqual(mina, minb, delta=.1*(mina+minb)+.1)
        self.assertAlmostEqual(maxa, maxb, delta=.1*(maxa+maxb)+.1)
    def test_imag2e(self):
        """Check resampling of maps"""
        m= IMP.em.read_map(self.get_input_file_name('1z5s.mrc'))
        ret= IMP.em.get_resampled(m, 2)
        mrw= IMP.em.MRCReaderWriter()
        IMP.em.write_map(ret, self.get_tmp_file_name("down_scaled.mrc"), mrw)
        self._test_min_max(m, ret)
        ret= IMP.em.get_resampled(m, .5)
        IMP.em.write_map(ret, self.get_tmp_file_name("up_scaled.mrc"), mrw)
        self._test_min_max(m, ret)

if __name__ == '__main__':
    IMP.test.main()
