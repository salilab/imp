import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.algebra

class Tests(IMP.test.TestCase):
    """Test the CHARMMAtom decorator"""

    def test_create(self):
        """Check creation of CHARMMAtom decorators"""
        m = IMP.Model()
        p = IMP.Particle(m)
        IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)

        IMP.atom.CHARMMAtom.setup_particle(p, 'CT1')
        p = IMP.Particle(m)
        if IMP.base.get_check_level() != IMP.base.NONE:
            self.assertRaises(IMP.base.UsageException,
                              IMP.atom.CHARMMAtom.setup_particle, p, 'CT1')

    def test_get_set(self):
        """Check get/set methods of CHARMMAtom decorators"""
        m = IMP.Model()
        p = IMP.Particle(m)
        IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)
        c = IMP.atom.CHARMMAtom.setup_particle(p, 'CT1')

        self.assertEqual(c.get_charmm_type(), 'CT1')
        c.set_charmm_type('CT3')
        self.assertEqual(c.get_charmm_type(), 'CT3')

    def test_show(self):
        """Check show method of CHARMMAtom decorators"""
        m = IMP.Model()
        p = IMP.Particle(m)
        IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)
        c = IMP.atom.CHARMMAtom.setup_particle(p, 'CT1')

        self.assertIn(' CHARMM type= ', str(c))

if __name__ == '__main__':
    IMP.test.main()
