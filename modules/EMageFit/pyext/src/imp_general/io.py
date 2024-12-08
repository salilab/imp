"""@namespace IMP.EMageFit.imp_general.io
   Utility functions to handle IO.
"""

import IMP
import IMP.atom
import IMP.core
import IMP.algebra
import IMP.em2d
import IMP.EMageFit.imp_general.representation
import IMP.EMageFit.csv_related
import time
import logging
log = logging.getLogger("io")


def get_vectors_from_points(points, vector_type="2d"):
    if vector_type == "2d":
        return [IMP.algebra.Vector2D(p[0], p[1]) for p in points]
    elif vector_type == "3d":
        return [IMP.algebra.Vector3D(p[0], p[1], p[2]) for p in points]
    else:
        raise ValueError("vector type not recognized")


def get_particles_from_points(points, model):
    """ Simply creates IMP particles from a set of 2D points
        model - is the model to store the particles
    """
    particles = []
    for x in points:
        p = IMP.Particle(model)
        d = IMP.core.XYZR.setup_particle(p)
        d.set_coordinates(IMP.algebra.Vector3D(x[0], x[1], x[2]))
        d.set_radius(2)
        particles.append(p)
    return particles


def get_particles_from_vector3ds(vs, model):
    particles = []
    for v in vs:
        p = IMP.Particle(model)
        d = IMP.core.XYZR.setup_particle(p)
        d.set_coordinates(v)
        d.set_radius(2)
        particles.append(p)
    return particles


def write_particles_as_text(leaves, fn_output):
    """ Writes a set of particles with coordinates to a file """
    xyzs = IMP.core.XYZs(leaves)
    f_output = open(fn_output, "w")
    f_output.write(imp_info([IMP, IMP.em2d]))

    f_output.write(get_common_title())
    for xyz in xyzs:
        x, y, z = xyz.get_coordinates()
        f_output.write("%8.3f %8.3f %8.3f\n" % (x, y, z))
    f_output.close()


def write_vectors_in_multifit2_format(vs, fn_output):
    """ writes a text files in the format required
        for point alignment in multifit2
    """
    import IMP.multifit as mfit
    edges = []
    for v in vs:
        edges.append([0, 0])

    ap = mfit.AnchorsData(vs, edges)
    mfit.write_txt(fn_output, ap)


def get_common_title():
    return "# coordinates x | y | z\n"


class Transformation3DToText:

    """
        Parseable output for a IMP Transformation3D
    """

    def __init__(self, T, delimiter="|"):
        q = T.get_rotation().get_quaternion()
        tr = T.get_translation()
        self.rot_text = delimiter.join([str(i) for i in q])
        self.tr_text = delimiter.join([str(i) for i in tr])
        self.delimiter = delimiter

    def get_text(self):
        return self.delimiter.join([self.rot_text, self.tr_text])


class TextToTransformation3D:

    def __init__(self, text, delimiter="|"):
        vals = [float(x) for x in text.split(delimiter)]
        if len(vals) != 7:
            raise ValueError("The text is not a transformation", vals)
        R = IMP.algebra.Rotation3D(vals[0], vals[1], vals[2], vals[3])
        t = IMP.algebra.Vector3D(vals[4], vals[5], vals[6])
        self.t = IMP.algebra.Transformation3D(R, t)

    def get_transformation(self):
        return self.t


class ReferenceFrameToText(Transformation3DToText):

    """
        Transform a IMP reference frame into parseable output
    """

    def __init__(self, ref, delimiter="|"):
        T = ref.get_transformation_to()
        Transformation3DToText.__init__(self, T, delimiter)


class TextToReferenceFrame(TextToTransformation3D):

    def __init__(self, text, delimiter="|"):
        TextToTransformation3D.__init__(self, text, delimiter)
        self.ref = IMP.algebra.ReferenceFrame3D(self.get_transformation())

    def get_reference_frame(self):
        return self.ref


def read_transforms(fn, n=False):
    """
        Read a file of IMP.algebra.Transformation3D. The it is assumed that the
        transformations are the only thing contained in a line
    """
    f = open(fn, "r")
    Ts = []
    for line in f:
        if n:
            if len(Ts) == n:
                break
        T = TextToTransformation3D(line).get_transformation()
        Ts.append(T)
    f.close()
    return Ts


def write_transforms(Ts, fn):
    """
        Write a file with the Transformation3Ds contained in Ts
    """
    f = open(fn, "w")
    for T in Ts:
        txt = Transformation3DToText(T).get_text()
        f.write(txt + "\n")
    f.close()


def read_reference_frames(fn, n=10):
    """
        Read the reference frames contained in a solutions file from sampling
        n is the maximum number of ref frames to read.
        NOTE: Currently the function returns only the reference frames and
        discards the score, the first element of a row
    """
    rows = IMP.EMageFit.csv_related.read_csv(fn, delimiter="/")
    x = min(n, len(rows))
    all_refs = []
    for i, row in enumerate(rows[0:x]):
        refs = [TextToReferenceFrame(t).get_reference_frame() for t in row[1:]]
        all_refs.append(refs)
    return all_refs


def write_pdb_for_reference_frames(fn_pdbs, refs_texts, fn_output):
    """
        Read the PDB files, apply reference frames to them, and write a pdb
    """
    model = IMP.Model()
    assembly = IMP.EMageFit.imp_general.representation.create_assembly(
        model, fn_pdbs)
    rbs = IMP.EMageFit.imp_general.representation.create_rigid_bodies(assembly)
    for t, rb in zip(refs_texts, rbs):
        r = TextToReferenceFrame(t).get_reference_frame()
        rb.set_reference_frame(r)
    IMP.atom.write_pdb(assembly, fn_output)


def imp_info(imp_modules=None):
    """
        Returns text with the time and information about the modules employed
        imp_modules is the set of modules whose infos are requested
    """

    tt = time.asctime()
    if imp_modules is None:
        versions = [IMP.get_module_version()]
    else:
        versions = [p.get_module_version() for p in imp_modules]
    text = "# " + tt + "\n"
    for x in versions:
        text += "# " + x + "\n"
    return text
