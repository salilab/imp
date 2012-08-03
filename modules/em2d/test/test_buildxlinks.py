import IMP
import IMP.test
import IMP.core as core
import IMP.atom as atom
import IMP.em2d as em2d
import IMP.em2d.utility as utility
import IMP.em2d.imp_general.io as io
import IMP.em2d.imp_general.representation as representation
import IMP.em2d.solutions_io as solutions_io
import IMP.em2d.Database as Database
import IMP.algebra as alg

import sys
import os
import itertools
import unittest

try:
    import IMP.em2d.buildxlinks as bx
except ImportError, detail:
    bx = None
    bxerr = str(detail)

class TestBuildXlinks(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        if bx is None:
            self.skipTest(bxerr)
        self.crosslinks = [ ("subunitA","A", 1, "subunitB","B", 1, 10),
                       ("subunitA","A", 2, "subunitB","B", 2, 20),
                       ("subunitB","B", 3, "subunitA","A", 3, 30),
                       ("subunitB","B", 4, "subunitC","C", 1, 40),
                       ("subunitB","B", 5, "subunitC","C", 2, 20),
                       ("subunitD","D", 6, "subunitC","C", 1, 10),
                       ("subunitC","C", 3, "subunitD","D", 2, 70),
                       ("subunitC","C", 4, "subunitD","D", 3, 80),
                       ("subunitD","D", 4, "subunitE","E", 1, 10),
                       ("subunitE","E", 5, "subunitD","D", 2, 30),
                       ("subunitD","D", 6, "subunitE","E", 3, 40),
                       ("subunitD","D", 7, "subunitE","E", 4, 50),
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
        self.xlinks = bx.XlinksDict()
        for c in self.crosslinks:
            x = bx.Xlink(*c)
            self.xlinks.add(x)
        self.pairs = set()
        for c in self.crosslinks:
            self.pairs.add((c[0],c[3]))

    def test_xlinks(self):
        """
        Test the Xlinks class
        """
        for p in self.pairs:
            xlist = self.xlinks.get_xlinks_for_pair(p)
            # build manually the list that xlist should build
            manual_xlist = []
            xl = None
            for c in self.crosslinks:
                if p == (c[0],c[3]) or p == (c[3], c[0]):
                    xl = bx.Xlink(*c)
                    if p == (c[3], c[0]):
                        xl.swap()
                    manual_xlist.append(xl)
            # check the number of crosslinks for the pair
            self.assertEqual(len(manual_xlist), len(xlist))
            # check amino acids and distances
            aa = [(xl.first_residue,xl.second_residue, xl.distance) for xl in xlist]
            ll = [(xl.first_residue,xl.second_residue, xl.distance) for xl in manual_xlist]
            for x in ll:
                self.assertTrue(x in aa)

        xlistDE = self.xlinks.get_xlinks_for_pair(("subunitD","subunitE"))
        xlistED = self.xlinks.get_xlinks_for_pair(("subunitE","subunitD"))
        self.assertEqual( len(xlistDE), 4)
        self.assertEqual( len(xlistED), 4)
        for y, x in zip(xlistDE, xlistED):
            self.assertEqual(x.first_residue ,y.second_residue)
            self.assertEqual(y.first_residue ,x.second_residue)
            self.assertEqual(x.distance, y.distance)


    def test_build_order(self):
        """
            Test the order recommendation for the xlinks
        """
        dock_order = bx.DockOrder()
        dock_order.set_xlinks(self.xlinks)
        docking_pairs = dock_order.get_docking_order()
        for p, q in zip(docking_pairs,self.optimal_order):
            self.assertEqual(p, q)


class TestInitialDocking(IMP.test.TestCase):
    def setUp(self):

        IMP.test.TestCase.setUp(self)
        if bx is None:
            self.skipTest(bxerr)
        self.max_distance = 30
        self.crosslinks =  [       ("3sfdB","B",  23,"3sfdA","A",456,self.max_distance),
                                   ("3sfdB","B", 241,"3sfdC","C",112,self.max_distance),
                                   ("3sfdB","B", 205,"3sfdD","D", 37,self.max_distance),
                                   ("3sfdB","B", 177,"3sfdD","D", 99,self.max_distance),
                                   ("3sfdC","C",   9,"3sfdD","D", 37,self.max_distance),
                                   ("3sfdC","C",  78,"3sfdD","D",128,self.max_distance),
                                    ]
        self.xlinks = bx.XlinksDict()
        for c in self.crosslinks:
            self.xlinks.add(bx.Xlink(*c))

    def test_docking_one_crosslink(self):
        """
        Test the initial docking that is done based on minimizing the
        distances of the cross-linking restraints
        """
        mydock = bx.InitialDockingFromXlinks()
        xl =  self.xlinks.get_xlinks_for_pair(("3sfdB","3sfdA"))
        mydock.set_xlinks(xl)
        self.assertEqual(len(mydock.xlinks_list), 1)
        mydock.clear_xlinks()
        self.assertEqual(len(mydock.xlinks_list), 0)
        model = IMP.Model()
        fn_receptor = self.get_input_file_name("3sfdB.pdb")
        h_receptor =  atom.read_pdb(fn_receptor, model,
                                    atom.NonWaterNonHydrogenPDBSelector())
        fn_ligand = self.get_input_file_name("3sfdA.pdb")
        h_ligand =  atom.read_pdb(fn_ligand, model,
                                    atom.NonWaterNonHydrogenPDBSelector())
        mydock.set_xlinks(xl)
        mydock.set_hierarchies(h_receptor, h_ligand)
        p = IMP.Particle(model)
        core.RigidBody.setup_particle(p, atom.get_leaves(h_receptor))
        rb_receptor = core.RigidBody(p)
        p = IMP.Particle(model)
        core.RigidBody.setup_particle(p, atom.get_leaves(h_ligand))
        rb_ligand = core.RigidBody(p)
        mydock.set_rigid_bodies(rb_receptor, rb_ligand)
        mydock.move_ligand()

        c1 = mydock.get_residue_coordinates(h_ligand,"A", 456)
        c2 = mydock.get_residue_coordinates(h_receptor,"B",23)
        dist = alg.get_distance(c1,c2)
        self.assertLessEqual(dist, self.max_distance)

    def test_docking_multiple_crosslinks(self):
        """
        Test the initial docking that is done based on minimizing the
        distances of the cross-linking restraints
        """
        mydock = bx.InitialDockingFromXlinks()
        xl =  self.xlinks.get_xlinks_for_pair(("3sfdC","3sfdD"))
        mydock.set_xlinks(xl)
        model = IMP.Model()
        fn_receptor = self.get_input_file_name("3sfdC.pdb")
        h_receptor =  atom.read_pdb(fn_receptor, model,
                                    atom.NonWaterNonHydrogenPDBSelector())
        fn_ligand = self.get_input_file_name("3sfdD.pdb")
        h_ligand =  atom.read_pdb(fn_ligand, model,
                                    atom.NonWaterNonHydrogenPDBSelector())
        mydock.set_hierarchies(h_receptor, h_ligand)
        p = IMP.Particle(model)
        core.RigidBody.setup_particle(p, atom.get_leaves(h_receptor))
        rb_receptor = core.RigidBody(p)
        p = IMP.Particle(model)
        core.RigidBody.setup_particle(p, atom.get_leaves(h_ligand))
        rb_ligand = core.RigidBody(p)
        mydock.set_rigid_bodies(rb_receptor, rb_ligand)
        mydock.move_ligand()

        for res1,res2 in zip([9, 78], [37, 128]):
            c1 = mydock.get_residue_coordinates(h_receptor,"C", res1)
            c2 = mydock.get_residue_coordinates(h_ligand, "D", res2)
            dist = alg.get_distance(c1,c2)
            self.assertLessEqual(dist, self.max_distance)


if __name__ == '__main__':
    IMP.test.main()
