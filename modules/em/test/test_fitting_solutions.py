from __future__ import print_function
import IMP
import IMP.em
import IMP.test
import pickle


class Tests(IMP.test.TestCase):

    def test_pickle(self):
        """Test (un-)pickle of FittingSolutions"""
        p = IMP.em.FittingSolutions()
        t = IMP.algebra.Transformation3D(
                IMP.algebra.get_random_rotation_3d(),
                IMP.algebra.get_random_vector_in(
                    IMP.algebra.get_unit_bounding_box_3d()))
        p.add_solution(t, 42.0)
        dump = pickle.dumps(p)
        newp = pickle.loads(dump)
        self.assertEqual(newp.get_number_of_solutions(), 1)
        self.assertAlmostEqual(newp.get_score(0), 42.0, delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
