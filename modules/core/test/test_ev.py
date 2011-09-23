import IMP
import IMP.test
import sys
import IMP.atom
import IMP.core
import IMP.container
import os
import time

class Tests(IMP.test.TestCase):
    """Tests for sampled density maps"""

    def _setup(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        #init IMP model ( the environment)
        m = IMP.Model()
        h0=IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                              m)
        h1=IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                              m)
        rb0 = IMP.atom.create_rigid_body(h0)
        rb1 = IMP.atom.create_rigid_body(h1)
        rb0.set_reference_frame(IMP.algebra.ReferenceFrame3D())
        rb1.set_reference_frame(IMP.algebra.ReferenceFrame3D())
        l0= IMP.atom.get_leaves(h0)
        l1= IMP.atom.get_leaves(h1)
        #set the restraint
        lsc=IMP.container.ListSingletonContainer(l0+l1)
        r=IMP.core.ExcludedVolumeRestraint(lsc, 1)
        m.add_restraint(r)
        return (m, rb0, rb1, l0, l1)
    def test_filters(self):
        """Test filters on excluded volume"""
        m = IMP.Model()
        m.set_log_level(IMP.SILENT)
        h0=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),
                              m)
        l0= IMP.atom.get_leaves(h0)[0:200]
        #set the restraint
        lsc=IMP.container.ListSingletonContainer(l0)
        r=IMP.core.ExcludedVolumeRestraint(lsc, 1, 0)
        m.add_restraint(r)
        cpc= IMP.container.ClosePairContainer(lsc, 0, 0)
        cr= IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), cpc)
        m.add_restraint(cr)
        f= IMP.atom.BondedPairFilter()
        r.add_pair_filter(f)
        cpc.add_pair_filter(f)
        print r.evaluate(False)
        print cr.evaluate(False)
        self.assertAlmostEqual(r.evaluate(False), cr.evaluate(False),
                               delta=.1)
if __name__ == '__main__':
    IMP.test.main()
