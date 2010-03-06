import unittest
import IMP.algebra
import IMP.test

class PluralTests(IMP.test.TestCase):

    def assertPlural(self, module, objs):
        not_found = []
        for f in dir(module):
            if f[0].upper()== f[0] and f[1].upper() != f[1]\
                    and  "_" not in f and not f.endswith("_swigregister")\
                    and f not in objs and not f.endswith("s")\
                    and not f.endswith("Temp") and not f.endswith("Iterator")\
                    and not f.endswith("Exception"):
                if f+"s" not in dir(module):
                    not_found.append(f)
        self.assert_(len(not_found) == 0, str(not_found))

    def assertShow(self, module, objs):
        not_found = []
        for f in dir(module):
            if f[0].upper()== f[0] and f[1].upper() != f[1]\
                    and  "_" not in f and not f.endswith("_swigregister")\
                    and f not in objs and not f.endswith("s")\
                    and not f.endswith("Temp") and not f.endswith("Iterator")\
                    and not f.endswith("Exception"):
                if not hasattr(getattr(module, f), 'show'):
                    not_found.append(f)
        self.assert_(len(not_found) == 0, str(not_found))


    def test_plural(self):
        """Test all algebra objects have plural version"""
        return self.assertPlural(IMP.algebra, ['FixedXYZ', 'FixedZYZ', 'LinearFit', 'Matrix2D', 'Matrix3D', 'MultiArray3D', 'NearestNeighbor2D', 'NearestNeighbor3D', 'NearestNeighbor4D', 'ParabolicFit', 'BoundingBox2D', 'BoundingBox3D', 'BoundingBox4D'])

    def test_show(self):
        """Test all algebra objects have show"""
        return self.assertShow(IMP.algebra, ['AxisAnglePair', 'Matrix3D', 'MultiArray3D', 'NearestNeighbor2D', 'NearestNeighbor3D', 'NearestNeighbor4D'])

if __name__ == '__main__':
    unittest.main()
