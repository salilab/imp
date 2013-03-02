import unittest
import IMP.rmf
import IMP.test
import IMP.container
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def test_0(self):
        """Test writing restraint sets to rmf"""
        for suffix in RMF.suffixes:
            fn=self.get_tmp_file_name("restsets."+suffix)
            f= RMF.create_rmf_file(fn)
            m= IMP.Model()
            p= IMP.Particle(m)
            IMP.atom.Hierarchy.setup_particle(p)
            IMP.atom.Mass.setup_particle(p, 1)
            IMP.core.XYZR.setup_particle(p).set_radius(1)
            IMP.rmf.add_hierarchies(f, [p]);
            rs= IMP.RestraintSet(m, 1.0)
            r= IMP.kernel._ConstRestraint(1, [p])
            r.set_name("restraint")
            rs.add_restraint(r)
            rs.evaluate(False)
            IMP.rmf.add_restraints(f, [rs])
            IMP.rmf.save_frame(f, 0)
            del f
            f= RMF.open_rmf_file_read_only(fn)
            hs= IMP.rmf.create_hierarchies(f, m)
            nrs= IMP.rmf.create_restraints(f, m)
            print nrs
            self.assertEqual(len(nrs), 1)
            rsnrs0= IMP.RestraintSet.get_from(nrs[0])
            self.assertEqual(len(rsnrs0.get_restraints()), 1)
            self.assertEqual(rsnrs0.get_name(), rs.get_name())
            rb= rsnrs0.get_restraints()[0]
            self.assertEqual(rb.evaluate(False), r.evaluate(False))
            self.assertEqual(rb.get_name(), r.get_name())

    def test_1(self):
        """Test handling non-disjoint restraint sets"""
        for suffix in RMF.suffixes:
            fn=self.get_tmp_file_name("ndrestsets."+suffix)
            f= RMF.create_rmf_file(fn)
            m= IMP.Model()
            p= IMP.Particle(m)
            IMP.atom.Hierarchy.setup_particle(p)
            IMP.atom.Mass.setup_particle(p, 1)
            IMP.core.XYZR.setup_particle(p).set_radius(1)
            IMP.rmf.add_hierarchies(f, [p]);
            rs0= IMP.RestraintSet(m, 1.0)
            rs1= IMP.RestraintSet(m, 1.0)
            r= IMP.kernel._ConstRestraint(1, [p])
            r.set_name("restraint")
            rs0.add_restraint(r)
            rs0.evaluate(False)
            rs1.add_restraint(r)
            rs1.evaluate(False)
            IMP.rmf.add_restraints(f, [rs0, rs1])
            IMP.rmf.save_frame(f, 0)
            del f
            f= RMF.open_rmf_file_read_only(fn)
            hs= IMP.rmf.create_hierarchies(f, m)
            nrs= IMP.rmf.create_restraints(f, m)
            print nrs
            self.assertEqual(len(nrs), 2)
            rsnrs0= IMP.RestraintSet.get_from(nrs[0])
            rsnrs1= IMP.RestraintSet.get_from(nrs[1])
            self.assertEqual(len(rsnrs0.get_restraints()), 1)
            self.assertEqual(rsnrs0.get_name(), rs0.get_name())
            rb0= rsnrs0.get_restraints()[0]
            self.assertEqual(rb0.evaluate(False), r.evaluate(False))
            self.assertEqual(rb0.get_name(), r.get_name())

            self.assertEqual(len(rsnrs1.get_restraints()), 1)
            self.assertEqual(rsnrs1.get_name(), rs1.get_name())
            rb1= rsnrs0.get_restraints()[0]
            self.assertEqual(rb0, rb1)

if __name__ == '__main__':
    unittest.main()
