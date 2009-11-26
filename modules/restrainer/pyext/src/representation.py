import IMP
import IMP.atom
import random
import math

class _Representation(object):
    """Store Representation"""
    def __init__(self):
        self.children = list()
        self.model = None

    def find_all_by_id(self, id): # assuming there are many obj with the same id
        """Return a list of all nodes that have the id given by the parameter"""

        def find_rec(node):
            if node.id == id:
                found.append(node)
            for child in node.children:
                find_rec(child)

        found = list()
        for child in self.children:
            find_rec(child)
        return found

    def find_by_id(self, id): # assuming there is just one obj with the same id
        """Return a node that have the id given by the parameter"""

        def find_rec(node):
            if node.id == id:
                return node
            for child in node.children:
                r = find_rec(child)
                if r:
                    return r
            return None

        for child in self.children:
            r = find_rec(child)
            if r:
                return r
        return None

    def to_model(self, model=None):
        """Return an IMP model that contains the representation"""
        if model is None:
            model = self.model
            if model is None:
                self.model = model = IMP.Model()
        else:
            self.model = model
        repr_particle = IMP.Particle(model)
        decorator = IMP.atom.Hierarchy.setup_particle(repr_particle)
        for child in self.children:
            child.set_parent(self)
        for child in self.children:
            child.add_as_child(decorator, model)
        return model

class _RepresentationNode(object):
    counter = 0
    def __init__(self, attributes):
        id = attributes.get('id')
        if id:
            self.id = id
        else:
            self.id = 'object_%d' % _RepresentationNode.counter
            _RepresentationNode.counter += 1
        self.children = list()
        self.model_decorator = None
        self.parent = None

    def set_parent(self, parent):
        self.parent = parent
        for child in self.children:
            child.set_parent(self)

    def add_as_child(self, particle, model):
        """Add this node as a child of particle."""
        #print 'add_as_child', self
        if not self.model_decorator:
            decorator = self.to_particle(model)
            self.model_decorator = decorator
        #print 'model_decorator:',[self.model_decorator]
        if decorator:
            particle.add_child(decorator)
        else:
            decorator = particle
        for child in self.children:
            child.add_as_child(decorator, model)

    def to_particle(self, model):
        self.this_particle = IMP.Particle(model)
        decorator = IMP.atom.Hierarchy.setup_particle(self.this_particle)
        return decorator

class _RepUniverse(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)


class _RepCollection(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)


class _RepAssembly(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)


class _RepSegment(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)


class _RepMolecule(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)


class _RepProtein(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)


class _RepNucleicAcid(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)


class _RepChain(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.filename = attributes.get('filename', '')
        self.chain_label = attributes.get('chain_label', '')
        self.selector =  attributes.get('selector', '')

    def to_particle(self, model):
        if self.filename:
            if self.selector == 'CAlpha':
                selector = IMP.atom.CAlphaSelector()
            elif self.selector == 'CBeta':
                selector = IMP.atom.CBetaSelector()
            elif self.selector == 'C':
                selector = IMP.atom.CSelector()
            elif self.selector == 'N':
                selector = IMP.atom.NSelector()
            elif self.selector == 'All':
                selector = IMP.atom.AllSelector()
            elif self.selector == 'Chain':
                selector = IMP.atom.ChainSelector(self.chain_label)
            elif self.selector == 'Water':
                selector = IMP.atom.WaterSelector()
            elif self.selector == 'Hydrogen':
                selector = IMP.atom.HydrogenSelector()
            elif self.selector == 'NonWater':
                selector = IMP.atom.NonWaterSelector()
            elif self.selector == 'P':
                selector = IMP.atom.PSelector()
            elif self.selector == 'IgnoreAlternatives':
                selector = IMP.atom.IgnoreAlternativesSelector()
            elif self.selector == 'NonWaterNonHydrogen':
                selector = IMP.atom.NonWaterSelector()
            else:
                selector = IMP.atom.NonWaterNonHydrogenSelector()
            decorator = IMP.atom.read_pdb(self.filename, model, selector)
            IMP.atom.add_radii(decorator)
            chains = IMP.atom.get_by_type(decorator, IMP.atom.CHAIN_TYPE)
            self.fragment_decorator = chains[0]
            parent = self.fragment_decorator.get_parent()
            parent.remove_child(self.fragment_decorator)
        if self.children and not self.filename:
            particle = IMP.Particle(model)
            decorator = IMP.atom.Chain.setup_particle(particle,
                self.chain_label)
        else:
            if not self.filename and not self.children:
                raise Exception, "Filename must be present for childless Chain %s" % self.id
            decorator = self.fragment_decorator
        return decorator

class _RepFragment(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)

    def to_particle(self, model):
        if len(self.children) != 1:
            raise Exception, "Fragment %s must have exactly one child" % self.id
        particle = IMP.Particle(model)
        child = self.children[0]
        child.add_attributes(particle)
        decorator = IMP.atom.Fragment.setup_particle(particle)
        if isinstance(child, _RepAtomicRep):
            if not isinstance(self.parent, _RepChain):
                raise Exception, "Parent of Fragment %s must be a chain" % self.id
            if particle.has_attribute(IMP.IntKey('start_residue')):
                start_residue = particle.get_value(IMP.IntKey('start_residue'))
            else:
                raise Exception, "Start residue is required for atomic rep of Fragment %s" % self.id
            if particle.has_attribute(IMP.IntKey('end_residue')):
                end_residue = particle.get_value(IMP.IntKey('end_residue'))
            else:
                raise Exception, "End residue is required for atomic rep of Fragment %s" % self.id
            for x in xrange(int(start_residue), int(end_residue) + 1):
                res_part = IMP.atom.get_residue(self.parent.fragment_decorator, x)
                if res_part != IMP.atom.Hierarchy():
                    res_parent = res_part.get_parent()
                    res_parent.remove_child(res_part)
                    decorator.add_child(res_part)
        else:
            if not particle.has_attribute(IMP.FloatKey('x')):
                particle.add_attribute(IMP.FloatKey('x'), random.random())
            if not particle.has_attribute(IMP.FloatKey('y')):
                particle.add_attribute(IMP.FloatKey('y'), random.random())
            if not particle.has_attribute(IMP.FloatKey('z')):
                particle.add_attribute(IMP.FloatKey('z'), random.random())
            if particle.get_value(IMP.FloatKey('radius')) < 0:
                particle.set_value(IMP.FloatKey('radius'),
                particle.get_value(IMP.FloatKey('calc_radius')))
        return decorator


class _RepAtomicRep(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.start_residue = int(attributes.get('start_residue', -1))
        if not self.start_residue:
            self.start_residue = int(attributes.get('start_nucleotide', -1))
        self.end_residue = int(attributes.get('end_residue', -1))
        if not self.end_residue:
            self.end_residue = int(attributes.get('end_nucleotide', -1))
        if self.start_residue < 0 or self.end_residue < 0:
            raise Exception, "AtomicRep %s must have both start_(residue|nucleotide) and end_(residue|nucleotide)" % self.id

    def to_particle(self, model):
        return None

    def add_attributes(self, parent):
        parent.add_attribute(IMP.IntKey("start_residue"), self.start_residue)
        parent.add_attribute(IMP.IntKey("end_residue"), self.end_residue)
        for child in self.children:
            child.add_attributes(parent)

class _RepGeometricShapeRep(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.start_residue = int(attributes.get('start_residue', -1))
        if self.start_residue < 0:
            self.start_residue = int(attributes.get('start_nucleotide', -1))
        self.end_residue = int(attributes.get('end_residue', -1))
        if self.end_residue < 0:
            self.end_residue = int(attributes.get('end_nucleotide', -1))
        self.total_residue = int(attributes.get('total_residue', -1))
        if self.total_residue >= 0 and self.end_residue >= 0 and \
            self.start_residue >= 0:
            if self.total_residue != self.end_residue + 1 - self.start_residue:
                raise Exception, "Total residues dubious consistency in Geometric Shape _Rep %s" % self.id

    def to_particle(self, model):
        return None

    def add_attributes(self, parent):
        parent.add_attribute(IMP.IntKey('start_residue'), self.start_residue)
        parent.add_attribute(IMP.IntKey('end_residue'), self.end_residue)
        r = -1
        if self.start_residue < 0 and self.end_residue < 0:
            total_residue = self.total_residue
        else:
            total_residue = self.end_residue-self.start_residue+1
        if total_residue >= 0:
            m = IMP.atom.mass_from_number_of_residues(total_residue)
            v = IMP.atom.volume_from_mass(m)
            r = (v/(4.0*math.pi)*3.0)**(1.0/3)
            parent.add_attribute(IMP.FloatKey('calc_radius'), r)
        for child in self.children:
            child.add_attributes(parent)


class _RepSphere(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.radius = float(attributes.get('radius', -1))
        self.weight = float(attributes.get('weight', 0))
        self.__initial_position = None

    def initial_position(self):
        if self.__initial_position is None:
            for child in self.children:
                if isinstance(child, _RepInitialPosition):
                    self.__initial_position = child
                    break
        return self.__initial_position

    def to_particle(self, model):
        return None

    def add_attributes(self, parent):
        parent.add_attribute(IMP.FloatKey("radius"), self.radius)
        parent.add_attribute(IMP.FloatKey("weight"), self.weight)
        parent.add_attribute(IMP.FloatKey("mass"), self.weight)
        for child in self.children:
            child.add_attributes(parent)

class _RepInitialPosition(_RepresentationNode):
    def __init__(self, attributes):
        _RepresentationNode.__init__(self, attributes)
        self.optimize = int(attributes.get('optimize', -1))
        self.x = float(attributes['x'])
        self.y = float(attributes['y'])
        self.z = float(attributes['z'])

    def to_particle(self, model):
        return None

    def add_attributes(self, parent):
        fl_x = IMP.FloatKey("x")
        fl_y = IMP.FloatKey("y")
        fl_z = IMP.FloatKey("z")
        parent.add_attribute(fl_x, self.x)
        parent.add_attribute(fl_y, self.y)
        parent.add_attribute(fl_z, self.z)
        parent.add_attribute(IMP.IntKey("optimize"), self.optimize)
        if self.optimize == 1:
            parent.set_is_optimized(fl_x, True)
            parent.set_is_optimized(fl_y, True)
            parent.set_is_optimized(fl_z, True)
