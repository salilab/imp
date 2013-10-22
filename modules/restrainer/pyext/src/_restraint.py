"""Interface between XML Restraint and IMP.kernel.Restraint."""

import IMP.core
import IMP.em
import IMP.saxs
import IMP.restrainer

class _RestraintSets(object):
    def __init__(self):
        self._restraint_sets = dict()
        self.set_by_restraint = dict()
        self.set_by_name = dict()
    def add(self, restraint_type, weight, name, restraint, model = None):
        if restraint_type is None:
            return
        weight = str(weight)
        restraint_type = '%s|%s' % (restraint_type, name)
        try:
            set_by_weight = self._restraint_sets[weight]
        except KeyError:
            set_by_weight = self._restraint_sets[weight] = dict()
        try:
            current_set = set_by_weight[restraint_type]
        except KeyError:
            current_set = set_by_weight[restraint_type] = IMP.kernel.RestraintSet()
            if model:
                model.add_restraint(current_set)
        current_set.add_restraint(restraint)
        self.set_by_restraint[restraint] = current_set
        self.set_by_name[name] = current_set
    def set_model(self, model):
        for weight, set_by_weight in self._restraint_sets.iteritems():
            for rest_set in set_by_weight.itervalues():
                model.add_restraint(rest_set)
                rest_set.set_weight(float(weight))
    def remove_restraint(self, restraint):
        try:
            current_set = self.set_by_restraint[restraint]
        except KeyError:
            return
        current_set.remove_restraint(restraint)
        del self.set_by_restraint[restraint]
    def get_weight(self, restraint):
        try:
            current_set = self.set_by_restraint[restraint]
        except KeyError:
            return 1.0
        return current_set.get_weight()
    def remove_all(self, model):
        for set_by_weight in self._restraint_sets.itervalues():
            for rest_set in set_by_weight.itervalues():
                model.remove_restraint(rest_set)
        self._restraint_sets.clear()

class Restraint(object):
    """Store Restraint"""
    def __init__(self):
        """"""
        self._children = list()
        self._restraint_sets = _RestraintSets()

    def add_to_representation(self, repr):
        """ Place restraint into IMP Model."""

        if repr._model is None:
            repr.get_model()
        self._set_root()
        self._model = repr._model
        for child in self._children:
            child.create_restraint(repr)
        self.set_include(include=True)
        self.add_to_model()

    def add_to_model(self):
        self._restraint_sets.remove_all(self._model)
        for child in self._children:
            child.include_restraint(self._restraint_sets)
        self._restraint_sets.set_model(self._model)

    def set_include(self, name=None, include=False, weight=-1.0):

        def set_include_rec(node):
            for child in node._children:
                set_include_rec(child)
            node.include_me = include
            if weight >= 0:
                node.weight = weight

        if not name:
            for child in self._children:
                set_include_rec(child)
            return
        r = self.get_restraint_by_name(name)
        if r:
            if weight >= 0:
                r.weight = weight
            r.include_me = include

    def clear(self):

        def clear_rec(node):
            node.include_me = False
            for child in node._children:
                clear_rec(child)

        for child in self._children:
            clear_rec(child)
        self._restraint_sets.remove_all(self._model)


    def show_all_restraints(self):
        """Show restraint name, initial weight, and score for the current state of the model"""

        def find_rec(node):
            if node.imp_restraint is not None:
                print "=== ", node.name, " ==="
                print "Restraint initial weight = ", node.root._restraint_sets.get_weight(node.imp_restraint)
                node.imp_restraint.evaluate(False)
                found.append(node)
            for child in node._children:
                find_rec(child)

        found = list()
        for child in self._children:
            find_rec(child)
        return found


    def get_all_restraints_by_name(self, name):
        """Assuming there are many restraint objects with the same name."""

        def find_rec(node):
            if node.name == name:
                found.append(node)
            for child in node._children:
                find_rec(child)

        found = list()
        for child in self._children:
            find_rec(child)
        return found

    def get_restraint_by_name(self, name):
        """Assuming there is just one restraint object with the same name."""

        def find_rec(node):
            if node.name == name:
                return node
            for child in node._children:
                r = find_rec(child)
                if r:
                    return r
            return None

        for child in self._children:
            r = find_rec(child)
            if r:
                return r
        return None

    def get_restraint_set_by_name(self, name):
        """Get an IMP::RestraintSet by name specified in restraint XML file."""
        return self._restraint_sets.set_by_name.get(name)

    def get_rigid_body_by_id(self, id):
        """Get rigid body by particle id."""

        def find_rec(node):
            if node.id == id:
                return node.rigid_body
            for child in node._children:
                r = find_rec(child)
                if r:
                    return r
            return None

        for child in self._children:
            r = find_rec(child)
            if r:
                return r
        return None


    def get_all_rigid_bodies(self):
        """Get all rigid bodies."""

        def find_rec(node):
            if node.rigid_body:
                all_bodies.append(node.rigid_body)
            for child in node._children:
                find_rec(child)

        all_bodies = list()
        for child in self._children:
            find_rec(child)
        return all_bodies


    def _set_root(self):

        def set_root_rec(node):
            node.root = self
            for child in node._children:
                set_root_rec(child)

        for child in self._children:
            set_root_rec(child)


class _RestraintNode(object):
    counter = 0

    def __init__(self, attributes, restraint_type=None):
        self.imp_restraint = None
        self.rigid_body = None
        self.id = None
        name = attributes.get('name')
        if name:
            self.name = name
        else:
            self.name = 'object_%d' % _RestraintNode.counter
            _RestraintNode.counter += 1
        self._children = list()
        self.child_restraints = list()
        self.restraint_type = restraint_type
        self.include_me = False
        self.repr_particle = None

    def get_particle(self):
        particle_list = list()
        for child in self._children:
            if isinstance(child, _RestraintParticle):
                particle_list.append(child)
        return particle_list

    def get_source(self):
        source_list = list()
        for child in self._children:
            counter = 0
            if isinstance(child, _RestraintSource):
                source_list.append(child)
                source_list[counter].author_list = child.get_author()
                source_list[counter].journal = child.get_journal()
                source_list[counter].title   = child.get_title()
                source_list[counter].year    = child.get_year()
            counter += 1
        return source_list

    def append_restraint_with_weight(self, restraint, restraint_sets):
        if restraint:
            self.child_restraints.append(restraint)
            if self.restraint_type:
                restraint_sets.add(self.restraint_type, self.weight, self.name, restraint)

    def create_restraint(self, repr):
        self.child_restraints = list()
        for child in self._children:
            restraint = child.create_restraint(repr)
            if restraint:
                self.child_restraints.append(restraint)

    def include_restraint(self, restraint_sets):
        if self.restraint_type and self.include_me:
            for child in self.child_restraints:
                restraint_sets.add(self.restraint_type, self.weight, self.name,
                    child)
        for child in self._children:
            child.include_restraint(restraint_sets)


class _RestraintNodeWithWeight(_RestraintNode):
    def __init__(self, attributes, type):
        _RestraintNode.__init__(self, attributes, type)
        self.weight = float(attributes.get('weight', 1))


class _RigidBodyRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes):
        _RestraintNodeWithWeight.__init__(self, attributes, 'RigidBody')

    def create_restraint(self, repr):
        for child in self._children:
            restraint = child.create_rigid_body_restraint(repr)


class _ExcludedVolumeRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes, restraint_type):
        _RestraintNodeWithWeight.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr):
        self.child_restraints = list()
        for child in self._children:
            restraint = child.create_excluded_volume_restraint(repr)
            #self.append_restraint_with_weight(restraint, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)


class _ConnectivityRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes, restraint_type):
        _RestraintNodeWithWeight.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr):
        self.child_restraints = list()
        for child in self._children:
            restraint = child.create_connectivity_restraint(repr)
            #self.append_restraint_with_weight(restraint, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)


class _CrosslinkRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes, restraint_type):
        _RestraintNodeWithWeight.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr):
        self.child_restraints = list()
        for child in self._children:
            restraint = child.create_crosslink_restraint(repr)
            #self.append_restraint_with_weight(restraint, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)


class _DistanceRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes, restraint_type):
        _RestraintNodeWithWeight.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr):
        self.child_restraints = list()
        for child in self._children:
            restraint = child.create_distance_restraint(repr)
            #self.append_restraint_with_weight(restraint, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)


class _DiameterRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes, restraint_type):
        _RestraintNodeWithWeight.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr):
        self.child_restraints = list()
        for child in self._children:
            restraint = child.create_diameter_restraint(repr)
            #self.append_restraint_with_weight(restraint)
            if restraint:
                self.child_restraints.append(restraint)


class _SAXSRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes, restraint_type):
        _RestraintNodeWithWeight.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr):
        self.child_restraints = list()
        for child in self._children:
            restraint = child.create_saxs_restraint(repr)
            #self.append_restraint_with_weight(restraint, restraint_sets)
            if restraint:
                self.child_restraints.append(restraint)


class _SANSRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes, restraint_type):
        _RestraintNodeWithWeight.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr):
        self.child_restraints = list()

class _EMRestraintNode(_RestraintNodeWithWeight):
    def __init__(self, attributes, restraint_type):
        _RestraintNodeWithWeight.__init__(self, attributes, restraint_type)

    def create_restraint(self, repr):
        self.child_restraints = list()
        for child in self._children:
            restraint = child.create_em_restraint(repr)
            #self.append_restraint_with_weight(restraint)
            if restraint:
                self.child_restraints.append(restraint)


class _RestraintY2H(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "Y2H")

class _RestraintExcludedVolume(_ExcludedVolumeRestraintNode):
    def __init__(self, attributes):
        _ExcludedVolumeRestraintNode.__init__(self, attributes, "ExcludedVolume")

class _RestraintPulldown(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "Pulldown")

class _RestraintXrayStruc(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "XrayStruc")

class _RestraintMSMS(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "MSMS")

class _RestraintArray(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "Array")

class _RestraintCopurification(_ConnectivityRestraintNode):
    def __init__(self, attributes):
        _ConnectivityRestraintNode.__init__(self, attributes, "Copurification")

class _RestraintCrossLink(_CrosslinkRestraintNode):
    def __init__(self, attributes):
        _CrosslinkRestraintNode.__init__(self, attributes, "CrossLink")

class _RestraintDistance(_DistanceRestraintNode):
    def __init__(self, attributes):
        _DistanceRestraintNode.__init__(self, attributes, "Distance")

class _RestraintDiameter(_DiameterRestraintNode):
    def __init__(self, attributes):
        _DiameterRestraintNode.__init__(self, attributes, "Diameter")

class _RestraintSAXS(_SAXSRestraintNode):
    def __init__(self, attributes):
        _SAXSRestraintNode.__init__(self, attributes, "SAXS")

class _RestraintSANS(_SANSRestraintNode):
    def __init__(self, attributes):
        _SANSRestraintNode.__init__(self, attributes, "SANS")

class _RestraintEM(_EMRestraintNode):
    def __init__(self, attributes):
        _EMRestraintNode.__init__(self, attributes, "EM")

class _RestraintRestraint(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes)
        self.imp_restraint = None
        self.std_dev = float(attributes.get('std_dev', 1))
        self.distance = float(attributes.get('distance', 0))
        self.max_diameter = float(attributes.get('max_diameter', -1))
        self.profile_filename = attributes.get('profile_filename', '')
        self.density_filename = attributes.get('density_filename', '')
        self.resolution = float(attributes.get('resolution', -1))
        self.spacing = float(attributes.get('spacing', -1))
        self.xorigin = float(attributes.get('xorigin', 0.0))
        self.yorigin = float(attributes.get('yorigin', 0.0))
        self.zorigin = float(attributes.get('zorigin', 0.0))
        self.linker_length = float(attributes.get('linker_length', -1))

    def include_restraint(self, restraint_sets):
        pass

    def get_weight(self):
        return self.root._restraint_sets.get_weight(self.imp_restraint)

    def create_rigid_body_restraint(self, repr):
        _RestraintNode.create_restraint(self, repr)
        self.rigid_bodies = list()
        for child, child_r in zip(self._children, self.child_restraints):
            if child.repr_particle:
                child.repr_particle.force_field = 0
            mhs = IMP.atom.Hierarchies()
            mhs.append(child_r)
            rb = IMP.restrainer.set_rigid_bodies(mhs)
            self.rigid_bodies.append(rb[0])
            child.rigid_body = rb[0]
        return None

    def create_force(self):
        for child in self._children:
            repr = child.repr_particle
            print 'repr = ',repr
            if repr:
                print 'repr.force_field=', repr.force_field
            if repr and repr.force_field == 1:
                print 'Use the force.'
                hierarchy = repr.model_decorator
                if not hierarchy:
                    continue
                repr.force_field = 0
                print 'hierarchy present'
                id = repr.id
                m = self.root._model
                ff = IMP.atom.CHARMMParameters(repr.topology_file, repr.param_file)
                topology = ff.create_topology(hierarchy)
                topology.apply_default_patches(ff)
                #topology.add_atom_types(hierarchy)
                bonds = topology.add_bonds(hierarchy, ff)
                angles = ff.create_angles(bonds)
                dihedrals = ff.create_dihedrals(bonds)
                impropers = topology.add_impropers(hierarchy, ff)
                cont = IMP.container.ListSingletonContainer(bonds, "bonds_%s" % id)
                bss = IMP.atom.BondSingletonScore(IMP.core.Harmonic(0, 1))
                m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
                cont = IMP.container.ListSingletonContainer(angles, "angles_%s" % id)
                bss = IMP.atom.AngleSingletonScore(IMP.core.Harmonic(0,1))
                m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
                cont = IMP.container.ListSingletonContainer(dihedrals, "dihedrals_%s" % id)
                bss = IMP.atom.DihedralSingletonScore()
                m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
                cont = IMP.container.ListSingletonContainer(impropers, "impropers_%s" % id)
                bss = IMP.atom.ImproperSingletonScore(IMP.core.Harmonic(0,1))
                m.add_restraint(IMP.container.SingletonsRestraint(bss, cont))
                #ff.add_radii(hierarchy)
                ff.add_well_depths(hierarchy)
                atoms = IMP.atom.get_by_type(hierarchy, IMP.atom.ATOM_TYPE)
                cont = IMP.container.ListSingletonContainer(atoms)
                nbl = IMP.container.ClosePairContainer(cont, 4.0)
                pair_filter = IMP.atom.StereochemistryPairFilter()
                pair_filter.set_bonds(bonds)
                pair_filter.set_angles(angles)
                pair_filter.set_dihedrals(dihedrals)
                nbl.add_pair_filter(pair_filter)
                sf = IMP.atom.ForceSwitch(6.0, 7.0)
                ps = IMP.atom.LennardJonesPairScore(sf)
                m.add_restraint(IMP.container.PairsRestraint(ps, nbl))


    def create_connectivity_restraint(self, repr):
        _RestraintNode.create_restraint(self, repr)
        self.create_force()
        if self.std_dev > 0:
            k = 1.0/(2.0*self.std_dev*self.std_dev)
        else:
            k = 1.0
        mhs = IMP.atom.Hierarchies()
        if not self.child_restraints:
            raise Exception, "Restraint %s is empty" % self.id
        for child in self.child_restraints:
            mhs.append(child)
        first_particle = self.child_restraints[0].get_particle()
        if IMP.core.RigidBody.get_is_setup(first_particle):
            rbs_tmp = []
            for mh in mhs:
                rbs_tmp.append(mh.get_particle())
            rbs = IMP.core.RigidBodies(rbs_tmp)
            refiner = IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
            self.sc = IMP.restrainer.create_simple_connectivity_on_rigid_bodies(rbs, refiner)
        else:
            self.sc = IMP.restrainer.create_simple_connectivity_on_molecules(mhs)
        self.sc.set_k(k)
        connectivity_restraint = self.sc.get_restraint()
        self.imp_restraint = connectivity_restraint
        return connectivity_restraint

    def create_excluded_volume_restraint(self, repr):
        _RestraintNode.create_restraint(self, repr)
        self.create_force()
        mhs = IMP.atom.Hierarchies()
        if not self.child_restraints:
            raise Exception, "Restraint %s is empty" % self.id
        for child in self.child_restraints:
            mhs.append(child)
        first_particle = self.child_restraints[0].get_particle()
        if IMP.core.RigidBody.get_is_setup(first_particle):
            rbs_tmp = []
            for mh in mhs:
                rbs_tmp.append(mh.get_particle())
            rbs = IMP.core.RigidBodies(rbs_tmp)
            self.sev = IMP.restrainer.create_simple_excluded_volume_on_rigid_bodies(rbs)
        else:
            self.sev = IMP.restrainer.create_simple_excluded_volume_on_molecules(mhs)
        ev_restraint = self.sev.get_restraint()
        self.imp_restraint = ev_restraint
        return ev_restraint

    def create_saxs_restraint(self, repr):
        _RestraintNode.create_restraint(self, repr)
        self.create_force()
        if self.profile_filename:
            self.exp_profile = IMP.saxs.Profile(self.profile_filename)
            particles = IMP.atom.Hierarchies()
            for child in self.child_restraints:
                for atoms in IMP.atom.get_by_type(child, IMP.atom.ATOM_TYPE):
                    particles.append(atoms)
            if particles:
                self.saxs_restraint = IMP.saxs.Restraint(particles, self.exp_profile)
                self.imp_restraint = self.saxs_restraint
                return self.saxs_restraint

    def create_em_restraint(self, repr):
        _RestraintNode.create_restraint(self, repr)
        self.create_force()
        if self.density_filename:
            mhs = IMP.atom.Hierarchies()
            self.dmap = IMP.restrainer.load_em_density_map(self.density_filename,
                           self.spacing, self.resolution)
            self.dmap_header = self.dmap.get_header_writable()

            self.dmap.set_origin (self.xorigin, self.yorigin, self.zorigin)
            self.dmap.update_voxel_size(self.spacing)
            self.dmap_header.set_resolution (self.resolution)

            for child in self.child_restraints:
                mhs.append(child)
            if mhs:
                self.sef = IMP.restrainer.create_simple_em_fit(mhs, self.dmap)
                em_restraint = self.sef.get_restraint()
                self.imp_restraint = em_restraint
                return em_restraint

    def create_crosslink_restraint(self, repr):
        _RestraintNode.create_restraint(self, repr)
        self.create_force()

    def create_distance_restraint(self, repr):
        _RestraintNode.create_restraint(self, repr)
        self.create_force()
        if self.std_dev > 0:
            k = 1.0/(2.0*self.std_dev*self.std_dev)
        else:
            k = 1.0
        h = IMP.core.Harmonic(self.distance, k)
        self.imp_restraint = IMP.core.DistanceRestraint(h, self.child_restraints[0],
            self.child_restraints[1])
        return self.imp_restraint

    def create_diameter_restraint(self, repr):
        _RestraintNode.create_restraint(self, repr)
        self.create_force()
        ps = []
        for child in self.child_restraints:
            ps.append(child.get_particle())
        self.diameter_restraint = IMP.restrainer.create_simple_diameter(
                                 ps, self.max_diameter)
        self.imp_restraint = self.diameter_restraint.get_restraint()
        return self.imp_restraint

class _RestraintSource(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes)
        self.pubmed_id = int(attributes.get('pubmed_id', -1))

        self.__author_list = list()
        self.__journal = None
        self.__title = None
        self.__year = None

    def get_author(self):
        if not self.__author_list:
            for child in self._children:
                if isinstance(child, _RestraintAuthor):
                    self.__author_list.append(child)
                    break
        return self.__author_list

    def get_journal(self):
        if self.__journal is None:
            for child in self._children:
                if isinstance(child, _RestraintJournal):
                    self.__journal = child.text
                    break
        return self.__journal

    def get_title(self):
        if self.__title is None:
            for child in self._children:
                if isinstance(child, _RestraintTitle):
                    self.__title = child.text
                    break
        return self.__title

    def get_year(self):
        if self.__year is None:
            for child in self._children:
                if isinstance(child, _RestraintYear):
                    self.__year = child.text
                    break
        return self.__year

class _RestraintAuthor(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes)
        self.first_name = attributes.get('first_name', '')
        self.last_name = attributes.get('last_name', '')

class _RestraintJournal(_RestraintNode):
    def __init__(self, attributes, text):
        _RestraintNode.__init__(self, attributes)
        self.text = text

class _RestraintTitle(_RestraintNode):
    def __init__(self, attributes, text):
        _RestraintNode.__init__(self, attributes)
        self.text = text

class _RestraintYear(_RestraintNode):
    def __init__(self, attributes, text):
        _RestraintNode.__init__(self, attributes)
        self.text = text

class _RestraintParticle(_RestraintNode):
    def __init__(self, attributes):
        _RestraintNode.__init__(self, attributes)
        self.id      = attributes.get('id', '')
        self.residue = attributes.get('residue', '')
        self.atom    = attributes.get('atom', '')

    def create_restraint(self, repr):
        self.repr_particle = self.find_representation(repr)
        if not self.repr_particle:
            raise Exception, "Particle with id=%s not found" % (self.id)
        decorator = self.repr_particle.model_decorator
        if self.residue:
            decorator = IMP.atom.get_residue(decorator, int(self.residue))
            if self.atom:
                decorator = IMP.atom.Residue(decorator.get_particle())
                atom_type = IMP.atom.AtomType(self.atom)
                decorator = IMP.atom.get_atom(decorator, atom_type)
        return decorator

    def find_representation(self, repr):
        return repr.find_by_id(self.id)
