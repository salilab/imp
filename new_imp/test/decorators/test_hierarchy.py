import unittest
import IMP
import IMP.test

class DecoratorTests(IMP.test.IMPTestCase):
    def test_hierarchy(self):
        """Check that the hierarchy works fine"""
        m = IMP.Model()
        pp=IMP.Particle()
        m.add_particle(pp)
        pp.thisown=0
        ppd= IMP.HierarchyDecorator.cast(pp)
        ppnd= IMP.NameDecorator.create(pp)
        ppnd.set_name("root");
        for i in range(0,10):
            pc=IMP.Particle()
            pc.thisown=0
            m.add_particle(pc)
            ppd.add_child(pc)
            pcnd= IMP.NameDecorator.create(pc)
            pcnd.set_name(str("child "+str(i)));

        self.assertEqual(ppd.get_number_of_children(), 10,
                         "Number of children is not as expected")
        IMP.assert_hierarchy(pp)


if __name__ == '__main__':
    unittest.main()
