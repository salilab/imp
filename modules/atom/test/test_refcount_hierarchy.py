import IMP
import IMP.test
import IMP.core
import IMP.atom

class DecoratorTests(IMP.test.TestCase):
    def test_simple(self):
        """Check that hierarchies don't have circular ref counts """
        # make sure internal things are created
        m= IMP.Model()
        h= IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        del m
        del h
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model()
        h= IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        hc= IMP.atom.Hierarchy.setup_particle(IMP.Particle(m))
        h.add_child(hc)
        del m
        del hc
        del h
        refcnt.assert_number(0)
    def test_simple_bond(self):
        """Check that bonded don't have circular ref counts """
        m= IMP.Model()
        h= IMP.atom.Bonded.setup_particle(IMP.Particle(m))
        del m
        del h
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model()
        h= IMP.atom.Bonded.setup_particle(IMP.Particle(m))
        hc= IMP.atom.Bonded.setup_particle(IMP.Particle(m))
        IMP.atom.create_bond(h, hc, 0)
        del m
        del hc
        del h
        refcnt.assert_number(0)
    def test_bonded(self):
        """Check that pdbs don't have circular ref counts """
        # charm creates all sorts of things
        m= IMP.Model()
        h= IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        del m
        del h
        IMP.set_log_level(IMP.MEMORY)
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model()
        h= IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        del m
        del h
        # not sure what the 2 are
        refcnt.assert_number(2)

if __name__ == '__main__':
    IMP.test.main()
