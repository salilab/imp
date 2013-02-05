import IMP
import IMP.test
import StringIO
import random

class DummyRestraint(IMP.Restraint):
    """Dummy do-nothing restraint"""
    def __init__(self, m, ps=[], cs=[]):
        IMP.Restraint.__init__(self, m)
        self.ps=ps
        self.cs=cs
    def unprotected_evaluate(self, accum):
        return 0.
    def get_version_info(self):
        return IMP.get_module_version_info()
    def get_input_particles(self):
        return self.ps
    def get_input_containers(self):
        return self.cs


class CustomError(Exception):
    pass

class FailingRestraint(IMP.Restraint):
    """Restraint that fails in evaluate"""
    def unprotected_evaluate(self, accum):
        raise CustomError("Custom error message")
    def get_version_info(self):
        return IMP.get_module_version_info()
    def get_input_particles(self):
        return []
    def get_input_containers(self):
        return []

class DummyScoreState(IMP.ScoreState):
    """Dummy do-nothing score state"""
    def __init__(self, ips=[], ics=[], ops=[], ocs=[]):
        IMP.ScoreState.__init__(self)
        self.ips=ips
        self.ics=ics
        self.ops=ops
        self.ocs=ocs
        self.updated=False
    def do_before_evaluate(self):
        IMP.base.add_to_log(IMP.base.TERSE, "Updating dummy "+self.get_name()+"\n")
        self.updated=True
    def do_after_evaluate(self, da):
        self.updated=True
    def get_input_particles(self):
        return self.ips
    def get_output_particles(self):
        #print [type(p) for p in self.ops]
        return self.ops
    def get_input_containers(self):
        return self.ics
    def get_output_containers(self):
        return self.ocs


class ClassScoreState(IMP.ScoreState):
    """Score state that shows the filehandle class"""
    def update(self):
        pass
    def do_show(self, fh):
        fh.write(str(fh.__class__))
        fh.write("; ")
    def get_type_name(self):
        return "ScoreStateTest"
    def get_version_info(self):
        return IMP.get_module_version_info()
    def get_input_particles(self):
        return []
    def get_output_particles(self):
        return []
    def get_input_objects(self):
        return IMP.ObjectsTemp()
    def get_output_objects(self):
        return IMP.ObjectsTemp()



class Tests(IMP.test.TestCase):
    def test_state_show(self):
        """Test score state show method"""
        m = IMP.Model("score state show")
        s = ClassScoreState()
        sio = StringIO.StringIO()
        s.show(sio)
        m.add_score_state(s)
        for s in m.get_score_states():
            s.show(sio)
        # Output should work for a direct call (in which the filehandle is
        # just the Python file-like object) or via a C++ proxy (in which case
        # the filehandle is a std::ostream adapter)
        self.assertGreater(len(sio.getvalue()), 0)

    def test_score_state(self):
        """Check score state methods"""
        m = IMP.Model("score state model")
        #self.assertRaises(IndexError, m.get_score_state,
        #                  0);
        s = DummyScoreState()
        m.add_score_state(s)
        news = m.get_score_state(0)
        self.assertIsInstance(news, IMP.ScoreState)
        #self.assertRaises(IndexError, m.get_score_state,
        #                  1);
        for s in m.get_score_states():
            s.show()

    def test_show(self):
        """Check Model.show() method"""
        class BrokenFile(object):
            def write(self, str):
                raise NotImplementedError()
        m = IMP.Model("model show")
        self.assertRaises(NotImplementedError, m.show, BrokenFile())
        self.assertRaises(AttributeError, m.show, None)
        s = StringIO.StringIO()
        m.show(s)
        self.assertGreater(len(s.getvalue()), 0)

    def test_refcount_director_score_state(self):
        """Refcounting should prevent director ScoreStates from being deleted"""
        dirchk = IMP.test.DirectorObjectChecker(self)
        m = IMP.Model("ref counting score states")
        s = DummyScoreState()
        s.python_member = 'test string'
        m.add_score_state(s)
        # Since C++ now holds a reference to s, it should be safe to delete the
        # Python object (director objects should not be freed while C++ holds
        # a reference)
        del s
        news = m.get_score_state(0)
        self.assertEqual(news.python_member, 'test string')
        # Make sure we kept a reference somewhere to this director object
        dirchk.assert_number(1)
        # Cleanup should touch nothing as long as we have Python reference news
        dirchk.assert_number(1)
        del news
        # Should also touch nothing as long as we have C++ references (from m)
        dirchk.assert_number(1)
        del m
        # No refs remain, so make sure all director objects are cleaned up
        dirchk.assert_number(0)

    def test_director_python_exceptions(self):
        """Check that exceptions raised in directors are handled"""
        no= IMP.base.SetNumberOfThreads(1)
        m = IMP.Model("director exceptions")
        r = FailingRestraint(m)
        m.add_restraint(r)
        self.assertRaises(CustomError, m.evaluate, False)

    def test_restraints(self):
        """Check restraint methods"""
        m = IMP.Model("restraint methods in model")
        #self.assertRaises(IndexError, m.get_restraint, 0);
        self.assertEqual(m.get_number_of_restraints(), 0)
        r = DummyRestraint(m)
        m.add_restraint(r)
        self.assertEqual(m.get_number_of_restraints(), 1)
        newr = m.get_restraint(0)
        self.assertIsInstance(newr, IMP.Restraint)
        #self.assertRaises(IndexError, m.get_restraint,1);
        for s in m.get_restraints():
            s.show()
    def test_temp_restraints(self):
        """Check free restraint methods"""
        m = IMP.Model("free restraint methods")
        #self.assertRaises(IndexError, m.get_restraint, 0);
        self.assertEqual(m.get_number_of_restraints(), 0)
        r = DummyRestraint(m)
        r.set_name("dummy")
        r.set_model(m)
        print r.evaluate(False)
        del r
        m.evaluate(False)

    def test_refcount_director_restraints(self):
        """Refcounting should prevent director Restraints from being deleted"""
        dirchk = IMP.test.DirectorObjectChecker(self)
        m = IMP.Model("ref count dir restraitns")
        r = DummyRestraint(m)
        r.python_member = 'test string'
        m.add_restraint(r)
        # Since C++ now holds a reference to r, it should be safe to delete the
        # Python object (director objects should not be freed while C++ holds
        # a reference)
        del r
        newr = m.get_restraint(0)
        self.assertEqual(newr.python_member, 'test string')
        # Make sure that all director objects are cleaned up
        dirchk.assert_number(1)
        del newr, m
        dirchk.assert_number(0)

    def test_particles(self):
        """Check particle methods"""
        m = IMP.Model("particle methods")
        p = IMP.Particle(m)
        self.assertEqual(m.get_number_of_particles(), 1)
        for s in m.get_particles():
            s.show()

    def _select(self, ps, n):
        ret=[]
        for i in range(0,n):
            ret.append(random.choice(ps))
        return ret
    def test_ranges(self):
        """Test float attribute ranges"""
        m=IMP.Model("float ranges")
        ps=[IMP.Particle(m) for i in range(0,2)]
        for k in [IMP.FloatKey(0), IMP.FloatKey(4), IMP.FloatKey(7)]:
            ps[0].add_attribute(k, k.get_index())
            ps[1].add_attribute(k, k.get_index()+1)
            rg= IMP.kernel._get_range(m, k)
            self.assertEqual(rg[0], k.get_index())
            self.assertEqual(rg[1], k.get_index()+1)
    def test_optimized_particles(self):
        """Test that model returns the right list of optimized particles"""
        m= IMP.Model()
        opt=[]
        for i in range(0, 100):
            pi= m.add_particle(str(i))
            m.add_attribute(IMP.FloatKey(i%8), pi, i)
            if i%3==0:
                m.set_is_optimized(IMP.FloatKey(i%8), pi, True)
                opt.append(pi)
        mopt= m.get_optimized_particles()
        mopti= [IMP.Particle.get_from(p).get_index() for p in mopt]
        mopti.sort()
        self.assertEqual(mopti, opt)
    def test_dependencies(self):
        """Check dependencies with restraints and score states"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        m= IMP.Model("dependencies")
        ps=[IMP.Particle(m) for i in range(0,20)]
        cs=[DummyScoreState(ips=self._select(ps[:5], 2),
                            ops= self._select(ps[5:], 2))
            for i in range(5)]
        for c in cs:
            m.add_score_state(c)
        rs=[DummyRestraint(m, ps=self._select(ps, 4))
            for i in range(5)]
        for r in rs:
            r.set_was_used(True)
        selected= self._select(rs, 4)
        rss= IMP.RestraintSet(selected, 1.0)
        sf= rss.create_scoring_function()

        dg= IMP.get_dependency_graph(m)
        #IMP.base.show_graphviz(dg)

        required=[]
        for r in selected:
            inputs= r.get_inputs()
            for i in inputs:
                for j in cs:
                    if i in j.get_outputs():
                        required.append(j)
        required=list(set(required))
        required.sort()
        print required
        found= sf.get_score_states()
        found.sort()
        self.assertEqual(required, found)
        sf.evaluate(False)
        for s in cs:
            print s.get_name()
            self.assertEqual(s.updated, s in required)
            #IMP.show_graphviz(rdg)


if __name__ == '__main__':
    IMP.test.main()
