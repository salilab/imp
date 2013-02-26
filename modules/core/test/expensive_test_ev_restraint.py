import IMP
import IMP.test
import IMP.core
import IMP.container

class ExcludedVolumeRestraintTests(IMP.test.TestCase):
    """Tests for excluded volume restraints"""
    def _create_rigid_body(self, m, n, r):
        ls= IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), r)
        ap=[]
        for i in range(0,n):
            cs= IMP.algebra.Sphere3D(IMP.algebra.get_random_vector_on(ls), r)
            d= IMP.core.XYZR.setup_particle(IMP.Particle(m), cs)
            ap.append(d);
        rbp= IMP.core.RigidBody.setup_particle(IMP.Particle(m), ap)
        #rbp.set_log_level(IMP.base.SILENT)
        rbp.set_coordinates_are_optimized(True)
        return rbp
    def _create_xyzrs(self, m, n, r):
        ap=[]
        for i in range(0,n):
            d= IMP.core.XYZR.setup_particle(IMP.Particle(m),
                                           IMP.algebra.Sphere3D(IMP.algebra.Vector3D(0,0,0), r))
            d.set_coordinates_are_optimized(True)
            ap.append(d);
        return ap
    def _setup_ev_restraint(self):
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model()
        m.set_log_level(IMP.base.SILENT)
        all=[]
        rbs=[]
        xyzrs=[]
        for i in range(0,5):
            rb= self._create_rigid_body(m, 10, 1)
            rbs.append(rb)
            all.extend(rb.get_members())
        xyzrs=self._create_xyzrs(m, 10, 1)
        all.extend(xyzrs)
        allc=IMP.container.ListSingletonContainer(all)
        r= IMP.core.ExcludedVolumeRestraint(allc, 1)
        r.set_log_level(IMP.base.SILENT)
        m.add_restraint(r)
        bb= IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0,0,0),
                                      IMP.algebra.Vector3D(100,100,100));
        bbr= IMP.container.SingletonsRestraint(IMP.core.BoundingBox3DSingletonScore(IMP.core.HarmonicUpperBound(0,1),
                                                                                    bb),
                                               allc)
        m.add_restraint(bbr)
        return (m, r, xyzrs, rbs)
    def _setup_movers(self, xyzrs, rbs):
        mvs=[]
        for p in xyzrs:
            mc= IMP.core.BallMover([p], 1)
            mc.set_log_level(IMP.base.SILENT)
            mvs.append(mc)
        for p in rbs:
            mc= IMP.core.RigidBodyMover(IMP.core.RigidBody(p), 1, .1)
            mvs.append(mc)
        return mvs
    def test_ev(self):
        """Testing excluded volume restraint"""
        (m,r, xyzrs, rbs)= self._setup_ev_restraint()
        print "mc"
        o= IMP.core.MonteCarlo()
        mvs= self._setup_movers(xyzrs, rbs)
        o.set_movers(mvs)
        o.set_model(m)
        print "opt"
        # rely on internal checks
        print o.optimize(1000)
        print "inspect", m.evaluate(False)
    def test_evs(self):
        """Testing excluded volume serial restraint"""
        (m,r, xyzrs, rbs)= self._setup_ev_restraint()
        print "mc"
        o= IMP.core.MonteCarlo()
        mvs= self._setup_movers(xyzrs, rbs)
        sm= IMP.core.SerialMover(mvs)
        o.set_movers([sm])
        o.set_model(m)
        print "opt"
        # rely on internal checks
        print o.optimize(1000)
        print "inspect", m.evaluate(False)
if __name__ == '__main__':
    IMP.test.main()
