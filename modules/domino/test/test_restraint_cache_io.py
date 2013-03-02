import IMP
import IMP.test
import IMP.domino
import IMP.core
import RMF
import random

num_particles=3
num_states=2
class Tests(IMP.test.TestCase):
    def _create_stuff(self):
        m= IMP.Model()
        m.set_log_level(IMP.base.SILENT)
        ps =[IMP.Particle(m) for i in range(0,num_particles)]
        for p in ps:
            d=IMP.core.XYZR.setup_particle(p)
            d.set_coordinates_are_optimized(True)
            d.set_radius(1)
        sps=[p for p in ps]
        random.shuffle(sps)
        r= IMP.core.ExcludedVolumeRestraint(sps, 1)
        r.set_log_level(IMP.base.SILENT)
        r.set_name("evr")
        r.set_maximum_score(1)
        pst= IMP.domino.ParticleStatesTable()
        cache= IMP.domino.RestraintCache(pst)
        cache.set_log_level(IMP.base.VERBOSE)
        # only one really is used
        r.set_was_used(True)
        pst.set_was_used(True)
        cache.set_was_used(True)
        return (m, ps, r, pst, cache)
    def test_decomposition(self):
        """Test simple I/O of restraint cache"""
        if not IMP.domino.IMP_DOMINO_HAS_RMF:
            self.skipTest("domino configured without RMF")
        else:
            import RMF
        bb= IMP.algebra.BoundingBox3D([0,0,0], [10,10,10])
        vs= [IMP.algebra.get_random_vector_in(bb) for i in range(0,num_states)]
        print vs
        (m0, ps0, r0, pst0, cache0)=self._create_stuff()
        (m1, ps1, r1, pst1, cache1)=self._create_stuff()
        for p in ps0:
            pst0.set_particle_states(p, IMP.domino.XYZStates(vs))
        for p in ps1:
            pst1.set_particle_states(p, IMP.domino.XYZStates(vs))
        s0=IMP.domino.Subset(ps0)
        cache0.add_restraints([r0])
        cache1.add_restraints([r1])
        rs0= cache0.get_restraints(s0, [])
        print rs0
        asss= [IMP.domino.Assignment([random.randint(0,num_states-1)
                                      for i in s0]) for i in range(0,100)]
        print asss
        for ass in asss:
            for r in rs0:
                cache0.get_score(r, s0, ass)
        #self.assertEqual(cache0.get_number_of_entries(), len(asss)*len(rs0))
        fn= self.get_tmp_file_name("cache_io.hdf5")
        fl= RMF.HDF5.create_file(fn)
        cache0.save_cache(ps0, rs0, fl, 10000000)
        # depend on cache validate call
        cache1.load_cache(ps1, fl)
        print "Have", cache1.get_number_of_entries()
        self.assertEqual(cache0.get_number_of_entries(), cache1.get_number_of_entries())

if __name__ == '__main__':
    IMP.test.main()
