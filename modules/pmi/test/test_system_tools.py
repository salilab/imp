from __future__ import print_function
import IMP.test
from IMP.pmi.topology.system_tools import _get_color_for_representation


class Tests(IMP.test.TestCase):

    def test_get_color_for_representation(self):
        """Test _get_color_for_representation()"""
        class MockRepresentation(object):
            def __init__(self, c):
                self.color = c

        c = _get_color_for_representation(MockRepresentation(0.1))
        self.assertAlmostEqual(c.get_red(), 0.0, delta=0.01)
        self.assertAlmostEqual(c.get_green(), 0.2, delta=0.01)
        self.assertAlmostEqual(c.get_blue(), 0.8, delta=0.01)

        c = _get_color_for_representation(MockRepresentation("red"))
        self.assertAlmostEqual(c.get_red(), 1.0, delta=0.01)
        self.assertAlmostEqual(c.get_green(), 0.0, delta=0.01)
        self.assertAlmostEqual(c.get_blue(), 0.0, delta=0.01)

        c = _get_color_for_representation(MockRepresentation("#fa8072"))
        self.assertAlmostEqual(c.get_red(), 0.980, delta=0.01)
        self.assertAlmostEqual(c.get_green(), 0.502, delta=0.01)
        self.assertAlmostEqual(c.get_blue(), 0.447, delta=0.01)

        c = _get_color_for_representation(MockRepresentation((0.1, 0.2, 0.3)))
        self.assertAlmostEqual(c.get_red(), 0.1, delta=0.01)
        self.assertAlmostEqual(c.get_green(), 0.2, delta=0.01)
        self.assertAlmostEqual(c.get_blue(), 0.3, delta=0.01)

        d = IMP.display.Color(0.2, 0.3, 0.4)
        c = _get_color_for_representation(MockRepresentation(d))
        self.assertAlmostEqual(c.get_red(), 0.2, delta=0.01)
        self.assertAlmostEqual(c.get_green(), 0.3, delta=0.01)
        self.assertAlmostEqual(c.get_blue(), 0.4, delta=0.01)

        self.assertRaises(TypeError,
                          _get_color_for_representation, MockRepresentation(42))

        r = MockRepresentation(None)
        self.assertIsNone(_get_color_for_representation(r))



if __name__ == '__main__':
    IMP.test.main()
