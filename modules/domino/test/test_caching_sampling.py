import IMP
import IMP.test
import IMP.domino
import IMP.core


class Tests(IMP.test.TestCase):

    def test_global_min3(self):
        """Test caching sampling with edge scores"""
        m= IMP.Model()
        m.set_log_level(IMP.base.SILENT)
        ps=[]
        ds=[]
        for i in range(0,3):
            p= IMP.Particle(m)
            d= IMP.core.XYZ.setup_particle(p)
            print d
            ds.append(d)
            ps.append(p)
        pts=[IMP.algebra.Vector3D(0,0,0),
             IMP.algebra.Vector3D(1,0,0),
             IMP.algebra.Vector3D(2,0,0),
             IMP.algebra.Vector3D(3,0,0)]
        particle_state= IMP.domino.XYZStates(pts)
        pst= IMP.domino.ParticleStatesTable()
        for p in ps:
            pst.set_particle_states(p, particle_state)
            m.add_restraint(IMP.kernel._ConstRestraint(1, [p]))
        sst= IMP.domino.BranchAndBoundAssignmentsTable(pst, [])
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[0], ps[1]))
        m.add_restraint(IMP.core.DistanceRestraint(IMP.core.Harmonic(1,1), ps[1], ps[2]))
        #sevt= IMP.domino.ModelSubsetEvaluatorTable(m, pst)
        sampler= IMP.domino.DominoSampler(m, pst)
        #sampler.set_subset_evaluator_table(sevt);
        sampler.set_log_level(IMP.base.VERBOSE)
        #sampler.set_assignments_table(sst)
        m.set_maximum_score(3.1)
        print "getting sample"
        cs= sampler.get_sample()
        print "done"
        self.assertGreater(cs.get_number_of_configurations(), 0)
        for i in range(0, cs.get_number_of_configurations()):
            print "loading ", i
            cs.load_configuration(i)
            print "testing"
            dist=IMP.core.get_distance(ds[0],ds[1])
            print dist
            self.assertLess((dist-1)**2, .1)
            self.assertLess((IMP.core.get_distance(ds[1],ds[2])-1)**2, .1)

        # test that they are the right distance
if __name__ == '__main__':
    IMP.test.main()
