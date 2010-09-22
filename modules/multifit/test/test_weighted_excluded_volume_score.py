import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.helper
import os
import time

class SampleTests(IMP.test.TestCase):
    """Tests for sampled density maps"""

    def setUp(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)

        #init IMP model ( the environment)
        self.mdl = IMP.Model()
        self.sel=IMP.atom.CAlphaPDBSelector()
        self.mhs=IMP.atom.Hierarchies()
        self.mhs.append(IMP.atom.read_pdb(self.get_input_file_name("1tyq_A.pdb"),
                             self.mdl,self.sel))
        self.mhs.append(IMP.atom.read_pdb(self.get_input_file_name("1tyq_D.pdb"),
                             self.mdl,self.sel))
        self.voxel_size=1.
        for mh in self.mhs:
            IMP.atom.add_radii(mh)
            IMP.multifit.add_surface_index(mh,self.voxel_size)
        self.rbs=IMP.helper.set_rigid_bodies(self.mhs)
        self.trans=[]
        self.trans.append(IMP.algebra.Transformation3D(IMP.algebra.Rotation3D(1.,0.,0.,0.),IMP.algebra.Vector3D(0.,0.,0.)))
        self.trans.append(IMP.algebra.Transformation3D(IMP.algebra.Rotation3D(0.995865,-0.0659049,-0.0582095,-0.0228412),IMP.algebra.Vector3D(5.67356,-7.52739,0.57034)))
        self.trans.append(IMP.algebra.Transformation3D(
           IMP.algebra.Rotation3D(0.953379,0.073075,0.291331,-0.0292404),
            IMP.algebra.Vector3D(-29.1857,11.4376,23.9337)))
        self.trans.append(IMP.algebra.Transformation3D(
            IMP.algebra.Rotation3D(0.960287,-0.182184,0.142794,0.155778),
            IMP.algebra.Vector3D(-3.98569,-18.6091,20.9074)))

    def test_transforme_surface(self):
        '''Test functionality of get_transformed_into'''
        mh1_surface = IMP.em.particles2surface(IMP.core.get_leaves(self.mhs[0]),1.)
        mh2_surface = IMP.em.particles2surface(IMP.core.get_leaves(self.mhs[1]),1.)

        for i,tt in enumerate(self.trans):
            IMP.core.transform(self.rbs[1],tt)
            st=IMP.em.create_density_map(IMP.atom.get_bounding_box(self.mhs[1]),
                                         self.voxel_size)
            IMP.em.get_transformed_into(mh2_surface,tt,st,False)
            self.assert_(IMP.algebra.get_distance(IMP.core.get_centroid(IMP.core.get_leaves(self.mhs[1])),st.get_centroid())<3.)
            IMP.core.transform(self.rbs[1],tt.get_inverse())
            #notice that get_transformed_into provides a different grid

    def test_score_weighted_excluded_volume(self):
        """Check that weighted excluded volume score works"""
        mh1_surface = IMP.em.particles2surface(IMP.core.get_leaves(self.mhs[0]),1.)
        #move chain A and calculate weighted excluded volume
        ps1=IMP.core.get_leaves(self.mhs[0])
        scores=[]
        scores_by_restraint=[]
        leaves_ref = IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
        fit_r = IMP.multifit.WeightedExcludedVolumeRestraint(self.rbs,
                                                       leaves_ref)
        self.mdl.add_restraint(fit_r)
        for i,t in enumerate(self.trans):
            IMP.core.transform(self.rbs[1],t)
            start = time.clock()
            mh2_surface = IMP.em.particles2surface(IMP.core.get_leaves(self.mhs[1]),1.)
            val=IMP.em.calculate_intersection_score(mh1_surface,mh2_surface)
            scores.append(val)
            scores_by_restraint.append(self.mdl.evaluate(None))
            IMP.core.transform(self.rbs[1],t.get_inverse())
        #test that score makes sense
        for i in range(2):
            self.assert_(scores[i]<scores[2],"Penetration should be scored badly")
        self.assertAlmostEqual(scores[3],0., delta=.1)
        #test that the restraint returns the same scores
        for i in range(4):
            self.assertAlmostEqual(scores[i],scores_by_restraint[i], delta=.1)


if __name__ == '__main__':
    IMP.test.main()
