import unittest
import IMP.rmf
import IMP.test
import IMP.container
import RMF
from IMP.algebra import *

class Tests(IMP.test.TestCase):

    def test_2(self):
        """Test that writing restraints doesn't reset model dependencies"""
        for suffix in RMF.suffixes:
            f= RMF.create_rmf_file(self.get_tmp_file_name("restrnp2."+suffix))
            m= IMP.Model()
            ps= [IMP.Particle(m) for i in range(0,3)]
            ds= [IMP.core.XYZR.setup_particle(p) for p in ps]
            for d in ds:
                d.set_radius(1)
            IMP.rmf.add_particles(f, ds)
            cpc= IMP.container.ConsecutivePairContainer(ps)
            r= IMP.container.PairsRestraint(IMP.core.SoftSpherePairScore(1), cpc)
            r.set_model(m)
            r.evaluate(False)
            IMP.rmf.add_restraint(f, r)
            IMP.base.set_log_level(IMP.base.MEMORY)
            sf= IMP.core.RestraintsScoringFunction([r])
            sf.evaluate(False)
            IMP.rmf.save_frame(f, 0)
            self.assertTrue(m.get_has_dependencies())

if __name__ == '__main__':
    unittest.main()
