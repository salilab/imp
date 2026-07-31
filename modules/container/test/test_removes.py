import IMP
import IMP.test
import IMP.core


class Tests(IMP.test.TestCase):

    """Tests for all pairs pair container"""

    def test_allp2(self):
        """Check that removal from a list container works and removal from model"""
        m = IMP.Model()
        ps = []
        psr = []
        for i in range(0, 50):
            p = IMP.Particle(m)
            ps.append(p)
            if i % 5 == 0:
                psr.append(p)
        for p in psr:
            ps.remove(p)
        for p in psr:
            m.remove_particle(p.get_index())
        print("bye")
        # make sure cleanup goes without error


if __name__ == '__main__':
    IMP.test.main()
