from pymol import cmd
from pymol import cgo
from chempy import models
import cProfile
import RMF

colors = {}

periodic_table = ["INV", "H", "He", "Li", "Be", "B", "C", "N", "O",
                  "F", "Ne", "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
                  "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge",
                  "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru",
                  "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I", "Xe", "Cs", "Ba",
                  "La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho",
                  "Er", "Tm", "Yb", "Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au",
                  "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th",
                  "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No",
                  "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn", "Uut",
                  "Fl", "Uup", "Lv", "Uus", "Uuo"]


class MyData:

    def __init__(self, fh, diameter):
        self.atom_factory = RMF.AtomFactory(fh)
        self.bond_factory = RMF.BondFactory(fh)
        self.particle_factory = RMF.ParticleFactory(fh)
        self.ball_factory = RMF.BallFactory(fh)
        self.segment_factory = RMF.SegmentFactory(fh)
        self.cylinder_factory = RMF.CylinderFactory(fh)
        self.diameter = diameter * 1.5


def _handle_atom_color(c, at):
    return
    ck = tuple((int(255. * x) for x in c))
    if ck not in colors:
        name = "rmf" + str(len(colors))
        print "color", name, c, ck
        cmd.set_color(name, list(c))
        colors[ck] = name
        at.color = name
    else:
        at.color = colors[ck]


def _handle_atom_coords(helper, mydata, model, cgol, pd, at):
    coords = [
        x for x in helper.get_global_coordinates(pd.get_coordinates())]
    coords[0] = coords[0] + helper.get_state_index() * mydata.diameter
    at.coord = coords


def _handle_atom_element(helper, mydata, model, cgol, at):
    element = mydata.atom_factory.get(helper).get_element()
    at.symbol = periodic_table[element]


def _handle_atom(helper, mydata, model, cgol):
    pd = mydata.particle_factory.get(helper)

    at = chempy.Atom()
    at.index = helper.set_is_displayed()
    at.name = helper.get_name()
    ri = helper.get_residue_index()
    if ri:
        at.resi = ri
    rn = helper.get_residue_type()
    if rn:
        at.resn = rn
    c = helper.get_rgb_color()
    if c:
        _handle_atom_color(c, at)
    cid = helper.get_chain_id()
    if cid:
        at.chain = cid
    _handle_atom_coords(helper, mydata, model, cgol, pd, at)
    at.segi = ""
    if mydata.atom_factory.get_is(helper):
        _handle_atom_element(helper, mydata, model, cgol, at)
    model.add_atom(at)


def _handle_bond(n, created, mydata):
    bd = mydata.bond_factory.get(n)
    b = chempy.Bond()
    bd0 = bd.get_bonded_0()
    bd1 = bd.get_bonded_1()
    for helpern in created:
        hp = created[helpern]
        if hp[0].get_is_displayed(bd0) and hp[0].get_is_displayed(bd1):
            b.index = [hp[0].get_index(bd0),
                       hp[0].get_index(bd1)]
            hp[1].add_bond(b)


def _create_atoms(helper, mydata, model, cgol, created):
    child = False
    for ch in helper.get_children():
        if _create_atoms(ch, mydata, model, cgol, created):
            child = True
    tp = helper.get_type()
    if tp == RMF.REPRESENTATION and not child and mydata.particle_factory.get_is(helper):
        _handle_atom(helper, mydata, model, cgol)
        child = True
    elif tp == RMF.GEOMETRY:
        if mydata.ball_factory.get_is(helper):
            d = mydata.ball_factory.get(helper)
            c = helper.get_rgb_color()
            if c:
                cgol.extend([cgo.COLOR] + helper.get_color())
            cgol.extend(
                [cgo.SPHERE] + d.get_coordinates() + [d.get_radius()])
        elif mydata.cylinder_factory.get_is(helper):
            d = mydata.cylinder_factory.get(helper)

        elif mydata.segment_factory.get_is(helper):
            pass
    elif tp == RMF.BOND and mydata.bond_factory.get_is(helper):
        _handle_bond(helper, created, mydata)

    return False


def _get_molecule_name(name, res):
    name = name.replace(":", "").replace(" ", "").replace(".", "_")
    if name.find("/") != -1:
        name = name.split("/")[-1]
    if res != -1:
        name = name + "-" + str(res)
    return name


def _create_molecule(n, mydata, resolution, created):
    model = models.Indexed()
    name = _get_molecule_name(n.get_name(), resolution)
    if name in created:
        name = name + "-2"
    print n, name, resolution
    th = RMF.TraverseHelper(n, name, resolution)
    created[name] = (th, model)
    print "creating molecule", name
    cgol = []
    _create_atoms(th, mydata, model, cgol, created)
    frame = n.get_file().get_current_frame().get_index() + 1

    if len(cgol) > 0:
        print cgol
        cmd.load_cgo(cgol, name + "-graphics", frame)


def _create_molecules(n, mf, cf, mydata, resolution, created):
    if mf.get_is(n) or cf.get_is(n):
        _create_molecule(n, mydata, resolution, created)
    elif n.get_type() == RMF.BOND and mydata.bond_factory.get_is(n):
        _handle_bond(n, created, mydata)
    else:
        for c in n.get_children():
            _create_molecules(c, mf, cf, mydata, resolution, created)


def _do_it(path):
    fh = RMF.open_rmf_file_read_only(path)
    mf = RMF.Molecule(fh)
    cf = RMF.ChainFactory(fh)

    res = RMF.get_resolutions(fh.get_root_node(), RMF.PARTICLE, .1)
    if len(res) == 1:
        res = [-1]
    fh.set_current_frame(RMF.FrameID(0))
    diameter = RMF.get_diameter(fh.get_root_node())
    mydata = MyData(fh, diameter)
    for f in fh.get_frames():
        created = {}
        fh.set_current_frame(f)
        for r in res:
            _create_molecules(
                fh.get_root_node(),
                mf,
                cf,
                mydata,
                r,
                created)
            if len(created) == 0:
                # fall back
                _create_molecule(fh.get_root_node(), mydata, r, created)
        for c in created:
            cmd.load_model(created[c][1], c, f.get_index() + 1)


def _open_rmf(path):
    # exec("_do_it(path)")
    cProfile.runctx("_do_it(path)", globals(), locals())
cmd.extend('rmf', _open_rmf)
