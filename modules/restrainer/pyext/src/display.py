import IMP

class Display(object):
    """Store Display"""
    def __init__(self):
        self._children = list()

    def create_log(self, repr, log_name):
        """Create Chimera log.
           @param repr <a href="classIMP_1_1restrainer_1_1representation_1_1Representation.html">
            Representation</a> object.
           @param log_name Log name
           @return IMP::Dispay::LogOptimizerState"""

        # Try to create Chimera log (like in display_log.py example)
        log = IMP.display.LogOptimizerState(IMP.display.ChimeraWriter(), log_name)
        # Find IMP particle that corresponds to a representation id
        for child in self._children:
            child.find_model_decorator(repr)
        # Detach parents and children, so the geometries are separate (so that
        # we can set separate colors)
        for child in self._children:
            child.detach_parent()
        # Add geometries of proper colors to the log
        for child in self._children:
            child.create_xyz(1.0, 1.0, 1.0, log)
        # Reunite parents with children
        for child in self._children:
            child.attach_parent()
        return log

class _DisplayNode(object):
    counter = 0
    def __init__(self, attributes):
        id = attributes.get('id')
        if id:
            self.id = id
        else:
            self.id = 'object_%d' % _DisplayNode.counter
            _DisplayNode.counter += 1
        self._children = list()

    def get_color(self):
        for child in self._children:
            if isinstance(child, _DisplayColor):
                return child
        return None

    def find_model_decorator(self, repr):
        particle = repr.find_by_id(self.id)
        if particle and particle.model_decorator:
            self.model_decorator = particle.model_decorator
        else:
            self.model_decorator = None
        for child in self._children:
            child.find_model_decorator(repr)

    def detach_parent(self):
        if self.model_decorator:
            self.parent = self.model_decorator.get_parent()
            self.parent.remove_child(self.model_decorator)
        for child in self._children:
            child.detach_parent()

    def attach_parent(self):
        if self.model_decorator:
            self.parent.add_child(self.model_decorator)
        for child in self._children:
            child.attach_parent()

    def create_xyz(self, r, g, b, log):
        color = self.get_color()
        if color:
            r = color.r
            g = color.g
            b = color.b
        atom_list = list()
        if self.model_decorator:
            atoms = IMP.atom.get_by_type(self.model_decorator,
                IMP.atom.XYZR_TYPE)
            for atomh in atoms:
                for atom in atomh.get_leaves():
                    atom_list.append(atom)
        c = IMP.container.ListSingletonContainer(atom_list)
        geometry = IMP.display.XYZRsGeometry(c,
            IMP.core.XYZR.get_default_radius_key())
        geometry.set_name(self.id)
        geometry.set_color(IMP.display.Color(r, g, b))
        log.add_geometry(geometry)
        for child in self._children:
            child.create_xyz(r, g, b, log)


class _DisplayColor(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)
        self.r = float(attributes.get('r', 0))
        self.g = float(attributes.get('g', 0))
        self.b = float(attributes.get('b', 0))

class _DisplayResidue(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)
        self.start = int(attributes.get('start', -1))
        self.end = int(attributes.get('end', -1))

class _DisplayUniverse(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)

class _DisplayCollection(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)

class _DisplayAssembly(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)

class _DisplaySegment(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)

class _DisplayMolecule(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)

class _DisplayProtein(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)

class _DisplayNucleicAcid(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)

class _DisplayChain(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)

class _DisplayFragment(_DisplayNode):
    def __init__(self, attributes):
        _DisplayNode.__init__(self, attributes)
