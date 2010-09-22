import IMP
import IMP.test
import IMP.core
import IMP.container

class RefCountTests(IMP.test.TestCase):
    """Test refcounting of particles"""
    def test_name(self):
        m= IMP.Model()
        l= IMP.container.ListSingletonContainer(m, "hi")
    def test_refcount_container(self):
        """Check that List containers keep track of changes"""
        m= IMP.Model()
        ps0=IMP.Particles()
        for i in range(0,20):
            ps0.append(IMP.Particle(m))
        ps1= IMP.Particles()
        for i in range(0,10):
            ps1.append(IMP.Particle(m))
        c= IMP.container.ListSingletonContainer(ps0)
        ac= c.get_added_container()
        rc= c.get_removed_container()
        print "start"
        m.evaluate(False)
        self.assertEqual(c.get_removed_container().get_number_of_particles(), 0)
        self.assertEqual(c.get_added_container().get_number_of_particles(), 0)
        print "add"
        m.evaluate(False)
        c.add_particles(ps1)
        self.assertEqual(c.get_added_container().get_number_of_particles(), len(ps1))
        self.assertEqual(c.get_removed_container().get_number_of_particles(), 0)
        print "set"
        m.evaluate(False)
        c.set_particles(ps1)
        self.assertEqual(c.get_added_container().get_number_of_particles(), 0)
        self.assertEqual(c.get_removed_container().get_number_of_particles(), len(ps0))
        c.set_was_used(True)


if __name__ == '__main__':
    IMP.test.main()
