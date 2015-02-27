from __future__ import print_function
import IMP.base
import IMP.test
import sys
import random
from io import StringIO

if sys.version_info[0] >=3 :
    def cmp(x, y):
        """Return -1 if x < y, 0 if x == y and 1 if x > y"""
        return (x > y) - (x < y)

class Tests(IMP.test.TestCase):

    def test_log_targets(self):
        """Test comparisons in python"""
        V = IMP.base._TestValue
        b = V(random.randint(0, 100))
        c = V(b.get())
        cp = cmp(b, c)
        icp = cmp(b.get(), c.get())
        lt = b < c
        ilt = (b.get() < c.get())
        eq = b == c
        ieq = (b.get() == c.get())
        print(cp, icp)
        print(lt, ilt)
        print(eq, ieq)
        self.assertEqual(cp, icp)
        self.assertEqual(lt, ilt)
        self.assertEqual(eq, ieq)
        for i in range(0, 10):
            c = V(random.randint(0, 100))
            cp = cmp(b, c)
            icp = cmp(b.get(), c.get())
            lt = b < c
            ilt = (b.get() < c.get())
            eq = b == c
            ieq = (b.get() == c.get())
            print(cp, icp)
            print(lt, ilt)
            print(eq, ieq)
            self.assertEqual(cp, icp)
            self.assertEqual(lt, ilt)
            self.assertEqual(eq, ieq)
if __name__ == '__main__':
    IMP.test.main()
