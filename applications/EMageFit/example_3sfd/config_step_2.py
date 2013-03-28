# Models for 3sfd using a sampling grid in domino


class MonteCarloParams: # <============== This has changed to make the running time short
    def __init__(self):
        self.temperatures = [30000,15000,10000,5000,1000]
        self.iterations = [2,2,2,2]
        self.cycles = 2
        self.max_translations = [20,15,10,5]
        self.max_rotations = [1,0.5,0.25,0.1]
        # Probability of using a random move within a RelativePositionMover
        self.non_relative_move_prob = 0.4

class DominoSamplingPositions:
    def __init__(self):
        self.read = "monte_carlo_solutions.db"
        self.max_number = 50 # max number of solutions to combine
        self.orderby = "em2d" # criteria to order the solutions

class DominoParams:
    def __init__(self, ):
        self.heap_solutions = 2000


class TestParameters(object):
    def __init__(self, ):
        self.do_test = False
        self.test_fn_assembly = ""



class Benchmark:
    def __init__(self, ):
        self.fn_pdb_native = "3sfd.pdb"
        # parameters for measuring the ccc with the 3D map
        self.resolution = 8
        self.voxel_size = 1.5
        self.native_map_threshold = 0.25
        # The component from the model is aligned to the native component before
        # computing the measures for the benchmark
        self.native_component = "3sfdB"
        self.model_component = "3sfdB"


class Experiment (object):

    def __init__(self,):

        # representaion
        self.model_name = ["3sfd"]
        self.names = ["3sfdA","3sfdB","3sfdC","3sfdD"]
        self.fn_pdbs = ["3sfdA.pdb","3sfdB.pdb","3sfdC.pdb","3sfdD.pdb"]
        self.n_residues = 10
        self.anchor = [False,True,False,False]
        self.fixed =[ False,False,False,False]

        # Pair score restraint:
        #             component1,component2,name,distance,weight,pairs,stddev
        self.pair_score_restraints =[ ["3sfdB","3sfdA","B_A",0,1,1,3.],
                                      ["3sfdB","3sfdC","B_C",0,1,1,3.],
                                      ["3sfdB","3sfdD","B_D",0,1,1,3.],
                                      ["3sfdD","3sfdC","D_C",0,1,1,3.]

                                    ]

        # Excluded volume restraint: distance, weight,pairs,stddev
        self.pairs_excluded_restraint = [0,1,0.1,2]

        # Em2DRestraint: name,images selection file,  pixel_size,
        # resolution, n_projections, weight, max_score
        self.em2d_restraints = [ ["em2d",  "em_images/images.sel", 1.5,5,20,1000,1000 ]
                                 ]

        # Cross-linking restraints component1,residue1, component2, residue2, distance,weight, stddev
        self.xlink_restraints =  [ ["3sfdB","B",23,"3sfdA","A",456,30,100,2],
                                   ["3sfdB","B",241,"3sfdC","C",112,30,100,2],
                                   ["3sfdB","B",205,"3sfdD","D",37,30,100,2],
                                   ["3sfdB","B",177,"3sfdD","D",99,30,100,2],
                                   ["3sfdC","C",95,"3sfdD","D",132,30,100,2],
                                   ["3sfdC","C",9,"3sfdD","D",37,30,100,2],
                                   ["3sfdC","C",78,"3sfdD","D",128,30,100,2],
                                    ]

        # self.have_hexdock = False
        self.dock_transforms =  [
                ["3sfdB","3sfdA","relative_positions_3sfdB-3sfdA.txt"],
                ["3sfdB","3sfdC","relative_positions_3sfdB-3sfdC.txt"],
                ["3sfdB","3sfdD","relative_positions_3sfdB-3sfdD.txt"],
                ["3sfdD","3sfdC","relative_positions_3sfdD-3sfdC.txt"],
                                    ]

        self.sampling_positions = DominoSamplingPositions()

        self.monte_carlo = MonteCarloParams()
        self.domino_params = DominoParams()

        self.test_opts = TestParameters()
        # benchmark
#        self.benchmark = Benchmark()

        # results
        self.n_solutions = 2000
        self.orderby = "em2d"
