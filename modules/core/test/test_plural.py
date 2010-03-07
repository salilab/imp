import unittest
import IMP.core
import IMP.test

plural_exceptions=['MoverBase', 'RigidBody', 'Hierarchy', 'HierarchyCounter', 'HierarchyVisitor', 'ModifierVisitor']

show_exceptions=['HierarchyCounter', 'HierarchyVisitor', 'ModifierVisitor']

class DirectoriesTests(IMP.test.TestCase):

    def test_data_directory(self):
        """Test all core objects have plural version"""
        return self.assertPlural(IMP.core, plural_exceptions)

    def test_2(self):
        """Test all core objects have show"""
        return self.assertShow(IMP.core, show_exceptions)

if __name__ == '__main__':
    unittest.main()
