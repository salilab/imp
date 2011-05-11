import os
import IMP
import IMP.test
import IMP.core
import IMP.algebra

class MSConnectivityTests(IMP.test.TestCase):
    """Class to test MS connectivity restraints"""

    def setUp(self):
        """Build test model."""
        IMP.test.TestCase.setUp(self)

        self.m = IMP.Model()
        self.ps= self.create_particles_in_box(self.m,6)
        self.ds= map(lambda p: IMP.core.XYZ.decorate_particle(p), self.ps)
        idk = IMP.StringKey("id")
        self.ds[0].add_attribute(idk, "A")
        self.ds[1].add_attribute(idk, "A")
        self.ds[2].add_attribute(idk, "B")
        self.ds[3].add_attribute(idk, "B")
        self.ds[4].add_attribute(idk, "C")
        self.ds[5].add_attribute(idk, "D")

        self.T = IMP.core.ExperimentalTree()
        i1 = self.T.add_node("A A B B C")
        i2 = self.T.add_node("A B B C")
        i3 = self.T.add_node("A A B B")
        i4 = self.T.add_node("A B")
        i5 = self.T.add_node("A B B")
        i6 = self.T.add_node("B C")
        i7 = self.T.add_node("A A B")
        i8 = self.T.add_node("A B")
        self.T.connect(i1, i2)
        self.T.connect(i1, i3)
        self.T.connect(i1, i4)
        self.T.connect(i2, i5)
        self.T.connect(i2, i6)
        self.T.connect(i3, i7)
        self.T.connect(i5, i8)

    def test_ms_connectivity_graph(self):
        """Test for the correctness of the graph."""
        IMP.set_log_level(IMP.SILENT)

        self.ds[0].set_coordinates(IMP.algebra.Vector3D(0.0, 0.0, 0.0))
        self.ds[1].set_coordinates(IMP.algebra.Vector3D(1.0, 1.0, 0.0))
        self.ds[2].set_coordinates(IMP.algebra.Vector3D(2.0, 0.0, 0.0))
        self.ds[3].set_coordinates(IMP.algebra.Vector3D(3.0, 0.0, 0.0))
        self.ds[4].set_coordinates(IMP.algebra.Vector3D(4.0, -1.0, 0.0))
        self.ds[5].set_coordinates(IMP.algebra.Vector3D(1000, 1000, 1000))

        # add MS connectivity restraints

        ub = IMP.core.HarmonicUpperBound(1.0, 0.1)
        ss= IMP.core.DistancePairScore(ub)
        r= IMP.core.MSConnectivityRestraint(ss, self.T)
        self.m.add_restraint(r)
        r.add_particles(self.ps)

        idk = IMP.StringKey("id")
        connections = list()
        pps= r.get_connected_pairs()
        for p in pps:
            conn = sorted([p[0].get_value(idk), p[1].get_value(idk)])
            connections.append('%s <-> %s' % (conn[0], conn[1]))
        connections.sort()
        self.assertEqual(', '.join(connections), 'A <-> A, A <-> B, B <-> B, B <-> C', 'Bad connectivity graph')

    def test_ms_connectivity(self):
        """Test for the connectivity restraint."""
        IMP.set_log_level(IMP.SILENT)

        self.randomize_particles(self.m.get_particles(), 50.0)
        self.ds[5].set_coordinates(IMP.algebra.Vector3D(1000, 1000, 1000))

        # add MS connectivity restraints

        ub = IMP.core.HarmonicUpperBound(1.0, 0.1)
        ss= IMP.core.DistancePairScore(ub)
        r= IMP.core.MSConnectivityRestraint(ss, self.T)
        self.m.add_restraint(r)
        r.add_particles(self.ps)

        print 'BEFORE optimization'
        for i in xrange(6):
            for j in xrange(6):
                if j > i:
                    print "distance from %d to %d is %s" % (i, j, IMP.core.get_distance(self.ds[i], self.ds[j]))

        o = IMP.core.ConjugateGradients()
        o.set_threshold(1e-4)
        o.set_model(self.m)
        o.optimize(100)

        print 'AFTER optimization'
        dist = [[0]*6 for x in xrange(6)]
        ok   = [[0]*6 for x in xrange(6)]
        sum  = 0
        for i in xrange(6):
            i_connected = False
            for j in xrange(6):
                if j > i:
                    dist[j][i] = dist[i][j] = IMP.core.get_distance(self.ds[i], self.ds[j])
                    ok[j][i] = ok[i][j] = (dist[i][j] < 1.2)
                    sum += ok[i][j]
                    print "distance from %d to %d is %s" % (i, j, dist[i][j])
                i_connected = i_connected or ok[i][j]
            self.assertTrue(i_connected or i==5, "Point %d is not connected" %(i))

        score= self.m.evaluate(False)
        self.assertLess(score, 10, "Score too high")
        pps= r.get_connected_pairs()
        lscore = 0
        for p in pps:
            lscore = lscore+ss.evaluate(IMP.ParticlePair(p[0], p[1]), None)
        self.assertAlmostEqual(score, lscore, delta=.1)
        self.assertGreaterEqual(sum, 4, "Wrong number of close pairs")

if __name__ == '__main__':
    IMP.test.main()
