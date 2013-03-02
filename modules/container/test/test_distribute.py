import IMP
import IMP.test
import IMP.container
import math

ik= IMP.IntKey("hi")
class Odd(IMP.SingletonPredicate):
    def get_value(self, p):
        return p.get_value(ik)%2
    def _do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]
class Mod5(IMP.SingletonPredicate):
    def get_value(self, p):
        return p.get_value(ik)%5
    def _do_get_inputs(self, m, pis):
        return [m.get_particle(i) for i in pis]

class Tests(IMP.test.TestCase):
    """Tests for all pairs pair container"""

    def test_allp(self):
        """Checking distribute particles"""
        m= IMP.Model()
        ps=[]
        for i in range(0,10):
            p= IMP.Particle(m)
            ps.append(p)
            p.add_attribute(ik, i)
        lp= IMP.container.ListSingletonContainer(ps)
        dss= IMP.container.DistributeSingletonsScoreState(lp)
        m.add_score_state(dss)
        even= dss.add_predicate(Odd(), 0)
        odd= dss.add_predicate(Odd(), 1)
        zf= dss.add_predicate(Mod5(), 0)
        m.update()
        print even.get_particles()
        print odd.get_particles()
        print zf.get_particles()
        pse=[ps[i*2] for i in range(len(ps)/2)]
        pso=[ps[i*2+1] for i in range(len(ps)/2)]
        psf=[ps[0], ps[5]]
        self.assertEqual(even.get_particles(), pse)
        self.assertEqual(odd.get_particles(), pso)
        self.assertEqual(zf.get_particles(), psf)
if __name__ == '__main__':
    IMP.test.main()
