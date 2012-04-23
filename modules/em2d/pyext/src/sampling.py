
import IMP
import IMP.algebra as alg
import IMP.em2d.solutions_io as solutions_io
import IMP.em2d.imp_general.io as io
import IMP.em2d.Database as Database
import itertools
import logging

log = logging.getLogger("sampling")

#########################################
"""

    Functions required for sampling in a 2D/3D grid

"""
#########################################


def create_sampling_grid_3d(diameter, n_axis_points):
    """
        Creates a grid of positions (Vector3Ds), centered at 0.
        @param diameter The shape of the grid is a sphere with this diameter.
        @param n_axis_points Number of points used alogn an axis for the grid.
        The axis X Y and Z will contain n_axis_points, equispaced.
        The other regions of space will contain only the points allowed by
        the size of the spere.
    """
    radius = diameter/2.0
    step = diameter/n_axis_points
    points = [-radius + step*n for n in range(n_axis_points +1 )]
    points = filter(lambda x: abs(x) > 1, points) # filter points closer than 1A
    log.info("Points along the axis %s", points)
    positions = []
    for x, y, z in itertools.product(points, points, points):
        d = (x**2. + y**2. + z**2.)**.5
        if(d < (1.01 *radius)): # allow 1% margin. Avoids approximation problems
            positions.append( alg.Vector3D(x,y,z))
    return positions


def create_sampling_grid_2d(diameter, n_axis_points):
    """
        Creates a grid of positions (Vector3Ds), centered at 0.
        The shape of the grid is a circle with diameter given by the parameter.
        n_axis_points is the number of points along an axis: The axis X Y
        will contain n_axis_points, equispaced. The other regions of space will
        contain only the points allowed by the size of the circle.
    """
    radius = diameter/2.0
    step = diameter/n_axis_points
    points = [-radius + step*n for n in range(n_axis_points +1 )]
    log.info("Points along the axis %s", points)
    positions = []
    for x, y in itertools.product(points, points):
        d = (x**2. + y**2.)**.5
        if(d < (1.01 *radius)): # allow 1% margin. Avoids approximation problems
            positions.append( alg.Vector3D(x,y, 0.0))
    return positions


def get_orientations_nearby(rotation, n, f):
    """
        Rotations nearby a given one. They are got intepolating with
        the rotations of the uniform coverage. The parameter f
        (0 <= f <= 1) must be close to 0 to get orientations
        that are close to the given orientation
        - Values from 0.1 (tight cluster) to 0.4 (loose)
          seem to be ok
        n - number of rotations requested
    """
    log.debug("Computing nearby rotations around %s", rotation)
    unif = alg.get_uniform_cover_rotations_3d(n)
    id_rot = alg.get_identity_rotation_3d()
    oris = []
    for rot in unif:
        r = alg.get_interpolated(rot, id_rot, f)
        r = alg.compose(r, rotation )
        oris.append(r)
    return oris


class SamplingSchema:
    """

        Sampling strategy to suggest positions and orientations for grid
        sampling.
        It can generate absolute grids of a given size or grids relative
        to a given set of positions
    """

    def __init__(self, n_components, fixed, anchored):
        """
            Build a set of orientations and positions for sampling
            @param n_components Number of components in the assembly
            @param fixed List of True/False values. If fixed == True
                    for a component its position is not changed.
            @param anchor A list of True/False values. If anchor = True,
                    the component is set at (0,0,0) without rotating it.
        """
        log.info("Creating sampling schema")
        log.info("Fixed components: %s.\n Anchored components %s",
                                                             fixed,anchored)
        # initial transformation for each component
        T = alg.Transformation3D( alg.get_identity_rotation_3d(),
                                    alg.Vector3D(0., 0., 0.) )
        self.initial = [T for v in range(n_components)]
        self.positions = [ [] for v in range(n_components)]
        self.orientations = [ [] for v in range(n_components)]
        self.anchored = anchored
        self.anchor(anchored)
        self.fixed = fixed
        self.fix(fixed)
        self.nc = n_components


    def read_previous_solution(self, fn_database, solution_index, orderby):
        if(fn_database != ""):
            text = solutions_io.get_best_solution( fn_database, solution_index,
                                                ["reference_frames"], orderby)
            log.debug("Previous_solution: %s",text)
            texts = text.split("/")
            self.initial = [ io.TextToTransformation3D(t).get_transformation()
                                                             for t in texts]
        if(self.nc != len(self.initial)):
            raise ValueError("Wrong number of solution transformations")
        self.previous_set = True

    def set_initial_transformations(self, assigned_transformations):
        self.initial = [T for T in assigned_transformations]
        self.previous_set = True

    def set_positions(self, opts):
        """
            There are different modes:
            - none: Means that no further exploration of positions is done.
                    The current position is the only one suggested
            - unif - The positions to explore are uniformly distributed on
                     a grid, with a diameter given by opts.diameter
                    and a number of points per axis opts.n_points
            - nearby - The positions explored are nearby the inital position
                        set by the function set_initial_transformations()
        """
        log.info("SamplingSchema: positions mode: %s ", opts.mode)
        if(opts.mode not in ["unif", "nearby", "none"]):
            raise ValueError("Sampling mode for the positions not recognized")
        if(opts.mode == "nearby" and not self.previous_set):
            raise ValueError("Nearby mode requested without previous solution")
        if(opts.mode == "none" and not self.previous_set):
            raise ValueError("mode \"none\" for positions can only be applied "\
                        "with a previous solution")
        if(opts.mode != "none"):
            if(opts.grid == "2d"):
                grid = create_sampling_grid_2d( opts.diameter, opts.n_points)
            elif(opts.grid == "3d"):
                grid = create_sampling_grid_3d(opts.diameter, opts.n_points)
            else:
                raise ValueError("Wrong grid mode")
        origin = alg.Vector3D(0.0, 0.0, 0.0)
        for i in range(self.nc):
            if(opts.mode == "unif"):
                self.positions[i] = grid
            elif(opts.mode == "nearby"):
                p0 = self.initial[i].get_translation()
                self.positions[i] = [ p0 + p for p in grid ]
                self.positions[i] = [ self.initial[i].get_translation() ]

    def set_orientations(self, opts):
        log.info("SamplingSchema: orientations mode: %s ", opts.mode)
        if(opts.mode not in ["unif", "nearby", "none"]):
            raise ValueError("Sampling mode for the orientations "\
                             "not recognized")
        if(opts.mode == "nearby" and not self.previous_set):
            raise ValueError("Nearby mode requested without previous solution")
        for i in range(self.nc):
            if(opts.mode == "unif"):
                unif = alg.get_uniform_cover_rotations_3d(opts.n_points)
                self.orientations[i] = unif
            elif(opts.mode == "nearby"):
                self.orientations[i] =  get_orientations_nearby(
                         self.initial[i].get_rotation(), opts.n_points, opts.f)
            elif(opts.mode == "none"):
                self.orientations[i] = [ self.initial[i].get_rotation() ]


    def get_sampling_transformations(self, i):
        Ts = []
        for r,p in itertools.product(self.orientations[i], self.positions[i]):
            Ts.append( alg.Transformation3D(r,p ) )
        return Ts

    def get_positions(self, i):
        return self.positions[i]

    def get_orientations(self, i):
        return self.orientations[i]

    def anchor(self, anchored):
        for i, is_anchored in enumerate(anchored):
            if(is_anchored):
                self.positions[i] = [alg.Vector3D(0.0, 0.0, 0.0)]
                self.orientations[i] = [alg.get_identity_rotation_3d()]

    def fix(self, fixed):
        for i, is_fixed in enumerate(fixed):
            if(is_fixed):
                self.positions[i] = [self.initial[i].get_translation()]
                self.orientations[i] = [self.initial[i].get_rotation()]

    def read_from_database(self, fn_database, fields=["reference_frames"],
                                    max_number=False, orderby=False ):
        """
            Read orientations and positions from a database file.
            self.anchored and self.fixed overwrite
            the positions and orientations read from the database
        """
        db = solutions_io.ResultsDB()
        db.connect(fn_database)
        data = db.get_solutions(fields, max_number, orderby)
        db.close()
        self.orientations = [ [] for T in range(self.nc)]
        self.positions = [ [] for T in range(self.nc)]
        # Each record contains a reference frame for each of the
        # components. But here the states considered make sense as columns.
        # Each rigidbody is considered to have all the states in a column.
        for d in data:
            texts = d[0].split("/")
            Ts = []
            for t in texts:
                x = io.TextToTransformation3D(t).get_transformation()
                Ts.append(x)
            for i in range(self.nc):
                self.positions[i].append(Ts[i].get_translation())
                self.orientations[i].append(Ts[i].get_rotation())
        self.anchor(self.anchored)
        self.fix(self.fixed)
