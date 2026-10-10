import IMP
import IMP.test
try:
    import jax
    import IMP._jax_util
except ImportError:
    jax = None


class Tests(IMP.test.TestCase):

    @IMP.test.skipIf(jax is None, "No JAX support")
    def test_get_grouped_restraints(self):
        """Test get_grouped_restraints() function"""
        rs = ["rsr:B:42", "rsr:A:4", "rsr:A:2", "other:A:2", "other:A:4"]

        def keyfunc(r):
            if r.startswith("rsr"):
                return r.split(":")[1]

        groups, ungrouped = IMP._jax_util.get_grouped_restraints(rs, keyfunc)
        self.assertEqual(groups, [['rsr:B:42'], ['rsr:A:4', 'rsr:A:2']])
        self.assertEqual(ungrouped, ['other:A:2', 'other:A:4'])


if __name__ == '__main__':
    IMP.test.main()
