import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    def _test_simple(self):
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
    def _test_simple_bond(self):
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
    def _test_bonded(self):
        """Check that pdbs don't have circular ref counts """
        # charm creates all sorts of things
        m= IMP.Model()
        h= IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        del m
        del h
        IMP.base.set_log_level(IMP.MEMORY)
        refcnt = IMP.test.RefCountChecker(self)
        m= IMP.Model()
        print "reading"
        h= IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        del m
        del h
        refcnt.assert_number(0)

    def test_rbbonded(self):
        """Check that pdbs with rigid bodies don't have circular ref counts """
        m= IMP.Model()
        h= IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        rb= IMP.atom.create_rigid_body([h], "test rb");
        #print [x.get_name() for x in rb.get_members()]
        del rb
        del m
        del h
        print "initial live"
        while (True):
            # charm creates all sorts of things
            refcnt = IMP.test.RefCountChecker(self)
            m= IMP.Model()
            p = IMP.Particle(m)
            p.set_name("TEST")
            del p
            h= IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
            #IMP.atom.show_molecular_hierarchy(h)
            rb= IMP.atom.create_rigid_body([h], "test rb");
            #print [x.get_name() for x in rb.get_members()]
            del rb
            #del x
            del m
            del h
            print dir()
            #print "live"
            #print [x  for x in IMP.Object.get_live_object_names() if (x.find("CHARMM")==-1)]
            refcnt.assert_number(0)
            #except:
            #    pass
            #print "wait"
            #for i in range(0,60000000):
            #    pass
            #print "end wait"
            return



if __name__ == '__main__':
    IMP.test.main()
