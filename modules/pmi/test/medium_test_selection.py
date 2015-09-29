import IMP
import IMP.test
import IMP.pmi.representation as representation
import IMP.pmi.tools as tools

class Tests(IMP.test.TestCase):
    def test_selection(self):
        """Test selection"""
        # input parameter
        pdbfile = self.get_input_file_name("1WCM.pdb")
        fastafile = self.get_input_file_name("1WCM.fasta.txt")

        components = ["Rpb3", "Rpb3.copy", "Rpb4"]
        chains = "CCD"
        colors = [0., 0.5, 1.0]
        beadsize = 20
        fastids = tools.get_ids_from_fasta_file(fastafile)

        m = IMP.Model()
        simo = representation.Representation(m)

        hierarchies = {}

        for n in range(len(components)):
            simo.create_component(components[n], color=colors[n])
            simo.add_component_sequence(components[n], fastafile,
                                        id=fastids[n + 2])
            hierarchies[components[n]] \
               = simo.autobuild_model(
                         components[n], pdbfile, chains[n],
                         resolutions=[1, 10, 100], missingbeadsize=beadsize)
            simo.setup_component_sequence_connectivity(components[n], 1)

        def test(a, b):
            assert a==b, "%d != %d" % (a,b)

        result_dict = {
            "All": 691,
            "resolution=1": 621,
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
            "Rpb4_118-215_pdb resolution=1": 98,
            "Rpb4_118-215_pdb resolution=10": 0,
            "Rpb4_118-215_pdb resolution=100": 0,
            "Rpb4_118-215_pdb#2 resolution=1": 0,
            "Rpb4_118-215_pdb#2 resolution=10": 10,
            "Rpb4_118-215_pdb#2 resolution=100": 0,
            "Rpb4_118-215_pdb#3 resolution=1": 0,
            "Rpb4_118-215_pdb#3 resolution=10": 0,
            "Rpb4_118-215_pdb#3 resolution=100": 1,
            "Rpb3.copy_1-2_bead resolution=1": 1,
            "Rpb3.copy_1-2_bead resolution=10": 1,
            "Rpb3.copy_1-2_bead resolution=100": 1,
            "Rpb3.copy_3-177_pdb resolution=1": 175,
            "Rpb3.copy_3-177_pdb resolution=10": 0,
            "Rpb3.copy_3-177_pdb resolution=100": 0,
            "Rpb3.copy_3-177_pdb#2 resolution=1": 0,
            "Rpb3.copy_3-177_pdb#2 resolution=10": 18,
            "Rpb3.copy_3-177_pdb#2 resolution=100": 0,
            "Rpb3.copy_3-177_pdb#3 resolution=1": 0,
            "Rpb3.copy_3-177_pdb#3 resolution=10": 0,
            "Rpb3.copy_3-177_pdb#3 resolution=100": 2,
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
            "Beads": 9,
            "Molecule": 691,
            "resolution=1,Molecule": 621,
            "resolution=10,Molecule": 72,
            "resolution=100,Molecule": 16,
            "resolution=1,Beads": 9,
            "resolution=10,Beads": 9,
            "resolution=100,Beads": 9,
            "resolution=2": 621,
            "resolution=7": 72,
            "resolution=10": 72,
            "resolution=100": 16}

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
