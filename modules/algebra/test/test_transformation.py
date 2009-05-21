import unittest
import IMP
import IMP.test
import IMP.algebra
import math

class ConeTests(IMP.test.TestCase):

    def test_cone_construction(self):
        """Check difference between two transformations"""
        a=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                       IMP.algebra.random_vector_in_unit_box())

        b=IMP.algebra.Transformation3D(IMP.algebra.random_rotation(),
                                       IMP.algebra.random_vector_in_unit_box())

        d= a/b
        x= IMP.algebra.random_vector_in_unit_box();
        ax= a.transform(x)
        bx= b.transform(x)
        dbx= d.transform(bx)
        x.show()
        dbx.show()
        ax.show()
        self.assert_((ax-dbx).get_squared_magnitude() < .1)

if __name__ == '__main__':
    unittest.main()
