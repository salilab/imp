import IMP
import IMP.test
import IMP.domino
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):

    def test_read_merge_tree(self):

        #protein and model
        #pdbFile = "/trombone1/home/dbarkan/IMP/docking/grb.flex.pdb"
        pdbFile = self.get_input_file_name("grb.flex.pdb")
        outputFile = self.get_tmp_file_name("mergeTreeOut.txt")
        closePairDistance = 4.0
        model = IMP.Model()
        protein = IMP.atom.read_pdb(pdbFile, model, IMP.atom.ATOMPDBSelector())
        for i, p in enumerate(model.get_particles()):
            p.set_name(str(i))
        #close pairs
        atoms = IMP.atom.get_by_type(protein, IMP.atom.ATOM_TYPE)
        cont = IMP.container.ListSingletonContainer(atoms)

        nbl = IMP.container.ClosePairContainer(cont, closePairDistance, 0.0)
        model.update()

        #mapping to get unique list of particles
        namesToParticles = {}
        finalPairs = []

        for [p0, p1] in nbl.get_particle_pairs():

            #filter explicitly particles greater than closePairDistance
            if (IMP.algebra.get_distance(IMP.core.XYZ.decorate_particle(p0).get_coordinates(),
                                         IMP.core.XYZ.decorate_particle(p1).get_coordinates()) < closePairDistance):
                finalPairs.append([p0, p1])
            namesToParticles[p0.get_name()] = p0
            namesToParticles[p1.get_name()] = p1

        #create interaction graph
        ig = IMP.domino.InteractionGraph()
        namesToVertices = {}
        for p in namesToParticles.values():
            namesToVertices[p.get_name()] = ig.add_vertex(p)
        for pair in finalPairs:
            ig.add_edge(namesToVertices[pair[0].get_name()], namesToVertices[pair[1].get_name()])

        #create merge tree
        jt = IMP.domino.get_junction_tree(ig)
        mt = IMP.domino.get_balanced_merge_tree(jt)

        #print first merge tree
        print "begin first merge tree"
        for v in mt.get_vertices():
            print mt.get_vertex_name(v)
        print "end first merge tree"

        #write out first merge tree
        sorted_keys = namesToParticles.keys()
        sorted_keys.sort()
        sortedParticles = [namesToParticles[x] for x in sorted_keys]
        outputFh = open(outputFile, 'w')
        IMP.domino.write_merge_tree(mt, sortedParticles, outputFh)
        outputFh.close()

        #read in second merge tree
        inputFh = open(outputFile, 'r')
        newMergeTree = IMP.domino.read_merge_tree(inputFh, sortedParticles)

        #print second merge tree
        print "begin second merge tree"
        for v in newMergeTree.get_vertices():
            print newMergeTree.get_vertex_name(v)
        print "end second merge tree"
        for v0, v1 in zip(mt.get_vertices(), newMergeTree.get_vertices()):
            self.assertEqual(mt.get_vertex_name(v0), newMergeTree.get_vertex_name(v1))
if __name__ == '__main__':
    IMP.test.main()
