import IMP
import IMP.test
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_cp(self):
        """Testing create_protein"""
        m = IMP.Model()
        p = IMP.atom.create_protein(m, "hi", 10.0, 150)
        self.assertEqual(len(p.get_children()), 7)


if __name__ == '__main__':
    IMP.test.main()
