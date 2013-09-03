

import IMP
import IMP.display as display
import IMP.container as container
import IMP.atom as atom
import IMP.core as core
import IMP.algebra as alg
import IMP.em2d as em2d
import IMP.em2d.imp_general.representation as representation
import IMP.em2d.csv_related as csv_related
import time
import logging
log = logging.getLogger("io")


def get_vectors_from_points(points,vector_type="2d"):
    if(vector_type == "2d"):
        return [alg.Vector2D(p[0], p[1]) for p in points]
    elif(vector_type == "3d"):
        return [alg.Vector3D(p[0], p[1], p[2]) for p in points]
    else:
        raise ValueError("vector type not recognized")

def get_particles_from_points(points, model):
    """ Simply creates IMP particles from a set of 2D points
        model - is the model to store the particles
    """
    particles = []
    for x in points:
        p=IMP.kernel.Particle(model)
        d=IMP.core.XYZR.setup_particle(p)
        d.set_coordinates(alg.Vector3D(x[0], x[1], x[2]))
        d.set_radius(2)
        particles.append(p)
    return particles

def get_particles_from_points(points, model):
    """ Simply creates IMP particles from a set of 2D points
        model - is the model to store the particles
    """
    particles = []
    for x in points:
        p=IMP.kernel.Particle(model)
        d=IMP.core.XYZR.setup_particle(p)
        d.set_coordinates(alg.Vector3D(x[0], x[1], x[2]))
        d.set_radius(2)
        particles.append(p)
    return particles


def get_particles_from_vector3ds(vs, model):
    particles = []
    for v in vs:
        p=IMP.kernel.Particle(model)
        d=IMP.core.XYZR.setup_particle(p)
        d.set_coordinates(v)
        d.set_radius(2)
        particles.append(p)
    return particles



def write_particles_as_text(leaves,fn_output):
    """ Writes a set of particles with coordinates to a file """
    xyzs = core.XYZs(leaves)
    f_output=open(fn_output,"w")
    f_output.write(io.imp_info([IMP,em2d]) )

    f_output.write(get_common_title())
    for xyz in xyzs:
        x,y,z = xyz.get_coordinates()
        f_output.write("%8.3f %8.3f %8.3f\n" % (x,y,z))
    f_output.close()

def write_hierarchy_to_chimera(h, fn):
    """ Writes a hierarchy contained in h to the file fn """
    chw = display.ChimeraWriter(fn)
    g = display.HierarchyGeometry(h)
    chw.add_geometry(g)
    g.set_name(fn)

def write_particles_to_chimera(ps, fn, name="particles"):
    """ Writes a bunch o particles to the file fn
        It is assumed that the particles can be decorated with XYZR
    """
    chw = display.ChimeraWriter(fn)
    lsc = container.ListSingletonContainer(ps, name)
    g = display.XYZRsGeometry(lsc)
    chw.add_geometry(g)
    g.set_name(fn)

def write_xyzrs_to_chimera(xyzrs, fn):
    """ Writes a bunch o particles to the file fn
        It is assumed that the particles can be decorated with XYZR
    """
    ps = [ a.get_particle() for a in xyzrs]
    write_particles_to_chimera(ps, fn)


def write_points_to_chimera(points,radius, fn, name="points"):
    """ Writes a bunch o particles to the file fn
        It is assumed that the particles can be decorated with XYZR
    """
    m = IMP.kernel.Model()
    ps = []
    for p in points:
        pa = IMP.kernel.Particle(m)
        xyzr = core.XYZR.setup_particle(pa)
        xyzr.set_radius(radius)
        xyzr.set_coordinates( alg.Vector3D(p[0], p[1], p[2]) )
        ps.append(pa)
    write_particles_to_chimera(ps, fn, name)

def write_vectors_to_chimera(vs, radius, fn, name="vectors"):
    """
        Writes vectors as points in chimera
    """

    m = IMP.kernel.Model()
    ps = []
    for v in vs:
        pa = IMP.kernel.Particle(m)
        xyzr = core.XYZR.setup_particle(pa)
        xyzr.set_radius(radius)
        xyzr.set_coordinates(v)
        ps.append(pa)
    write_particles_to_chimera(ps, fn, name)

def write_vectors_in_multifit2_format(vs,fn_output):
    """ writes a text files in the format required
        for point alignment in multifit2
    """
    import IMP.multifit as mfit
    edges = []
    for v in vs:
        edges.append([0,0])

    ap = mfit.AnchorsData(vs,edges)
    mfit.write_txt(fn_output,ap)

def get_common_title():
    return "# coordinates x | y | z\n"


class Transformation3DToText:
    """
        Parseable output for a IMP Transformation3D
    """
    def __init__(self,T, delimiter="|"):
        q = T.get_rotation().get_quaternion()
        tr =  T.get_translation()
        self.rot_text = delimiter.join([str(i) for i in q])
        self.tr_text = delimiter.join([str(i) for i in tr])
        self.delimiter = delimiter
    def get_text(self):
        return self.delimiter.join([self.rot_text, self.tr_text] )


class TextToTransformation3D:
    def __init__(self, text, delimiter="|"):
        vals = [float(x) for x in text.split(delimiter)]
        if(len(vals) != 7):
            raise ValueError("The text is not a transformation",vals)
        R = alg.Rotation3D(vals[0], vals[1], vals[2], vals[3])
        t = alg.Vector3D(vals[4], vals[5], vals[6])
        self.t = alg.Transformation3D(R,t)
    def get_transformation(self):
        return self.t


class ReferenceFrameToText(Transformation3DToText):
    """
        Transform a IMP reference frame into parseable output
    """
    def __init__(self,ref, delimiter="|"):
        T = ref.get_transformation_to()
        Transformation3DToText.__init__(self, T, delimiter)


class TextToReferenceFrame(TextToTransformation3D):
    def __init__(self, text, delimiter="|"):
        TextToTransformation3D.__init__(self, text, delimiter)
        self.ref = alg.ReferenceFrame3D(self.get_transformation())
    def get_reference_frame(self):
        return self.ref


def read_transforms(fn,n=False):
    """
        Read a file of alg.Transformation3D. The it is assumed that the
        transformations are the only thing contained in a line
    """
    f = open(fn,"r")
    Ts = []
    for l in f:
        if(n != False):
            if(len(Ts) == n):
                break
        T = TextToTransformation3D(l).get_transformation()
        Ts.append(T)
    f.close()
    return Ts


def write_transforms(Ts, fn):
    """
        Write a file with the Transformation3Ds contained in Ts
    """
    f = open(fn,"w")
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
    rows = csv_related.read_csv(fn, delimiter = "/")
    x = min(n,len(rows))
    all_refs = []
    for i,row in enumerate(rows[0:x]):
        refs = [ TextToReferenceFrame(t).get_reference_frame() for t in row[1:]]
        all_refs.append(refs)
    return all_refs



def write_pdb_for_reference_frames(fn_pdbs, refs_texts, fn_output):
    """
        Read the PDB files, apply reference frames to them, and write a pdb
    """
    model = IMP.kernel.Model()
    assembly = representation.create_assembly(model, fn_pdbs)
    rbs = representation.create_rigid_bodies(assembly)
    for t, rb in zip(refs_texts, rbs):
        r = TextToReferenceFrame(t).get_reference_frame()
        rb.set_reference_frame(r)
    atom.write_pdb(assembly, fn_output)


def show_model_info(model, assembly, components_rbs):
    """ Prints information about the representation
        Prints the number of components (hierarchies), its children,
        and information about the chains and rigid bodies
    """
    print "##################################################################################"
    print "Model Info"
    print "##################################################################################"

    print model.show()
    print "#########################"
    print "Hierarchies in the assembly:"
    print "#########################"
    for c in assembly.get_children():
        print c.get_name(), "Is valid?", c.get_is_valid(True), "children", c.get_number_of_children()
        print "Child info: ",c.show()

        hchains = IMP.atom.get_by_type(c, IMP.atom.CHAIN_TYPE)
        print "Number of chains in the hierarchy:", len(hchains)
        for h in hchains:
            chain = atom.Chain(h.get_particle())
            print chain.get_name(),"particles",len(atom.get_leaves(chain) )
    print "#########################"
    print "Rigid bodies"
    print "#########################"
    for r in components_rbs:
        print "rigid body: Particles:",r.get_number_of_members(),
        print "coordinates:",r.get_coordinates()

    print "#########################"
    print "Restraints"
    print "#########################"
    n = model.get_number_of_restraints()
    for i in range(n):
        print model.get_restraint(i)


def imp_info(imp_modules= None):
    """
        Returns text with the time and information about the modules employed
        imp_modules is the set of modules whose infos are requested
    """

    tt = time.asctime()
    if(imp_modules == None):
        versions = [IMP.get_module_version()]
#        versions = [IMP.get_module_version_info()] # until SVN 11063
    else :
#        versions = [p.get_module_version_info() for p in imp_modules] # until SVN 11063
        versions = [p.get_module_version() for p in imp_modules]
    text = "# " + tt + "\n"
    for x in versions:
#        text += "# " + x.get_module() + " " + x.get_version() + "\n" # until SVN 11063
        text += "# " + x + "\n"
    return text
