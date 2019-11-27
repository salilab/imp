import IMP
import IMP.test
import IMP.pmi.topology
import IMP.pmi.restraints.crosslinking
import os

class Tests(IMP.test.TestCase):
    def test_cysteine_cross_link(self):
        """Test creation of CysteineCrossLinkRestraint"""
        with open('expdata.txt', 'w') as fh:
            fh.write('2 alpha 2 beta 1 Epsilon-Intra-Solvent\n')

        m = IMP.Model()
        simo = IMP.pmi.topology.System(m)
        st1 = simo.create_state()
        alpha = st1.create_molecule("alpha", sequence="AA")
        alpha.add_representation(alpha, resolutions=[1], color=0.25)
        beta = st1.create_molecule("beta", sequence="EE")
        beta.add_representation(beta, resolutions=[1], color=0.5)
        hier = simo.build()

        xl = IMP.pmi.restraints.crosslinking.CysteineCrossLinkRestraint(hier,
                                          filename="expdata.txt", cbeta=True)
        xl.add_to_model()
        os.unlink('expdata.txt')

if __name__ == '__main__':
    IMP.test.main()
