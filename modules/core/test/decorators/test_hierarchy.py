import unittest
import IMP
import IMP.test
import IMP.core

class DecoratorTests(IMP.test.TestCase):
    def test_hierarchy(self):
        """Check that the hierarchy works fine"""
        m = IMP.Model()
        pp=IMP.Particle()
        m.add_particle(pp)
        ppd= IMP.core.HierarchyDecorator.create(pp)
        ppnd= IMP.core.NameDecorator.create(pp)
        ppnd.set_name("root");
        for i in range(0,10):
            pc=IMP.Particle()
            m.add_particle(pc)
            pcd= IMP.core.HierarchyDecorator.create(pc)
            ppd.add_child(pcd)
            pcnd= IMP.core.NameDecorator.create(pc)
            pcnd.set_name(str("child "+str(i)));

        self.assertEqual(ppd.get_number_of_children(), 10,
                         "Number of children is not as expected")
        ppd.validate()


if __name__ == '__main__':
    unittest.main()
