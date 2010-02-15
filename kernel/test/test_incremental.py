import unittest
import IMP
import IMP.test


fk= IMP.FloatKey("hi")

def score(p, da):
    v= p.get_value(fk)
    if da is not None:
        p.add_to_derivative(fk, v, da)
    return v

class DummyRestraint(IMP.Restraint):
    """Dummy do-nothing restraint"""
    def __init__(self, p):
        IMP.Restraint.__init__(self)
        self.p=p
    def unprotected_evaluate(self, accum):
        return score(self.p, accum)
    def do_show(self, fh):
        fh.write("DummyRestraint")
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")
    def get_input_particles(self):
        return IMP.ParticlesTemp(1,self.p)
    def get_input_containers(self):
        return IMP.ContainersTemp()


class DummyIncrementalRestraint(IMP.Restraint):
    """Dummy do-nothing incremental restraint"""
    def __init__(self, p):
        IMP.Restraint.__init__(self)
        self.p=p
    def unprotected_evaluate(self, accum):
        self.oldvalue= score(self.p, accum)
        print "non-incremental "+ str(self.oldvalue)
        return self.oldvalue
    def get_is_incremental(self):
        return True
    def unprotected_incremental_evaluate(self, accum):
        print "Particle " +self.p.get_name() + " is " +str(self.p.get_is_changed())
        print "Current " + str(self.p.get_value(fk)) + " old "\
            + str(self.p.get_prechange_particle().get_value(fk))
        if self.p.get_is_changed():
            if accum is not None:
                da= IMP.DerivativeAccumulator(accum, -1)
            else:
                da=None
            ov= score(self.p.get_prechange_particle(), da)
            print "ov="+str(ov)
            v= score(self.p, accum)
            print "v="+str(v)
            print "old="+str(self.oldvalue)
            return self.oldvalue-ov+v
        else:
            return self.oldvalue
    def do_show(self, fh):
        fh.write("DummyRestraint")
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")
    def get_input_particles(self):
        return IMP.ParticlesTemp(1,self.p)
    def get_input_containers(self):
        return IMP.ContainersTemp()


class ModelTests(IMP.test.TestCase):
    def _setup_model(self):
        m = IMP.Model()
        IMP.set_log_level(IMP.TERSE)
        ps= IMP.Particles()
        n=1
        for i in range(0,n):
            p= IMP.Particle(m)
            ps.append(p)
            p.add_attribute(fk, i, True)
            r= DummyRestraint(p)
            m.add_restraint(r)
        return (m, ps)
    def _check_evaluate(self, m, ps, deriv):
        v= m.evaluate(deriv)
        cv=0
        for p in ps:
            cv= cv+2*p.get_value(fk)
        self.assertEqual(v, cv)
        print "score is " +str(v)
        if deriv:
            for p in ps:
                self.assertEqual(p.get_derivative(fk),2*p.get_value(fk))
    def test_state_show(self):
        """Test incremental evaluation without derivatives"""
        (m, ps)= self._setup_model()
        v= m.evaluate(False)
        print v
        m.set_is_incremental(True)
        v1= m.evaluate(False)
        self.assertEqual(v, v1)
    def test_mark_dirty(self):
        """Test that particles become dirty"""
        (m, ps)= self._setup_model()
        m.set_is_incremental(True)
        self.assert_(ps[0].get_is_changed())
        m.evaluate(False)
        ps[0].set_value(fk, 2)
        self.assertEqual(ps[0].get_prechange_particle().get_value(fk), 0)
    def test_evaluate(self):
        """Test incremental evaluation without derivatives"""
        (m, ps)= self._setup_model()
        m.set_is_incremental(True)
        ps[0].set_value(fk, 2)
        for p in ps:
            r= DummyIncrementalRestraint(p)
            m.add_restraint(r)
        self._check_evaluate(m, ps, False)
        ps[0].set_value(fk, 6)
        self._check_evaluate(m, ps, False)
    def test_deriv_evaluate(self):
        """Test incremental evaluation with derivatives"""
        (m, ps)= self._setup_model()
        m.set_is_incremental(True)
        ps[0].set_value(fk, 2)
        for p in ps:
            r= DummyIncrementalRestraint(p)
            m.add_restraint(r)
        self._check_evaluate(m, ps, True)
        ps[0].set_value(fk, 6)
        self._check_evaluate(m, ps, True)

if __name__ == '__main__':
    unittest.main()
