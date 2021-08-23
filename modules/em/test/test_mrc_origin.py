from __future__ import print_function
import IMP
import IMP.test
import IMP.em


class Tests(IMP.test.TestCase):
    def test_origin_nxstart(self):
        """If origin is zero, check it gets set from nxstart"""
        d = IMP.em.read_map(self.get_input_file_name("emd-20362.mrc"))
        self.assertLess(
            IMP.algebra.get_distance(d.get_origin(),
                                     (-14.0646, -52.3906, -35.0926)),
            0.1)

        d = IMP.em.read_map(self.get_input_file_name("emd_8564.map"))
        self.assertLess(
            IMP.algebra.get_distance(d.get_origin(),
                                     (-98.4, -98.4, -98.4)),
            0.1)


if __name__ == '__main__':
    IMP.test.main()
