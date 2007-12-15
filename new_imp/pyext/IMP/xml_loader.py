import xml.dom.minidom
import IMP
import IMP.utils

def load_imp_model(model, xml_file_path, base_particle = 0):
    """ Load imp model from given xml file """
    doc = xml.dom.minidom.parse(xml_file_path)

    # there should only be a single imp node, but just in case ...
    for head in doc.childNodes:
        if (head.nodeName == 'imp'):
            for n in head.childNodes:
                # process particle set nodes
                if n.nodeName == 'particles':
                    load_particles(model, n, base_particle)

                # process restraint set nodes
                elif n.nodeName == 'restraint_set':
                    load_restraint_set(model, n, base_particle)

    return doc


def load_particles(model, particles_node, base_particle):
    """ Load particles from the DOM """
    # save particle pointers to prevent destructors from running
    try:
        model.particles
    except AttributeError:
        model.particles = []
        model.num_particles = 0

    # process individual particle nodes
    for part in particles_node.childNodes:
        if (part.nodeName == 'particle'):
            load_particle(model, part, base_particle)


def load_particle(model, particle_node, base_particle):
    """ Load particle from the DOM """
    id = particle_node.getAttribute('id')
    model.particles.append(IMP.utils.XYZParticle(model))
    p_idx = len(model.particles)-1

    # process particle attributes
    for attr in particle_node.childNodes:
        if attr.nodeName == 'float':
            load_float(model, p_idx, attr)

        elif attr.nodeName == 'int':
            load_int(model, p_idx, attr)

        elif attr.nodeName == 'string':
            load_string(model, p_idx, attr)


def load_float(model, p_idx, attr_node):
    """ Load float attribute from the DOM and add it to the particle """
    name = attr_node.getAttribute('name')
    p = model.particles[p_idx]

    # defaults
    optimize = True
    value = 0.0
    for n in attr_node.childNodes:
        if n.nodeName == 'optimize':
            optimize = (n.childNodes[0].nodeValue == 'T')

        if n.nodeName == 'value':
            value = n.childNodes[0].nodeValue

    p.add_attribute(IMP.FloatKey(str(name)), float(value), bool(optimize))


def load_int(model, p_idx, attr_node):
    """ Load int attribute from the DOM and add it to the particle """
    name = attr_node.getAttribute('name')
    p = model.particles[p_idx]

    # defaults
    value = 0
    for n in attr_node.childNodes:
        if n.nodeName == 'value':
            value = n.childNodes[0].nodeValue
    p.add_attribute(IMP.IntKey(str(name)), int(value))


def load_string(model, p_idx, attr_node):
    """ Load string attribute from the DOM and add it to the particle """
    name = attr_node.getAttribute('name')
    p = model.particles[p_idx]

    # defaults
    value = ""
    for n in attr_node.childNodes:
        if n.nodeName == 'value':
            value = n.childNodes[0].nodeValue

    p.add_attribute(IMP.StringKey(name), str(value))


def load_restraint_set(model, restraint_set_node, base_particle):
    """ Load restraint set from the DOM """

    # save pointers to prevent destructors from running
    try:
        model.restraint_sets
    except AttributeError:
        model.restraint_sets = []

    try:
        model.restraints
    except AttributeError:
        model.restraints = []

    try:
        model.score_funcs
    except AttributeError:
        model.score_funcs = {}

    # get restraint set parameters
    name = str(restraint_set_node.getAttribute('name'))
    model.restraint_sets.append(IMP.RestraintSet(name))
    rs_idx = len(model.restraint_sets) - 1
    model.add_restraint(model.restraint_sets[rs_idx])

    # process restraints in subnodes
    for r in restraint_set_node.childNodes:
        if (r.nodeName == 'distance_restraint'):
            load_rsr_distance(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'torus_restraint'):
            load_rsr_torus(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'coordinate_restraint'):
            load_rsr_coordinate(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'connectivity_restraint'):
            load_rsr_connectivity(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'pair_connectivity_restraint'):
            load_rsr_pair_connectivity(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'proximity_restraint'):
            load_rsr_proximity(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'exclusion_volume_restraint'):
            load_rsr_exclusion_volume(model, rs_idx, r, base_particle)

        elif (r.nodeName == 'exclusion_volume_restraint'):
            load_rsr_exclusion_volume(model, rs_idx, r, base_particle)


def load_rsr_distance(model, rs_idx, rsr, base_particle):
    """ Load distance restraint set from the DOM """

    # process restraint parameters

    # defaults
    distance_attribute = ''
    distance = None
    sd = 0.1
    score_func = None

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_pairs':
            particle_pairs = get_particle_pairs(param)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance_attribute':
            distance_attribute = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func_str = str(param.childNodes[0].nodeValue)

    # make sure we have the needed parameters
    if score_func_str == None:
        print "No score function specified."
        return

    if distance == None and distance_attribute == '':
        print "No distance specified."
        return

    # add restraint for each particle pair in the list
    for pair in particle_pairs:
        p1 = model.particles[int(pair[0])]
        p2 = model.particles[int(pair[1])]
        if distance_attribute != '':
            distance = p1.get_value(IMP.FloatKey(distance_attribute)) \
                + p2.get_value(IMP.FloatKey(distance_attribute))

        score_func_params = get_basic_score_func_params(score_func_str, distance, sd)
        sf = score_func_params.create_score_func()
        model.restraints.append(IMP.DistanceRestraint(model, p1, p2, sf))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])


def load_rsr_torus(model, rs_idx, rsr, base_particle):
    """ Load torus restraint set from the DOM """

    # process restraint parameters

    # defaults
    distance_attribute = ''
    sd = 0.1
    tube_radius = 5.0
    main_radius = 20.0
    score_func = None

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_list':
            particle_list = get_particle_list(param)

        elif param.nodeName == 'main_radius':
            main_radius = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'tube_radius':
            tube_radius = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func_str = str(param.childNodes[0].nodeValue)

    # make sure we have the needed parameters
    if score_func_str == None:
        print "No score function specified."
        return

    # add restraint for each particle in the list
    for p_idx in particle_list:
        p = model.particles[int(p_idx)]
        score_func_params = get_basic_score_func_params(score_func_str, 0.0, sd)
        model.restraints.append(IMP.TorusRestraint(model, p, main_radius, tube_radius, score_func_params))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])


def load_rsr_coordinate(model, rs_idx, rsr, base_particle):
    """ Load coordinate restraint set from the DOM """
    print "Load coordinate restraint"

    # process restraint parameters

    # defaults
    axis = 'X_AXIS'
    sd = 0.1
    distance = 10.0
    score_func = None

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_list':
            particle_list = get_particle_list(param)

        elif param.nodeName == 'axis_type':
            axis = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func_str = str(param.childNodes[0].nodeValue)

    # make sure we have the needed parameters
    if score_func_str == None:
        print "No score function specified."
        return

    # add restraint for each particle in the list
    for p_idx in particle_list:
        p = model.particles[int(p_idx)]
        score_func_params = get_basic_score_func_params(score_func_str, distance, sd)
        model.restraints.append(IMP.CoordinateRestraint(model, p, axis, score_func_params))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])


def load_rsr_connectivity(model, rs_idx, rsr, base_particle):
    """ Load connectivity restraint set from the DOM """
    print "Load connectivity restraint"

    # process restraint parameters

    # defaults
    distance_attribute = ''
    type_attribute = 'protein'
    distance = None
    sd = 0.1
    score_func = None

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_list':
            particle_list = get_particle_list(param)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance_attribute':
            distance_attribute = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'type_attribute':
            type_attribute = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func_str = str(param.childNodes[0].nodeValue)

    # make sure we have the needed parameters
    if score_func_str == None:
        print "No score function specified."
        return

    # make sure we have the needed parameters
    if distance == None and distance_attribute == '':
        print "No distance specified."
        return

    # create a vector of particle indexes
    particle_indexes = IMP.Ints()
    for p_idx in particle_list:
        particle_indexes.push_back(int(p_idx))

    # add restraint for for vector of particles
    if distance_attribute:
        score_func_params = get_basic_score_func_params(score_func_str, 0.0, sd)
        model.restraints.append(IMP.ConnectivityRestraint(model, particle_indexes, type_attribute, distance_attribute, score_func_params))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])
    else:
        score_func_params = get_basic_score_func_params(score_func_str, distance, sd)
        model.restraints.append(IMP.ConnectivityRestraint(model, particle_indexes, type_attribute, score_func_params))
        model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])


def load_rsr_pair_connectivity(model, rs_idx, rsr, base_particle):
    """ Load pair connectivity restraint set from the DOM """
    print "Load  pair connectivity restraint"

    # process restraint parameters

    # defaults
    distance_attribute = ''
    distance = None
    sd = 0.1
    score_func = None
    num_lists = 0
    number_of_connects = 1

    # get the parameters
    for param in rsr.childNodes:
        if num_lists == 0 and param.nodeName == 'particle_list':
            particle_list1 = get_particle_list(param)
            num_lists = 1

        elif param.nodeName == 'particle_list':
            particle_list2 = get_particle_list(param)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance_attribute':
            distance_attribute = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func_str = str(param.childNodes[0].nodeValue)

        elif param.nodeName == 'number_of_connects':
            number_of_connects = int(param.childNodes[0].nodeValue)

    # make sure we have the needed parameters
    if distance == None and distance_attribute == '':
        print "No distance specified."
        return

    # make sure we have the needed parameters
    if score_func_str == None:
        print "No score function specified."
        return

    # create a vector of particle indexes
    particle_indexes1 = IMP.Ints()
    particle_indexes1.clear()
    for p_idx in particle_list1:
        particle_indexes1.push_back(int(p_idx))

    particle_indexes2 = IMP.Ints()
    particle_indexes2.clear()
    for p_idx in particle_list2:
        particle_indexes2.push_back(int(p_idx))

    # add restraint for for vector of particles
    if distance_attribute:
        score_func_params = get_basic_score_func_params(score_func_str, 0.0, sd)
        r = IMP.PairConnectivityRestraint(model, particle_indexes1, particle_indexes2, distance_attribute, score_func_params, number_of_connects)
        model.restraints.append(r)
        model.restraint_sets[rs_idx].add_restraint(r)
    else:
        score_func_params = get_basic_score_func_params(score_func_str, distance, sd)
        r = IMP.PairConnectivityRestraint(model, particle_indexes1, particle_indexes2, score_func_params, number_of_connects)
        model.restraints.append(r)
        model.restraint_sets[rs_idx].add_restraint(r)


def load_rsr_proximity(model, rs_idx, rsr, base_particle):
    """ Load proximity restraint set from the DOM """
    print "Load proximity restraint"

    # process restraint parameters

    # defaults
    distance = 5.0
    sd = 0.1
    score_func = 'harmonic_upper_bound'

    # get the parameters
    for param in rsr.childNodes:
        if param.nodeName == 'particle_list':
            particle_list = get_particle_list(param)

        elif param.nodeName == 'distance':
            distance = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'score_func':
            score_func_str = str(param.childNodes[0].nodeValue)

    # make sure we have the needed parameters
    if score_func_str == None:
        print "No score function specified."
        return

    # create a vector of particle indexes
    particle_indexes = IMP.Ints()
    for p_idx in particle_list:
        particle_indexes.push_back(int(p_idx))

    # add restraint for for vector of particles
        score_func_params = get_basic_score_func_params(score_func_str, 0.0, sd)
    model.restraints.append(IMP.ProximityRestraint(model, particle_indexes, distance, score_func_params))
    model.restraint_sets[rs_idx].add_restraint(model.restraints[len(model.restraints)-1])



def load_rsr_exclusion_volume(model, rs_idx, rsr, base_particle):
    """ Load exclusion volume restraint set from the DOM """
    print "Load  exclusion volume restraint"

    # defaults
    distance_attribute = ''
    sd = 0.1
    num_lists = 0

    # get the parameters
    for param in rsr.childNodes:
        if num_lists == 0 and param.nodeName == 'particle_list':
            particle_list1 = get_particle_list(param)
            num_lists = 1

        elif param.nodeName == 'particle_list':
            particle_list2 = get_particle_list(param)
            num_lists = 2

        elif param.nodeName == 'sd':
            sd = float(param.childNodes[0].nodeValue)

        elif param.nodeName == 'distance_attribute':
            distance_attribute = str(param.childNodes[0].nodeValue)

    # make sure we have the needed parameters
    if distance_attribute == '':
        print "No distance specified."
        return

    # create a vector of particle indexes
    particle_indexes1 = IMP.Ints()
    particle_indexes1.clear()
    for p_idx in particle_list1:
        particle_indexes1.push_back(int(p_idx))

    score_func_params = get_basic_score_func_params("harmonic_lower_bound", 0.0, sd)
    if (num_lists == 2):
        particle_indexes2 = IMP.Ints()
        particle_indexes2.clear()
        for p_idx in particle_list2:
            particle_indexes2.push_back(int(p_idx))

        print "adding inter particle exclusion volumes for two bodies: ", model, particle_indexes1, particle_indexes2, distance_attribute, sd
        r = IMP.ExclusionVolumeRestraint(model, particle_indexes1, particle_indexes2, distance_attribute, score_func_params)
        model.restraints.append(r)
        model.restraint_sets[rs_idx].add_restraint(r)
    else:
        print "adding intra particle exclusion volumes for one body: ", model, particle_indexes1, distance_attribute, sd
        r = IMP.ExclusionVolumeRestraint(model, particle_indexes1, distance_attribute, score_func_params)
        model.restraints.append(r)
        model.restraint_sets[rs_idx].add_restraint(r)



def get_basic_score_func_params(score_func_str, mean=0.0, sd=0.1):
    """ Get score function params corresponding to the given name """
    return IMP.BasicScoreFuncParams(score_func_str, mean, sd)


# support different ways of specifying particle pairs in the XML.
# <pair>1 3</pair> direct entering of particle pair indexes:
#   (1, 3)
# <all_pairs>3 4 5 6</all_pairs> all possible pairs from a list
#   (3, 4) (3, 5) (3, 6) (4, 5) (4, 6) (5, 6)
def get_particle_pairs(pairs_node):
    """ Get a list of all particle pairs specified by the given node """
    pairs = []
    for pair_node in pairs_node.childNodes:
        if pair_node.nodeName == 'pair':
            pairs.append(str(pair_node.childNodes[0].nodeValue).split())

        if pair_node.nodeName == 'all_pairs':
            list = get_particle_list(pair_node)
            for i in range(0, len(list)-1):
                for j in range(i+1, len(list)):
                    pairs.append((i, j))

    return pairs


# <particle_list>3 4 5 6</particle_list> list (3, 4, 5, 6)
def get_particle_list(particle_list_node):
    """ Get a list of all particles specified by the given node """
    return str(particle_list_node.childNodes[0].nodeValue).split()
