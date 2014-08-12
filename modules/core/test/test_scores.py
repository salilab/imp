import IMP
import IMP.atom
import IMP.algebra
import itertools
import IMP.test

class TestEMRestraint(IMP.test.TestCase):
    def test_some_scores(self):
        pdb_fn=self.get_input_file_name('helix2.pdb')

        mdl=IMP.Model()
        mh=IMP.atom.read_pdb(pdb_fn,mdl,IMP.atom.NonWaterPDBSelector())
        ps=IMP.atom.Selection(mh,atom_type=IMP.atom.AtomType("CA")).get_selected_particles()
        strength=10.0
        rs = IMP.RestraintSet(mdl, "ElasticNetwork")
        for pidx in itertools.combinations(xrange(len(ps)),2):
            pair=[ps[pidx[0]],ps[pidx[1]]]
            distance=IMP.algebra.get_distance(IMP.core.XYZ(pair[0]).get_coordinates(),
                                              IMP.core.XYZ(pair[1]).get_coordinates())
            ts=IMP.core.Harmonic(distance,strength)
            rs.add_restraint(IMP.core.DistanceRestraint(ts,pair[0],pair[1]))
        mdl.add_restraint(rs)
        print rs.evaluate(False)

if __name__ == '__main__':
    IMP.test.main()
