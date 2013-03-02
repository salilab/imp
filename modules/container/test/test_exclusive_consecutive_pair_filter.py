import IMP
import IMP.test
import IMP.container
import IMP.display
import math

class Tests(IMP.test.TestCase):
    """Tests for tunnel scores"""
    def test_connectivity_zero_set(self):
        """Test exclusive consecutive pair filter"""
        m= IMP.Model()
        ps0=[IMP.Particle(m) for i in range(0,15)]
        cpc0= IMP.container.ExclusiveConsecutivePairContainer(ps0)
        ps1=[IMP.Particle(m) for i in range(0,15)]
        cpc1= IMP.container.ExclusiveConsecutivePairContainer(ps1)
        cpc0.set_was_used(True)
        cpc1.set_was_used(True)
        flt= IMP.container.ExclusiveConsecutivePairFilter()
        flt.set_was_used(True)
        self.assertEqual(flt.get_value((ps1[0], ps1[1])), 1)
        self.assertEqual(flt.get_value((ps0[0], ps1[1])), 0)



if __name__ == '__main__':
    IMP.test.main()
