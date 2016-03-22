import IMP
import IMP.test
import IMP.pmi.representation
import IMP.pmi.restraints.crosslinking
import os

class Tests(IMP.test.TestCase):
    def test_cysteine_cross_link(self):
        """Test creation of CysteineCrossLinkRestraint"""
        with open('seq.fasta', 'w') as fh:
            fh.write('>chainA\nA\n>chainB\nE\n')
        with open('expdata.txt', 'w') as fh:
            fh.write('962 alpha 691 beta 1 Epsilon-Intra-Solvent\n')

        m = IMP.Model()
        r = IMP.pmi.representation.Representation(m)
        r.create_component("alpha",color=0.25)
        r.add_component_sequence("alpha","seq.fasta", id="chainA", offs=962)
        r.add_component_beads("alpha",[(962,962)])

        r.create_component("beta",color=0.5)
        r.add_component_sequence("beta","seq.fasta", id="chainB",offs=691)
        r.add_component_beads("beta",[(691,691)])


        xl = IMP.pmi.restraints.crosslinking.CysteineCrossLinkRestraint([r],
                                          filename="expdata.txt", cbeta=True)
        xl.add_to_model()
        os.unlink('seq.fasta')
        os.unlink('expdata.txt')

if __name__ == '__main__':
    IMP.test.main()
