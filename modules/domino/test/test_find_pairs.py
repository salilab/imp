import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.algebra
import random

ns=10
class Tests(IMP.test.TestCase):
    def test_global_min1(self):
        """Test close pairs list generation"""
        m= IMP.Model()
        ps= [IMP.Particle(m) for i in range(0,10)]
        ds= [IMP.core.XYZR.setup_particle(p) for p in ps]
        for d in ds:
            d.set_radius(6)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(50,50,50))
        bbs=IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(4,4,4))
        pst= IMP.domino.ParticleStatesTable()
        for p in ps:
            v0=IMP.algebra.get_random_vector_in(bb)
            vs=[v0+IMP.algebra.get_random_vector_in(bbs) for i in range(0,ns)]
            yxz= IMP.domino.XYZStates(vs)
            pst.set_particle_states(p, yxz)
        allp= IMP.domino.get_possible_interactions(ps, 2, pst)
        cpf= IMP.core.GridClosePairsFinder()
        cpf.set_distance(1.8)
        pst.set_was_used(True)
        for rep in range(0,1000):
            for p in ps:
                st= pst.get_particle_states(p)
                st.load_particle_state(random.sample(range(0,ns), 1)[0], p)
            curp= cpf.get_close_pairs(ps)
            print len(curp)
            for p in curp:
                self.assertTrue(p in allp or (p[1], p[0]) in allp)
if __name__ == '__main__':
    IMP.test.main()
