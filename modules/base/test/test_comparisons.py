import IMP.base
import IMP.test
import sys
import random
from StringIO import StringIO

class Tests(IMP.test.TestCase):
    def test_log_targets(self):
        """Test comparisons in python"""
        V= IMP.base._TestValue
        b= V(random.randint(0,100))
        c= V(b.get())
        cp= cmp(b,c)
        icp= cmp(b.get(), c.get())
        lt= b < c
        ilt= (b.get() < c.get())
        eq= b==c
        ieq= (b.get() == c.get())
        print cp, icp
        print lt, ilt
        print eq, ieq
        self.assertEqual(cp, icp)
        self.assertEqual(lt, ilt)
        self.assertEqual(eq, ieq)
        for i in range(0,10):
            c= V(random.randint(0,100))
            cp= cmp(b,c)
            icp= cmp(b.get(), c.get())
            lt= b < c
            ilt= (b.get() < c.get())
            eq= b==c
            ieq= (b.get() == c.get())
            print cp, icp
            print lt, ilt
            print eq, ieq
            self.assertEqual(cp, icp)
            self.assertEqual(lt, ilt)
            self.assertEqual(eq, ieq)
if __name__ == '__main__':
    IMP.test.main()
