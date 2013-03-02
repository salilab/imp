import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.algebra


class Tests(IMP.test.TestCase):

    def test_global_min2(self):
        """Test clustering of states"""
        m= IMP.Model()
        p0= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p0)
        p1= IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p1)
        pts=[]
        for i in range(0,101, 5):
            for j in range(0,101,5):
                pts.append(IMP.algebra.Vector3D(i,j,0))
        fa=[]
        fb=[]
        for i in range(len(pts)):
            v= pts[i]
            if v[0] < 20 and v[1] < 20:
                fa.append(i)
            if v[0] >80 and v[1] > 80:
                fb.append(i)
        self.assertEqual(len(fa), len(fb))
        print "sets are"
        print fa
        print fb
        s= IMP.domino.Subset([p0,p1])
        states=[]
        for i in fa:
            for j in fb:
                states.append(IMP.domino.Assignment([i,j]))
                states.append(IMP.domino.Assignment([j,i]))
        pst= IMP.domino.ParticleStatesTable()
        ps=IMP.domino.XYZStates(pts)
        pst.set_particle_states(p0, ps)
        pst.set_particle_states(p1, IMP.domino.XYZStates(pts))
        IMP.base.set_log_level(IMP.base.TERSE)
        ssl= IMP.domino.get_state_clusters(s, states, pst, 6)
        print "Solutions are"
        for s in ssl:
            print s
if __name__ == '__main__':
    IMP.test.main()
