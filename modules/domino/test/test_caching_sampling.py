from __future__ import print_function
import IMP
import IMP.test
import IMP.domino
import IMP.core


class Tests(IMP.test.TestCase):

    def test_global_min3(self):
        """Test caching sampling with edge scores"""
        m = IMP.Model()
        m.set_log_level(IMP.SILENT)
        ps = []
        ds = []
        for i in range(0, 3):
            p = IMP.Particle(m)
            d = IMP.core.XYZ.setup_particle(p)
            print(d)
            ds.append(d)
            ps.append(p)
        pts = [IMP.algebra.Vector3D(0, 0, 0),
               IMP.algebra.Vector3D(1, 0, 0),
               IMP.algebra.Vector3D(2, 0, 0),
               IMP.algebra.Vector3D(3, 0, 0)]
        particle_state = IMP.domino.XYZStates(pts)
        pst = IMP.domino.ParticleStatesTable()
        r = IMP.RestraintSet(m)
        for p in ps:
            pst.set_particle_states(p, particle_state)
            r.add_restraint(IMP._ConstRestraint(m, [p], 1))
        sst = IMP.domino.BranchAndBoundAssignmentsTable(pst, [])
        r.add_restraint(IMP.core.DistanceRestraint(m,
            IMP.core.Harmonic(1, 1), ps[0], ps[1]))
        r.add_restraint(IMP.core.DistanceRestraint(m,
            IMP.core.Harmonic(1, 1), ps[1], ps[2]))
        #sevt= IMP.domino.ModelSubsetEvaluatorTable(m, pst)
        sampler = IMP.domino.DominoSampler(m, pst)
        sampler.set_restraints(r)
        # sampler.set_subset_evaluator_table(sevt);
        sampler.set_log_level(IMP.VERBOSE)
        # sampler.set_assignments_table(sst)
        r.set_maximum_score(3.1)
        print("getting sample")
        cs = sampler.create_sample()
        print("done")
        self.assertGreater(cs.get_number_of_configurations(), 0)
        for i in range(0, cs.get_number_of_configurations()):
            print("loading ", i)
            cs.load_configuration(i)
            print("testing")
            dist = IMP.core.get_distance(ds[0], ds[1])
            print(dist)
            self.assertLess((dist - 1) ** 2, .1)
            self.assertLess((IMP.core.get_distance(ds[1], ds[2]) - 1) ** 2, .1)

        # test that they are the right distance
if __name__ == '__main__':
    IMP.test.main()
