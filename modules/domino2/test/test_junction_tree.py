import sys
import unittest
import IMP
import IMP.test
import IMP.domino2
import IMP.core
import IMP.atom
import IMP.helper
import time
import random


class DOMINOTests(IMP.test.TestCase):
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
            ps0= jt.get_label(v0).get_particles()
            for v1 in jt.get_vertices():
                if v0 == v1:
                    continue
                ps1= jt.get_label(v1).get_particles()
                intersection=[x for x in ps0 if x in ps1]
                path= self._get_path(jt, v0, v1)
                for v in path:
                    print "node", v
                    set= jt.get_label(v).get_particles()
                    print [x.get_name() for x in set]
                    for i in intersection:
                        print i.get_name()
                        self.assert_(i in set)
    def _test_global_min2(self):
        """Testing random junction tree"""
        m= IMP.Model()
        ps=[]
        np=40
        ne=40
        ig= IMP.domino2.InteractionGraph()
        vs={}
        for i in range(0,np):
            ps.append(IMP.Particle(m))
            vs[ps[-1]]=ig.add_vertex(ps[-1])
        for i in range(1, np):
            ig.add_edge(vs[ps[i-1]], vs[ps[i]]);
        for i in range(0, ne):
            p0= random.choice(ps)
            p1= random.choice(ps)
            if p0 != p1:
                ig.add_edge(vs[p0], vs[p1])
        ig.show()
        jt= IMP.domino2.get_junction_tree(ig)
        jt.show()
        self._check_jt_property(jt)

    def test_global_min0(self):
        """Testing junction tree algorit"""
        m= IMP.Model()
        ps=[]
        ig= IMP.domino2.InteractionGraph()
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
        jt= IMP.domino2.get_junction_tree(ig)
        jt.show()
        self._check_jt_property(jt)

if __name__ == '__main__':
    unittest.main()
