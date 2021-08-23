from __future__ import print_function
import IMP.test
import io


class Tests(IMP.test.TestCase):

    """Test particles"""

    def _equal_lists(self, va, vb):
        print("testing")
        self.assertEqual(len(va), len(vb))
        for i in range(len(va)):
            print(va[i])
            print(vb[i])
            self.assertEqual(va[i], vb[i])

    def _almost_equal_lists(self, va, vb):
        print("testing")
        self.assertEqual(len(va), len(vb))
        for i in range(len(va)):
            print(va[i])
            print(vb[i])
            self.assertAlmostEqual(va[i], vb[i], delta=.01)

    def test_a(self):
        """Check float typemaps"""
        vs = [1.1, 2.1, 3]
        vso = IMP._pass_floats(vs)
        self._almost_equal_lists(vs, vso)

    def test_b(self):
        """Check int typemaps"""
        vs = [1, 2, 3]
        vso = IMP._pass_ints(vs)
        self._equal_lists(vs, vso)

    def test_bl(self):
        """Check ints list typemaps"""
        vs = [[1, 2, 3, 4], [5, 6, 7, 8]]
        vso = IMP._pass_ints_list(vs)
        self._equal_lists(vs, [list(x) for x in vso])

    def test_bls(self):
        """Check ints lists typemaps"""
        vs = [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
        vso = IMP._pass_ints_lists(vs)
        print("bw",vso)
        self._equal_lists(vs, [[list(x) for x in y] for y in vso])

    def test_intpairs(self):
        """Check int pairs typemap"""
        ips = [(1, 2), (3, 4)]
        self.assertEqual(IMP._test_intranges(ips), 2)

    def test_c(self):
        """Check string typemaps"""
        vs = ["1.0", "2.0", "3"]
        vso = IMP._pass_strings(vs)
        self._equal_lists(vs, vso)

    def test_pair(self):
        """Checking that conversion to/from pairs is OK"""
        p = (1.0, 2.0)
        op = IMP._pass_pair(p)
        print("first")
        print(p)
        print(op)
        self.assertAlmostEqual(p[0], op[0], delta=.01)
        self.assertAlmostEqual(p[1], op[1], delta=.01)
        op = IMP._pass_plain_pair(p)
        print("second")
        print(p)
        print(op)
        self.assertAlmostEqual(p[0], op[0], delta=.01)
        self.assertAlmostEqual(p[1], op[1], delta=.01)

    def test_pair_overload(self):
        """Checking that conversion from IntRange is OK with overload"""
        ir = (1, 5)
        r = IMP._test_intrange(ir)
        self.assertEqual(r, ir)
        rr = IMP._test_intrange()
        self.assertEqual(rr, (-1, -1))

    def test_overloaded_strings(self):
        """Checking that overloaded functions with strings work"""
        IMP._pass_overloaded_strings(["a", "b"], 1)
        IMP._pass_overloaded_strings(["a", "b"])

    def test_const_ref_primitive(self):
        """Checking that return of primitive types by const& works"""
        t = IMP._TestValue(1)
        self.assertEqual(t.get(), 1)
        self.assertAlmostEqual(t.get_float(), 2.0, delta=1e-6)
        self.assertAlmostEqual(t.get_double(), 2.0, delta=1e-6)
        self.assertAlmostEqual(t.get_Float(), 2.0, delta=1e-6)
        self.assertEqual(t.get_int(), 42)
        self.assertEqual(t.get_Int(), 42)
        self.assertEqual(t.get_string(), "foobar")
        self.assertEqual(t.get_String(), "foobar")

if __name__ == '__main__':
    IMP.test.main()
