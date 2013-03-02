import IMP
import IMP.test
import IMP.domino
import IMP.core
import random


class Tests(IMP.test.TestCase):
    def _get_path(self, jt, v0, v1):
        print "path for ", v0, v1
        front =[([], v0)]
        while len(front) >0:
            cur=front[-1]
            front=front[0:-1]
            #print "trying", cur[1], cur[0]
            for v in jt.get_in_neighbors(cur[1]):
                #print "neighbor ", v
                if len(cur[0]) == 0 or not v== cur[0][-1]:
                    if v== v1:
                        path= cur[0]+[cur[1],v]
                        print "path is ", path
                        return path
                    front.append((cur[0]+[cur[1]], v))
        raise ValueError("huh")
    def _check_jt_property(self, jt):
        print "vertices are", jt.get_vertices()
        for v0 in jt.get_vertices():
            ps0= [p for p in jt.get_vertex_name(v0)]
            for v1 in jt.get_vertices():
                if v0 == v1:
                    continue
                #ps1= jt.get_label(v1).get_particles()
                intersection=[x for x in ps0 if x in jt.get_vertex_name(v1)]
                path= self._get_path(jt, v0, v1)
                for v in path:
                    print "node", v
                    set=[x for x in jt.get_vertex_name(v)]
                    print [x.get_name() for x in jt.get_vertex_name(v)]
                    for i in intersection:
                        print i.get_name()
                        self.assertIn(i, set)
    def test_global_min2(self):
        """Testing random junction tree"""
        m= IMP.Model()
        ps=[]
        np=20
        ne=np*3
        ig= IMP.domino.InteractionGraph()
        vs=[]
        for i in range(0,np):
            ps.append(IMP.Particle(m))
            vs.append(ig.add_vertex(ps[-1]))
        for i in range(1, np):
            ig.add_edge(vs[i-1], vs[i])
        for i in range(0, ne):
            p0= random.choice(vs)
            p1= random.choice(vs)
            if p0 != p1 and p1 not in ig.get_in_neighbors(p0):
                ig.add_edge(p0, p1)
        #IMP.show_graphviz(ig)
        cig = IMP.domino.get_triangulated(ig)
        #IMP.show_graphviz(cig)
        cg= IMP.domino.get_clique_graph(cig)
        #IMP.show_graphviz(cg)
        jt= IMP.domino.get_minimum_spanning_tree(cg)
        #IMP.show_graphviz(jt)
        jt2= IMP.domino.get_junction_tree(ig)
        #IMP.show_graphviz(jt2)
        self._check_jt_property(jt)
        self._check_jt_property(jt2)

    def test_global_min0(self):
        """Testing junction tree algorithm"""
        m= IMP.Model()
        ps=[]
        ig= IMP.domino.InteractionGraph()
        for i in range(0,8):
            ps.append(IMP.Particle(m))
            ig.add_vertex(ps[-1])
        ig.add_edge(0,1)
        ig.add_edge(0,2)
        ig.add_edge(0,3)
        ig.add_edge(0,4)
        ig.add_edge(1,6)
        ig.add_edge(1,7)
        ig.add_edge(2,5)
        ig.add_edge(2,7)
        ig.add_edge(3,6)
        ig.add_edge(4,6)
        ig.add_edge(6,7)
        ig.show()
        jt= IMP.domino.get_junction_tree(ig)
        jt.show()
        self._check_jt_property(jt)

if __name__ == '__main__':
    IMP.test.main()
