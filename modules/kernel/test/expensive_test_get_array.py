import IMP.test


class Tests(IMP.test.TestCase):

    def test_get_array(self):
        """Test std::get<IMP::Array>"""
        # Should be a compile-time error to access an out of range element
        self.assertCompileFails(
            includes=['IMP/base_types.h'],
            body="""
IMP::ParticleIndexPair pp(IMP::ParticleIndex(1), IMP::ParticleIndex(3));
IMP::ParticleIndex p = std::get<2>(pp);""")


if __name__ == '__main__':
    IMP.test.main()
