import IMP
import IMP.test
import IMP.pmi.representation as representation
import IMP.pmi.tools as tools

class Tests(IMP.test.TestCase):

    def init_representation_complex(self, m):
        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")
        components = ["Rpb3","Rpb4","Rpb5","Rpb6"]
        chains = "CDEF"
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

        return r

    def test_get_terminal_residue_two_methods(self):
        m = IMP.Model()
        rcomplex=self.init_representation_complex(m)


        results={("Rpb4","cter"):221,
        ("Rpb4","nter"):4,
        ("Rpb5","cter"):215,
        ("Rpb5","nter"):1,
        ("Rpb6","cter"):155,
        ("Rpb6","nter"):72,
        ("Rpb3","cter"):268,
        ("Rpb3","nter"):3}

        for name in ["Rpb3","Rpb4","Rpb5","Rpb6"]:
            cter=IMP.pmi.tools.get_terminal_residue(rcomplex, rcomplex.hier_dict[name], terminus="C", resolution=1)
            self.assertEqual(results[(name,"cter")],IMP.atom.Residue(cter).get_index())
            nter=IMP.pmi.tools.get_terminal_residue(rcomplex, rcomplex.hier_dict[name], terminus="N", resolution=1)
            self.assertEqual(results[(name,"nter")],IMP.atom.Residue(nter).get_index())

        results={("Rpb4","cter"):(IMP.atom.Residue,221),
        ("Rpb4","nter"):(IMP.atom.Fragment,list(range(1,4))),
        ("Rpb5","cter"):(IMP.atom.Residue,215),
        ("Rpb5","nter"):(IMP.atom.Residue,1),
        ("Rpb6","cter"):(IMP.atom.Residue,155),
        ("Rpb6","nter"):(IMP.atom.Fragment,list(range(1,21))),
        ("Rpb3","cter"):(IMP.atom.Fragment,list(range(309, 319))),
        ("Rpb3","nter"):(IMP.atom.Fragment,list(range(1, 3)))}

        for name in ["Rpb3","Rpb4","Rpb5","Rpb6"]:
            all=IMP.pmi.tools.select_by_tuple(rcomplex,name,resolution=1)
            nter=all[0]
            cter=all[-1]
            result=results[(name,"nter")]
            if result[0] is IMP.atom.Residue:
                self.assertEqual(result[1],IMP.atom.Residue(nter).get_index())
            if result[0] is IMP.atom.Fragment:
                self.assertEqual(result[1],IMP.atom.Fragment(nter).get_residue_indexes())
            result=results[(name,"cter")]
            if result[0] is IMP.atom.Residue:
                self.assertEqual(result[1],IMP.atom.Residue(cter).get_index())
            if result[0] is IMP.atom.Fragment:
                self.assertEqual(result[1],IMP.atom.Fragment(cter).get_residue_indexes())

    def test_selection(self):
        """Test selection"""
        # input parameter
        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")

        components = ["Rpb3", "Rpb3.copy", "Rpb4"]
        chains = "CCD"
        colors = [0., 0.5, 1.0]
        beadsize = 20
        fastids = ['1WCM:C','1WCM:C','1WCM:D']

        m = IMP.Model()
        simo = representation.Representation(m)

        hierarchies = {}

        for n in range(len(components)):
            simo.create_component(components[n], color=colors[n])
            simo.add_component_sequence(components[n], fastafile,
                                        id=fastids[n])
            hierarchies[components[n]] \
               = simo.autobuild_model(
                         components[n], pdbfile, chains[n],
                         resolutions=[1, 10, 100], missingbeadsize=beadsize)
            simo.setup_component_sequence_connectivity(components[n], 1)

        def test(a, b):
            assert a==b, "%d != %d" % (a,b)

        result_dict = {
            "All": 803,
            "resolution=1": 721,
            "resolution=1,resid=10": 3,
            "resolution=1,resid=10,name=Rpb3": 1,
            "resolution=1,resid=10,name=Rpb3,ambiguous": 2,
            "resolution=1,resid=10,name=Rpb4,ambiguous": 1,
            "resolution=1,resrange=(10,20),name=Rpb3": 11,
            "resolution=1,resrange=(10,20),name=Rpb3,ambiguous": 22,
            "resolution=10,resrange=(10,20),name=Rpb3": 2,
            "resolution=10,resrange=(10,20),name=Rpb3,ambiguous": 4,
            "resolution=100,resrange=(10,20),name=Rpb3": 1,
            "resolution=100,resrange=(10,20),name=Rpb3,ambiguous": 2,
            "Rpb4_1-3_bead resolution=1": 1,
            "Rpb4_1-3_bead resolution=10": 1,
            "Rpb4_1-3_bead resolution=100": 1,
            "Rpb4_4-76_pdb resolution=1": 73,
            "Rpb4_4-76_pdb resolution=10": 0,
            "Rpb4_4-76_pdb resolution=100": 0,
            "Rpb4_4-76_pdb#2 resolution=1": 0,
            "Rpb4_4-76_pdb#2 resolution=10": 8,
            "Rpb4_4-76_pdb#2 resolution=100": 0,
            "Rpb4_4-76_pdb#3 resolution=1": 0,
            "Rpb4_4-76_pdb#3 resolution=10": 0,
            "Rpb4_4-76_pdb#3 resolution=100": 1,
            "Rpb4_77-96_bead resolution=1": 1,
            "Rpb4_77-96_bead resolution=10": 1,
            "Rpb4_77-96_bead resolution=100": 1,
            "Rpb4_97-116_bead resolution=1": 1,
            "Rpb4_97-116_bead resolution=10": 1,
            "Rpb4_97-116_bead resolution=100": 1,
            "Rpb4_117_bead resolution=1": 1,
            "Rpb4_117_bead resolution=10": 1,
            "Rpb4_117_bead resolution=100": 1,
            "Rpb4_118-221_pdb resolution=1": 104,
            "Rpb4_118-221_pdb resolution=10": 0,
            "Rpb4_118-221_pdb resolution=100": 0,
            "Rpb4_118-221_pdb#2 resolution=1": 0,
            "Rpb4_118-221_pdb#2 resolution=10": 11,
            "Rpb4_118-221_pdb#2 resolution=100": 0,
            "Rpb4_118-221_pdb#3 resolution=1": 0,
            "Rpb4_118-221_pdb#3 resolution=10": 0,
            "Rpb4_118-221_pdb#3 resolution=100": 2,
            "Rpb3.copy_1-2_bead resolution=1": 1,
            "Rpb3.copy_1-2_bead resolution=1": 1,
            "Rpb3.copy_1-2_bead resolution=10": 1,
            "Rpb3.copy_1-2_bead resolution=100": 1,
            "Rpb3.copy_3-268_pdb resolution=1": 266,
            "Rpb3.copy_3-268_pdb resolution=10": 0,
            "Rpb3.copy_3-268_pdb resolution=100": 0,
            "Rpb3.copy_3-268_pdb#2 resolution=1": 0,
            "Rpb3.copy_3-268_pdb#2 resolution=10": 27,
            "Rpb3.copy_3-268_pdb#2 resolution=100": 0,
            "Rpb3.copy_3-268_pdb#3 resolution=1": 0,
            "Rpb3.copy_3-268_pdb#3 resolution=10": 0,
            "Rpb3.copy_3-268_pdb#3 resolution=100": 3,
            "Rpb3.copy_269-288_bead resolution=1": 1,
            "Rpb3.copy_269-288_bead resolution=10": 1,
            "Rpb3.copy_269-288_bead resolution=100": 1,
            "Rpb3.copy_289-308_bead resolution=1": 1,
            "Rpb3.copy_289-308_bead resolution=10": 1,
            "Rpb3.copy_289-308_bead resolution=100": 1,
            "Rpb3.copy_309-318_bead resolution=1": 1,
            "Rpb3.copy_309-318_bead resolution=10": 1,
            "Rpb3.copy_309-318_bead resolution=100": 1,
            "Rpb3_1-2_bead resolution=1": 1,
            "Rpb3_1-2_bead resolution=10": 1,
            "Rpb3_1-2_bead resolution=100": 1,
            "Rpb3_3-268_pdb resolution=1": 266,
            "Rpb3_3-268_pdb resolution=10": 0,
            "Rpb3_3-268_pdb resolution=100": 0,
            "Rpb3_3-268_pdb#2 resolution=1": 0,
            "Rpb3_3-268_pdb#2 resolution=10": 27,
            "Rpb3_3-268_pdb#2 resolution=100": 0,
            "Rpb3_3-268_pdb#3 resolution=1": 0,
            "Rpb3_3-268_pdb#3 resolution=10": 0,
            "Rpb3_3-268_pdb#3 resolution=100": 3,
            "Rpb3_269-288_bead resolution=1": 1,
            "Rpb3_269-288_bead resolution=10": 1,
            "Rpb3_269-288_bead resolution=100": 1,
            "Rpb3_289-308_bead resolution=1": 1,
            "Rpb3_289-308_bead resolution=10": 1,
            "Rpb3_289-308_bead resolution=100": 1,
            "Rpb3_309-318_bead resolution=1": 1,
            "Rpb3_309-318_bead resolution=10": 1,
            "Rpb3_309-318_bead resolution=100": 1,
            "Beads": 12,
            "Molecule": 803,
            "resolution=1,Molecule": 721,
            "resolution=10,Molecule": 85,
            "resolution=100,Molecule": 21,
            "resolution=1,Beads": 12,
            "resolution=10,Beads": 12,
            "resolution=100,Beads": 12,
            "resolution=2": 721,
            "resolution=7": 85,
            "resolution=10": 85,
            "resolution=100": 21}

        test(result_dict["All"], len(tools.select(simo)))
        test(result_dict["resolution=1"], len(tools.select(simo, resolution=1)))
        test(result_dict["resolution=1,resid=10"],
             len(tools.select(simo, resolution=1, residue=10)))
        test(result_dict["resolution=1,resid=10,name=Rpb3"],
             len(tools.select(simo, resolution=1, name="Rpb3", residue=10)))
        test(result_dict["resolution=1,resid=10,name=Rpb3,ambiguous"],
             len(tools.select(simo, resolution=1, name="Rpb3",
                              name_is_ambiguous=True, residue=10)))
        test(result_dict["resolution=1,resid=10,name=Rpb4,ambiguous"],
             len(tools.select(simo, resolution=1, name="Rpb4",
                              name_is_ambiguous=True, residue=10)))
        test(result_dict["resolution=1,resrange=(10,20),name=Rpb3"],
             len(tools.select(simo, resolution=1, name="Rpb3",
                              first_residue=10, last_residue=20)))
        test(result_dict["resolution=1,resrange=(10,20),name=Rpb3,ambiguous"],
             len(tools.select(simo, resolution=1, name="Rpb3",
                              name_is_ambiguous=True, first_residue=10,
                              last_residue=20)))
        test(result_dict["resolution=10,resrange=(10,20),name=Rpb3"],
             len(tools.select(simo, resolution=10, name="Rpb3",
                              first_residue=10, last_residue=20)))
        test(result_dict["resolution=10,resrange=(10,20),name=Rpb3,ambiguous"],
             len(tools.select(simo, resolution=10, name="Rpb3",
                              name_is_ambiguous=True, first_residue=10,
                              last_residue=20)))
        test(result_dict["resolution=100,resrange=(10,20),name=Rpb3"],
             len(tools.select(simo, resolution=100, name="Rpb3",
                              first_residue=10, last_residue=20)))
        test(result_dict["resolution=100,resrange=(10,20),name=Rpb3,ambiguous"],
             len(tools.select(simo, resolution=100, name="Rpb3",
                              name_is_ambiguous=True, first_residue=10,
                              last_residue=20)))

        for key in hierarchies:
            seen = {}
            for h in hierarchies[key]:
                # Handle duplicate names
                if h.get_name() in seen:
                    name = h.get_name() + "#%d" % seen[h.get_name()]
                    seen[h.get_name()] += 1
                else:
                    name = h.get_name()
                    seen[h.get_name()] = 2

                test(result_dict[name + " resolution=1"],
                     len(tools.select(simo, resolution=1, hierarchies=[h])))
                test(result_dict[name + " resolution=10"],
                     len(tools.select(simo, resolution=10, hierarchies=[h])))
                test(result_dict[name + " resolution=100"],
                     len(tools.select(simo, resolution=100, hierarchies=[h])))

        test(result_dict["Beads"],
             len(tools.select(simo, representation_type="Beads")))
        test(result_dict["Molecule"],
             len(tools.select(simo, representation_type="Molecule")))
        test(result_dict["resolution=1,Molecule"],
             len(tools.select(simo, resolution=1,
                              representation_type="Molecule")))
        test(result_dict["resolution=10,Molecule"],
             len(tools.select(simo, resolution=10,
                              representation_type="Molecule")))
        test(result_dict["resolution=100,Molecule"],
             len(tools.select(simo, resolution=100,
                              representation_type="Molecule")))
        test(result_dict["resolution=1,Beads"],
             len(tools.select(simo, resolution=1, representation_type="Beads")))
        test(result_dict["resolution=10,Beads"],
             len(tools.select(simo, resolution=10,
                              representation_type="Beads")))
        test(result_dict["resolution=100,Beads"],
             len(tools.select(simo, resolution=100,
                              representation_type="Beads")))
        test(result_dict["resolution=2"], len(tools.select(simo, resolution=2)))

        test(result_dict["resolution=7"],
             len(tools.select(simo, resolution=7)))
        test(result_dict["resolution=10"],
             len(tools.select(simo, resolution=10)))
        test(result_dict["resolution=100"],
             len(tools.select(simo, resolution=100)))


if __name__ == '__main__':
    IMP.test.main()
