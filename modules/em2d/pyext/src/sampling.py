
import IMP
import IMP.algebra as alg
import IMP.em2d.solutions_io as solutions_io
import IMP.em2d.imp_general.io as io
import IMP.em2d.Database as Database
import itertools
import logging
import os

log = logging.getLogger("sampling")

#
"""

    Functions required for sampling in a 2D/3D grid

"""
#


def create_sampling_grid_3d(diameter, n_axis_points):
    """
        Creates a grid of positions (Vector3Ds), centered at 0.
        @param diameter The shape of the grid is a sphere with this diameter.
        @param n_axis_points Number of points used alogn an axis for the grid.
        The axis X Y and Z will contain n_axis_points, equispaced.
        The other regions of space will contain only the points allowed by
        the size of the spere.
    """
    radius = diameter / 2.0
    step = diameter / n_axis_points
    points = [-radius + step * n for n in range(n_axis_points + 1)]
    # filter points closer than 1A
    points = filter(lambda x: abs(x) > 1, points)
    log.info("Points along the axis %s", points)
    positions = []
    for x, y, z in itertools.product(points, points, points):
        d = (x ** 2. + y ** 2. + z ** 2.) ** .5
        # allow 1% margin. Avoids approximation problems
        if(d < (1.01 * radius)):
            positions.append(alg.Vector3D(x, y, z))
    return positions


def create_sampling_grid_2d(diameter, n_axis_points):
    """
        Creates a grid of positions (Vector3Ds), centered at 0.
        The shape of the grid is a circle with diameter given by the parameter.
        n_axis_points is the number of points along an axis: The axis X Y
        will contain n_axis_points, equispaced. The other regions of space will
        contain only the points allowed by the size of the circle.
    """
    if(diameter < 0):
        raise ValueError("create_sampling_grid_2d: Negative diameter.")
    if(n_axis_points < 1):
        raise ValueError("create_sampling_grid_2d: Less than one axis point.")
    radius = diameter / 2.0
    step = diameter / n_axis_points
    points = [-radius + step * n for n in range(n_axis_points + 1)]
    log.info("Points along the axis %s", points)
    positions = []
    for x, y in itertools.product(points, points):
        d = (x ** 2. + y ** 2.) ** .5
        # allow 1% margin. Avoids approximation problems
        if(d < (1.01 * radius)):
            positions.append(alg.Vector3D(x, y, 0.0))
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
        r = alg.compose(r, rotation)
        oris.append(r)
    return oris


class SamplingSchema:

    def __init__(self, n_components, fixed, anchored):
        """
            Build a set of orientations and positions for sampling with DOMINO
            @param n_components Number of components in the assembly
            @param fixed List of True/False values. If fixed == True
                    for a component its position is not changed.
            @param anchor A list of True/False values. If anchor = True,
                    the component is set at (0,0,0) without rotating it.
        """
        if(n_components < 1):
            raise ValueError(
                "SamplingSchema: Requesting less than 1 components.")

        self.anchored = anchored
        self.fixed = fixed
        self.n_components = n_components

    def get_sampling_transformations(self, i):
        return self.transformations[i]

    def read_from_database(self, fn_database, fields=["reference_frames"],
                           max_number=False, orderby=False):
        """
            Read orientations and positions from a database file.
            self.anchored and self.fixed overwrite
            the positions and orientations read from the database
        """
        if not os.path.exists(fn_database):
            raise IOError("read_from_database: Database file not found. "
                          "Are you perhaps trying to run the DOMINO optimization without "
                          "having the database yet?")

        db = solutions_io.ResultsDB()
        db.connect(fn_database)
        data = db.get_solutions(fields, max_number, orderby)
        db.close()
        self.transformations = [[] for T in range(self.n_components)]
        # Each record contains a reference frame for each of the
        # components. But here the states considered make sense as columns.
        # Each rigidbody is considered to have all the states in a column.
        for d in data:
            texts = d[0].split("/")
            for i, t in zip(range(self.n_components), texts):
                T = io.TextToTransformation3D(t).get_transformation()
                self.transformations[i].append(T)

        # Set the anchored components
        for i in range(self.n_components):
            if self.anchored[i]:
                origin = alg.Transformation3D(alg.get_identity_rotation_3d(),
                                              alg.Vector3D(0.0, 0.0, 0.))
                self.transformations[i] = [origin]

        # If fixed, only the first state is kept
        for i in range(self.n_components):
            if self.fixed[i]:
                if len(self.transformations[i]) == 0:
                    raise ValueError("There are positions to keep fixed")
                self.transformations[i] = [self.transformations[i][0]]
