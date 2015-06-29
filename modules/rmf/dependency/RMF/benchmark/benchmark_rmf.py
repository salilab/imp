## \example benchmark/benchmark_rmf.py
# \brief Benchmark operations on the RMF from python to see how much overhead
#        that adds

import RMF
import datetime

scale = 5


def create_residue(nh, af, pf):
    for i in range(0, 2 * scale):
        child = nh.add_child("CA", RMF.REPRESENTATION)
        pf.get(child).set_mass(1)
        pf.get(child).set_radius(1.0 + i / 18.77)
        af.get(child).set_element(7)


def create_chain(nh, rf, af, pf):
    for i in range(0, 60 * scale):
        child = nh.add_child(str(i), RMF.REPRESENTATION)
        rf.get(child).set_residue_type("cys")
        rf.get(child).set_residue_index(i)
        create_residue(child, af, pf)


def create_hierarchy(file):
    cf = RMF.ChainFactory(file)
    af = RMF.AtomFactory(file)
    rf = RMF.ResidueFactory(file)
    pf = RMF.ParticleFactory(file)
    n = file.get_root_node()
    for i in range(0, 3 * scale):
        child = n.add_child(str(i), RMF.REPRESENTATION)
        cf.get(child).set_chain_id(str(i))
        create_chain(child, rf, af, pf)


def create_frame(fh, ipf, atoms, frame):
    ret = [0.0, 0.0, 0.0]
    for n in atoms:
        v = RMF.Vector3((1.0 * n.get_index() + 0 + frame) / 17.0,
                        (1.0 * n.get_index() + 1 + frame) / 19.0,
                        (1.0 * n.get_index() + 2 + frame) / 23.0)
        ret[0] += v[0]
        ret[1] += v[1]
        ret[2] += v[2]
        ipf.get(fh.get_node(n)).set_coordinates(v)
    return ret[0] + ret[1] + ret[2]


def create(file):
    create_hierarchy(file)
    atoms = []
    for n in file.get_node_ids():
        if len(file.get_node(n).get_children()) == 0:
            atoms.append(n)
    ipf = RMF.IntermediateParticleFactory(file)
    ret = 0.0
    for i in range(0, 20):
        file.add_frame("frame", RMF.FRAME)
        ret += create_frame(file, ipf, atoms, i)
    return ret


def traverse(file):
    ret = 0.0
    queue = [file.get_root_node()]
    ipcf = RMF.IntermediateParticleConstFactory(file)
    while (len(queue) > 0):
        cur = queue[-1]
        queue = queue[:-1]

        if ipcf.get_is(cur):
            ret += ipcf.get(cur).get_radius()
        queue += cur.get_children()

    return ret


def load(file, nodes):
    ipcf = RMF.IntermediateParticleConstFactory(file)
    v = [0.0, 0.0, 0.0]
    for fr in file.get_frames():
        file.set_current_frame(fr)
        for n in nodes:
            cur = ipcf.get(file.get_node(n)).get_coordinates()
            v[0] += cur[0]
            v[1] += cur[1]
            v[2] += cur[2]
    return v[0] + v[1] + v[2]


def benchmark_create(file, type):
    start = datetime.datetime.now()
    dist = create(file)
    print type, "create,", datetime.datetime.now() - start, ",", dist


def benchmark_traverse(file, type):
    start = datetime.datetime.now()
    file.set_current_frame(RMF.FrameID(0))
    t = traverse(file)
    print type, "traverse,", datetime.datetime.now() - start, ",", t


def benchmark_load(file, type):
    nodes = []
    ipcf = RMF.IntermediateParticleConstFactory(file)
    for n in file.get_node_ids():
        if ipcf.get_is(file.get_node(n)):
            nodes.append(n)
    start = datetime.datetime.now()
    dist = load(file, nodes)
    print type, "load,", datetime.datetime.now() - start, ",", dist

name = "benchmark_python.rmf"
fh = RMF.create_rmf_file(name)
benchmark_create(fh, " python rmf")
del fh

fh = RMF.open_rmf_file_read_only(name)
benchmark_traverse(fh, "python rmf")
benchmark_load(fh, "python rmf")
