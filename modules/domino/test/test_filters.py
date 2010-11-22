import sys
import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom
import IMP.algebra
import time
import random

def cross(args):
    ans = [[]]
    for arg in args:
        ans = [x+[y] for x in ans for y in arg]
    return ans

class DOMINOTests(IMP.test.TestCase):
    def _get_filtered(self, f, n):
        states= [[0,1,2,3,4,5] for x in range(n)]
        all= cross(states)
        ret=[]
        for s in all:
            if f.get_is_ok(s):
                ret.append(s)
        return ret
    def test_global_min2(self):
        """Test exclusion filtering pairs"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ss= IMP.domino.Subset(ps)
        print ps, ss
        ft= IMP.domino.ExclusionSubsetFilterTable()
        pairs=[(ps[0], ps[1]), (ps[3], ps[4])]
        for pr in pairs:
            ft.add_pair(pr)
        psp=[x for x in ps]
        random.shuffle(psp)
        css= IMP.domino.Subset(psp[0:5])
        print css
        f= ft.get_subset_filter(css, [])
        filtered= self._get_filtered(f, 5)
        for s in filtered:
            for i, p0 in enumerate(css):
                for j, p1 in enumerate(css):
                    if i==j:
                        continue
                    if (p0, p1) in pairs or (p1, p0) in pairs:
                        self.assert_(s[i] != s[j])
    def test_global_min3(self):
        """Test exclusion filtering all"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ss= IMP.domino.Subset(ps)
        print [x.get_name() for x in ps], ss
        ft= IMP.domino.ExclusionSubsetFilterTable()
        ft.add_set(ps)
        psp=[x for x in ps]
        random.shuffle(psp)
        css= IMP.domino.Subset(psp[0:5])
        print "selected", css
        f= ft.get_subset_filter(css, [])
        filtered= self._get_filtered(f, 5)
        print "filtered", filtered
        for s in filtered:
            print s
            for i, p0 in enumerate(css):
                for j, p1 in enumerate(css):
                    if i==j:
                        continue
                    self.assert_(s[i] != s[j])
    def test_global_min4(self):
        """Test equivalence filtering"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ss= IMP.domino.Subset(ps)
        print [x.get_name() for x in ps], ss
        ft= IMP.domino.EquivalenceSubsetFilterTable()
        ft.add_set(ps)
        psp=[x for x in ps]
        random.shuffle(psp)
        css= IMP.domino.Subset(psp[0:5])
        print "selected", css
        f= ft.get_subset_filter(css, [])
        filtered= self._get_filtered(f, 5)
        seen=[]
        for s in filtered:
            cur= [x for x in s]
            cur.sort()
            print s, cur
            self.assert_(cur not in seen)
            seen.append(cur)
if __name__ == '__main__':
    IMP.test.main()
