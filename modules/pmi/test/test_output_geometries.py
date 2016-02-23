import os

import IMP
import IMP.test
import IMP.display
import IMP.algebra
import IMP.atom
import IMP.core
import IMP.pmi.macros
import IMP.pmi.output
import IMP.rmf
import RMF


class OutputGeometriesTests(IMP.test.TestCase):

    def test_round_trip(self):
        mdl = IMP.Model()
        p = IMP.Particle(mdl)
        IMP.core.XYZR.setup_particle(p, 1)
        IMP.atom.Mass.setup_particle(p, 1)
        h = IMP.atom.Hierarchy.setup_particle(p)
        geo = IMP.display.SphereGeometry(
            IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4.))
        rmf_fn = self.get_tmp_file_name("geometrytest.rmf")
        o = IMP.pmi.output.Output()
        o.init_rmf(rmf_fn, [h], geometries=[geo])
        o.write_rmf(rmf_fn)
        o.close_rmf(rmf_fn)
        f = RMF.open_rmf_file_read_only(rmf_fn)
        rmf_geos = IMP.rmf.create_geometries(f)
        IMP.rmf.load_frame(f, RMF.FrameID(0))
        self.assertEqual(len(rmf_geos), 1)
        self.assertEqual(len(rmf_geos[0].get_components()), 1)

    def test_replica_exchange_round_trip(self):
        mdl = IMP.Model()
        pdb_file = self.get_input_file_name("mini.pdb")
        fasta_file = self.get_input_file_name("mini.fasta")

        seqs = IMP.pmi.topology.Sequences(fasta_file)
        s = IMP.pmi.topology.System(mdl)
        st = s.create_state()

        mol = st.create_molecule("P1", seqs[0])
        ares = mol.add_structure(pdb_file, chain_id="A", soft_check=True)
        mol.add_representation(ares, [1])
        root_hier = s.build()

        out_dir = IMP.test.TempDir(None)
        rex = IMP.pmi.macros.ReplicaExchange0(mdl,
                                              root_hier=root_hier,
                                              number_of_frames=1,
                                              monte_carlo_steps=1,
                                              number_of_best_scoring_models=0,
                                              global_output_directory=out_dir.tmpdir)
        geo = IMP.display.SphereGeometry(
            IMP.algebra.Sphere3D(IMP.algebra.Vector3D(1, 2, 3), 4.))
        rex.add_geometries([geo])
        rex.execute_macro()
        rmf_fn = os.path.join(out_dir.tmpdir, "rmfs", "0.rmf3")
        f = RMF.open_rmf_file_read_only(rmf_fn)
        rmf_geos = IMP.rmf.create_geometries(f)
        IMP.rmf.load_frame(f, RMF.FrameID(0))
        self.assertEqual(len(rmf_geos), 1)
        self.assertEqual(len(rmf_geos[0].get_components()), 1)


if __name__ == "__main__":
    IMP.test.main()
