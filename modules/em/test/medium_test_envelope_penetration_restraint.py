import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def _create_density_map(self,res,apix):
        self.dmap=IMP.em.particles2density(self.ps,
                                           res,apix)
        self.dmap.show()
        IMP.em.write_map(self.dmap,"temp.mrc",IMP.em.MRCReaderWriter())
        self.thr=IMP.em.get_threshold_for_approximate_mass(self.dmap,2*IMP.atom.get_mass_from_number_of_residues(len(IMP.atom.get_by_type(self.mh,IMP.atom.RESIDUE_TYPE))))
        print "threshold:",self.thr
    def _load_particles(self):
        self.mh=IMP.atom.read_pdb(self.get_input_file_name('1tdx.pdb'),self.mdl)
        self.ps=IMP.core.get_leaves(self.mh)
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        self.mdl = IMP.Model()
        self._load_particles()
        self._create_density_map(10,1.5)
        self.epr = IMP.em.EnvelopePenetrationRestraint(self.ps,self.dmap,self.thr)
        self.mdl.add_restraint(self.epr)
    def test_evaluate(self):
        """Check the restraint is being evaluated"""
        score = self.mdl.evaluate(False)
        self.assertLess(score, 2, "the score "+str(score)+" should be lower")

    def test_penetration(self):
        trans = IMP.algebra.Vector3D(5,5,5)
        t=IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),trans)
        xyz=IMP.core.XYZs(self.ps)
        for x in xyz:
            x.set_coordinates(t.get_transformed(x.get_coordinates()))
        score = self.mdl.evaluate(False)
        print score
        self.assertGreater(score, 200, "the score "+str(score)+" should be higher")
if __name__ == '__main__':
    IMP.test.main()
