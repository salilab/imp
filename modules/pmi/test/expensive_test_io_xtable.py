from __future__ import print_function
import IMP
import os
import IMP.test
import IMP.core
import IMP.container
import IMP.pmi
import IMP.pmi.representation
import IMP.pmi.io
import IMP.pmi.io.crosslink
import IMP.pmi.output
try:
    import matplotlib
except ImportError:
    matplotlib = None
if matplotlib is not None:
    import IMP.pmi.io.xltable


class Tests(IMP.test.TestCase):

    def init_crosslink_db(self):
        cldbkc=IMP.pmi.io.crosslink.CrossLinkDataBaseKeywordsConverter()
        cldbkc.set_protein1_key("pep1.accession")
        cldbkc.set_protein2_key("pep2.accession")
        cldbkc.set_residue1_key("pep1.xlinked_aa")
        cldbkc.set_residue2_key("pep2.xlinked_aa")
        cldb=IMP.pmi.io.crosslink.CrossLinkDataBase(cldbkc)
        cldb.create_set_from_file(IMP.pmi.get_data_path("polii_xlinks.csv"))
        return cldb

    def init_representation_complex(self):
        m = IMP.Model()
        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")
        components = ["Rpb1","Rpb2","Rpb3","Rpb4"]
        chains = "ABCD"
        colors = [0.,0.1,0.5,1.0]
        beadsize = 20
        fastids = IMP.pmi.tools.get_ids_from_fasta_file(fastafile)

        r = IMP.pmi.representation.Representation(m)
        hierarchies = {}
        for n in range(len(components)):
            r.create_component(components[n], color=colors[n])
            r.add_component_sequence(components[n], fastafile,
                                     id="1WCM:"+chains[n])
            hierarchies[components[n]] = r.autobuild_model(
                components[n], pdbfile, chains[n],
                resolutions=[1, 10, 100], missingbeadsize=beadsize)
            r.setup_component_sequence_connectivity(components[n], 1)

        o=IMP.pmi.output.Output()
        o.init_rmf("expensive_test_io_xtable.rmf",[r.prot])
        o.write_rmf("expensive_test_io_xtable.rmf")
        o.close_rmf("expensive_test_io_xtable.rmf")

    def test_init_table(self):
        if matplotlib is None:
            self.skipTest("no matplotlib module")
        cldb=self.init_crosslink_db()
        self.init_representation_complex()
        xlt=IMP.pmi.io.xltable.XLTable(35)
        prots = ["Rpb1","Rpb2","Rpb3","Rpb4"]
        chains = "ABCD"
        for n,prot in enumerate(prots):
            xlt.load_sequence_from_fasta_file(self.get_input_file_name("1WCM.fasta.txt"),
                                  id_in_fasta_file="1WCM:"+chains[n],
                                  protein_name=prot)
        xlt.load_rmf_coordinates("expensive_test_io_xtable.rmf",0,prots)
        xlt.load_crosslinks(cldb)
        xlt.setup_contact_map()
        xlt.load_pdb_coordinates(self.get_input_file_name('1WCM.pdb'),{'A':'Rpb1'})
        xlt.plot_table(prot_listx=prots,
           prot_listy=prots,
           alphablend=0.4,
           scale_symbol_size=1.1,
           gap_between_components=50,
           filename="XL_table.pdf",
           contactmap=True,
           crosslink_threshold=35.0,
           display_residue_pairs=False)

if __name__ == '__main__':
    IMP.test.main()
