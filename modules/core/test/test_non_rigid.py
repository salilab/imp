import IMP
import IMP.test
import IMP.core
import IMP.container
import math

class ICMover(IMP.core.MonteCarloMover):
    def __init__(self, m, pi, r):
        IMP.core.MonteCarloMover.__init__(self, m, "ICMover"+m.get_particle(pi).get_name())
        self.pi = pi
        self.ball = IMP.algebra.Sphere3D(IMP.algebra.get_zero_vector_3d(), r)
    def do_propose(self):
        nrm = IMP.core.NonRigidMember(self.get_model(), self.pi)
        ic = nrm.get_internal_coordinates()
        self.old = ic
        ic += IMP.algebra.get_random_vector_in(self.ball)
        nrm.set_internal_coordinates(ic)
        return IMP.core.MonteCarloMoverResult([self.pi], 1.0)
    def do_reject(self):
        nrm = IMP.core.NonrigidMember(self.get_model(), self.pi)
        nrm.set_internal_coordinates(self.old)
        del self.old
    def do_accept(self):
        del self.old
    def do_get_inputs(self):
        return [self.get_model().get_particle(self.pi)]

class Tests(IMP.test.TestCase):
    """Tests for angle restraints"""
    def _move(self, movers):
        for m in movers:
            m.propose()
            m.accept()
    def _check_close_pairs(self, m, cpcpps):
        m.update()
        ps = [p.get_index() for p in m.get_particles() if not IMP.core.RigidBody.particle_is_instance(p)]
        #print cpcpps
        for i in range(0, len(ps)):
            if IMP.core.NonRigidMember.particle_is_instance(m, ps[i]):
                d= IMP.core.NonRigidMember(m, ps[i])
                ic = d.get_internal_coordinates()
                oc = d.get_coordinates()
                rf = d.get_rigid_body().get_reference_frame()
                oic = rf.get_global_coordinates(ic)
                dist = IMP.algebra.get_distance(oic, oc)
                #print ic, oc, rf, oic, dist
                self.assert_(dist < .1)
            for j in range(0, i):
                pp = (ps[i], ps[j])
                ppi = (ps[j], ps[i])
                d0 = IMP.core.XYZR(m, pp[0])
                d1 = IMP.core.XYZR(m, pp[1])
                #print m.get_particle(ps[i]).get_name(), m.get_particle(ps[j]).get_name(), pp
                if IMP.core.RigidMember.particle_is_instance(m, ps[i]) and IMP.core.RigidMember.particle_is_instance(m, ps[j])\
                    and IMP.core.RigidMember(m, ps[i]).get_rigid_body() == IMP.core.RigidMember(m, ps[j]).get_rigid_body():
                    self.assert_(pp not in cpcpps and ppi not in cpcpps)
                else:
                    d = IMP.core.get_distance(d0, d1)
                    if d < 0:
                        self.assert_(pp in cpcpps or ppi in cpcpps)
    def _create_rigid_body(self, m, name):
        rbp = m.add_particle("rb"+name)
        rbd = IMP.core.RigidBody.setup_particle(m.get_particle(rbp), IMP.algebra.ReferenceFrame3D())
        for i in range(0, 3):
            pi = m.add_particle("p"+name+str(i))
            c = IMP.algebra.get_zero_vector_3d()
            c[i] = 1
            IMP.core.XYZR.setup_particle(m, pi, IMP.algebra.Sphere3D(c, 1))
            rbd.add_member(pi)
        return rbd
    def _add_non_rigid(self, m, rb, name):
        ret = []
        for i in range(0,2):
            pi = m.add_particle("nrb"+name+"-"+str(i))
            c = IMP.algebra.get_zero_vector_3d()
            c[i] = -1
            IMP.core.XYZR.setup_particle(m, pi, IMP.algebra.Sphere3D(c, 1))
            rb.add_non_rigid_member(pi)
            ret.append(pi)
        return ret
    def test_deriv(self):
        """Check non-rigid particles"""
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.Model()
        r0 = self._create_rigid_body(m, "0")
        r1 = self._create_rigid_body(m, "1")
        nr0 = self._add_non_rigid(m, r0, "0")
        nr1 = self._add_non_rigid(m, r1, "1")
        cpc = IMP.container.ClosePairContainer([p for p in m.get_particles() if not IMP.core.RigidBody.particle_is_instance(p)],
                                               0, IMP.core.RigidClosePairsFinder(), 0)
        movers = [IMP.core.RigidBodyMover(m, r0.get_particle_index(), 1, 1),
                  IMP.core.RigidBodyMover(m, r1.get_particle_index(), 1, 1)]\
                  + [ICMover(m, x, 1) for x in nr0 + nr1]
        cpc.set_was_used(True)
        for i in range(0, 100):
            m.update()
            self._check_close_pairs(m, cpc.get_indexes())
            self._move(movers)

    def test_2(self):
        """Check non-rigid particles with ExcludedVolumeRestraint"""
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.Model()
        r0 = self._create_rigid_body(m, "0")
        r1 = self._create_rigid_body(m, "1")
        nr0 = self._add_non_rigid(m, r0, "0")
        nr1 = self._add_non_rigid(m, r1, "1")
        evr = IMP.core.ExcludedVolumeRestraint([p for p in m.get_particles() if not IMP.core.RigidBody.particle_is_instance(p)])
        movers = [IMP.core.RigidBodyMover(m, r0.get_particle_index(), 1, 1),
                  IMP.core.RigidBodyMover(m, r1.get_particle_index(), 1, 1)]\
                  + [ICMover(m, x, 1) for x in nr0 + nr1]
        for i in range(0, 100):
            pis = []
            evr.evaluate(False)
            self._check_close_pairs(m, evr.get_indexes())
            self._move(movers)
        del evr
        del m
        del nr0
        del nr1
        del r0
        del r1
    def test_3(self):
        """Check that particles can be converted between rigid and non-rigid"""
        IMP.base.set_log_level(IMP.base.SILENT)
        m = IMP.Model()
        r0 = self._create_rigid_body(m, "0")
        nr0 = self._add_non_rigid(m, r0, "0")
        assert(IMP.core.NonRigidMember.particle_is_instance(m, nr0[0]))
        r0.set_is_rigid_member(nr0[0], True)
        assert(IMP.core.RigidMember.particle_is_instance(m, nr0[0]))
        assert(not IMP.core.NonRigidMember.particle_is_instance(m, nr0[0]))

        r0.set_is_rigid_member(nr0[0], False)
        assert(not IMP.core.RigidMember.particle_is_instance(m, nr0[0]))
        assert(IMP.core.NonRigidMember.particle_is_instance(m, nr0[0]))
if __name__ == '__main__':
    IMP.test.main()
