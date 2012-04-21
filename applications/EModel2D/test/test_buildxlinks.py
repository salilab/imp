import IMP
import IMP.test
import IMP.core as core
import IMP.atom as atom
import IMP.em2d as em2d
import sys
import os
import add_parent
add_parent.add_parent_to_python_path()

import buildxlinks as bx
import itertools


class TestBuildXlinks(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.crosslinks = [ ("subunitA", 1, "subunitB", 1, 10),
                       ("subunitA", 2, "subunitB", 2, 20),
                       ("subunitB", 3, "subunitA", 3, 30),
                       ("subunitB", 4, "subunitC", 1, 40),
                       ("subunitB", 5, "subunitC", 2, 20),
                       ("subunitD", 6, "subunitC", 1, 10),
                       ("subunitC", 3, "subunitD", 2, 70),
                       ("subunitC", 4, "subunitD", 3, 80),
                       ("subunitD", 4, "subunitE", 1, 10),
                       ("subunitE", 5, "subunitD", 2, 30),
                       ("subunitD", 6, "subunitE", 3, 40),
                       ("subunitD", 7, "subunitE", 4, 50),
                    ]

        # docking order
        # E into D (4 xlinks)
        # C into D (2 xlinks)
        # C into B (2 xlinks)
        # A into B (remaining 3 xlinks)
        self.optimal_order = [('subunitD', 'subunitE'),
                             ('subunitD', 'subunitC'),
                             ('subunitC', 'subunitB'),
                             ('subunitB', 'subunitA')]
        # get different pairs of subunits
        self.pairs = set()
        for c in self.crosslinks:
            if c[0] > c[2]:
                self.pairs.add((c[2], c[0]))
            else:
                self.pairs.add((c[0], c[2]))

        self.xlinks = bx.Xlinks()
        for c in self.crosslinks:
            self.xlinks.add(*c)


    def test_xlinks(self):
        """
        Test the Xlinks class
        """
        for p in self.pairs:
            xlist = self.xlinks.get_xlinks_for_pair(p)
            # build manually the list that xlist should build
            manual_xlist = []
            for c in self.crosslinks:
                if p == (c[0],c[2]):
                    manual_xlist.append((c[1], c[3], c[4]))
                elif p == (c[2], c[0]):
                    manual_xlist.append((c[3], c[1], c[4]))
            # check the number of crosslinks for the pair
            self.assertEqual(len(manual_xlist), len(xlist))
            # check amino acids and distances
            for x in xlist:
                self.assertTrue(x in manual_xlist)

    def test_build_order(self):
        """
            Test the the order recommendation for the xlinks
        """
        dock_order = bx.DockOrder()
        dock_order.set_xlinks(self.xlinks)
        docking_pairs = dock_order.get_docking_order()
        for p, q in zip(docking_pairs,self.optimal_order):
            self.assertEqual(p, q)



if __name__ == '__main__':
    IMP.test.main()
