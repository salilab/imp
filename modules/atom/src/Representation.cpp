/**
 *  \file Atom.cpp   \brief Simple atoms decorator.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/Representation.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Mass.h>
#include <IMP/core/Gaussian.h>
#include <IMP/log.h>

#include <boost/unordered_map.hpp>
#include <sstream>
#include <vector>


IMPATOM_BEGIN_NAMESPACE

double get_resolution(Model* m, ParticleIndex pi) {
  double min = std::numeric_limits<double>::max();
  IMP_FOREACH(Hierarchy l, get_leaves(Hierarchy(m, pi))) {
    double cur = core::XYZR(l).get_radius();
    IMP_USAGE_CHECK(cur > 0, "Particle " << l << " has an invalid radius");
    min = std::min(min, cur);
  }
  return 1.0 / min;
}
const double ALL_RESOLUTIONS = -std::numeric_limits<double>::max();

IntsKey Representation::get_types_key() {
  static IntsKey k("representation_types");
  return k;
}

FloatsKey Representation::get_resolutions_key() {
  static FloatsKey k("representation_resolutions");
  return k;
}

FloatKey Representation::get_base_resolution_key() {
  static FloatKey key("base_resolution");
  return key;
}

ParticleIndexesKey Representation::get_representations_key() {
  static ParticleIndexesKey key("representations");
  return key;
}

void Representation::do_setup_particle(Model* m,
                                       ParticleIndex pi,
                                       double resolution) {
  if (resolution < 0) {
    resolution = get_resolution(m, pi);
  }
  m->add_attribute(get_base_resolution_key(), pi, resolution);
}

namespace {
double get_resolution_distance(double a, double b) {
  if (a < b) std::swap(a, b);
  return a / b - 1;
}
}

Hierarchy Representation::get_representation(double resolution,
                                             RepresentationType type) {
  IMP_USAGE_CHECK(type == BALLS || type == DENSITIES, "No matching types found");
  double closest_resolution = get_model()->get_attribute(
     get_base_resolution_key(), get_particle_index());

  int closest_index = -1;

  IMP_LOG_VERBOSE("Initial resolution to beat is "<<closest_resolution
                  << std::endl);

  // if requesting resolution 0 (and it is present) just return the base res
  if (type==BALLS && resolution==0.0 && closest_resolution==0.0) {
  }
  // if multiple representations, check for matching type and try to improve on base resolution
  else if (get_model()->get_has_attribute(get_types_key(), get_particle_index())) {
    Ints types =
        get_model()->get_attribute(get_types_key(), get_particle_index());
    IMP_LOG_VERBOSE("Found " << types.size() << " resolution levels"
                             << std::endl);
    Floats resolutions =
        get_model()->get_attribute(get_resolutions_key(), get_particle_index());
    for (unsigned int i = 0; i < types.size(); ++i) {
      if (types[i]==type) {
        double cur_resolution = resolutions[i];
        // force a match if requesting non-BALLS and this is the first match
        if ((closest_index==-1  && type!=BALLS) ||
            (get_resolution_distance(resolution, cur_resolution) <
             get_resolution_distance(resolution, closest_resolution))) {
              closest_index = i;
              closest_resolution = cur_resolution;
        }
      }
    }
  }
  IMP_LOG_VERBOSE("After searching, the closest resolution is "<<closest_resolution
                  <<" and closest index is "<<closest_index
                  << std::endl);

  if (closest_index==-1){
    if (type==BALLS) { //requested balls, couldn't find better than base res
      IMP_LOG_VERBOSE("Returning highest resolution children" << std::endl);
      return *this;
    }
    else {             // requested other than balls but couldn't find ANY match
      IMP_LOG_VERBOSE("The requested representation type was not found" << std::endl);
      return Hierarchy();
    }
  }
  else {
    IMP_LOG_VERBOSE("Returning children with resolution " << closest_resolution
                    << std::endl);
    return Hierarchy(get_model(), get_model()->get_attribute(
                                      get_representations_key(),
                                      get_particle_index())[closest_index]);
  }
}

Hierarchies Representation::get_representations(RepresentationType type) {
  Hierarchies ret;
  if (get_model()->get_has_attribute(get_types_key(), get_particle_index())) {
    Ints types =
        get_model()->get_attribute(get_types_key(), get_particle_index());
    for (unsigned int i = 0; i < types.size(); ++i) {
      if (types[i] == type) {
        ret.push_back(Hierarchy(
            get_model(), get_model()->get_attribute(get_representations_key(),
                                                    get_particle_index())[i]));
      }
    }
  }
  if (type == BALLS) ret += Hierarchies(1, *this);
  return ret;
}

void Representation::add_representation(ParticleIndexAdaptor rep,
                                        RepresentationType type,
                                        double resolution) {
  if (resolution < 0) {
    resolution = get_resolution(get_model(), rep);
  }
  int ct = 0;
  Hierarchies lvs = core::get_leaves(Hierarchy(get_model(),rep));
  for (unsigned i=0;i<lvs.size();i++) {
    ct += int(!core::Gaussian::get_is_setup(lvs[i]));
  }
  IMP_USAGE_CHECK( type!=DENSITIES || ct==0,
                   "DENSITY representations must be Gaussian");
  // If (self) has a parent, give new resolution the same parent (key)
  // However, don't do this if the new representation is the same particle!
  //  (e.g. for self-density representation)
  Hierarchies chs = get_children();
  if (get_model()->get_has_attribute(Hierarchy::get_traits().get_parent_key(),
                                     get_particle_index()) &&
      std::find(chs.begin(),chs.end(),Hierarchy(get_model(),rep))==chs.end()){
    get_model()->add_attribute(Hierarchy::get_traits().get_parent_key(), rep,
                               get_parent().get_particle_index());
  }
  if (get_model()->get_has_attribute(get_types_key(), get_particle_index())) {
    // if this particle already has this representation type setup, just add this resolution
    get_model()
        ->access_attribute(get_types_key(), get_particle_index())
        .push_back(type);
    get_model()
        ->access_attribute(get_representations_key(), get_particle_index())
        .push_back(rep);
    get_model()
        ->access_attribute(get_resolutions_key(), get_particle_index())
        .push_back(resolution);
  } else {
    // otherwise initiate a new list of resolutions for this type
    get_model()->add_attribute(get_types_key(), get_particle_index(),
                               Ints(1, type));
    get_model()->add_attribute(get_representations_key(), get_particle_index(),
                               ParticleIndexes(1, rep));
    get_model()->add_attribute(get_resolutions_key(), get_particle_index(),
                               Floats(1, resolution));
  }
}

Floats Representation::get_resolutions(RepresentationType type) const {
  Floats ret;
  if (type == BALLS) {
    ret.push_back(get_model()->get_attribute(get_base_resolution_key(),
                                             get_particle_index()));
  }

  if (get_model()->get_has_attribute(get_types_key(), get_particle_index())) {
    Ints types =
        get_model()->get_attribute(get_types_key(), get_particle_index());
    Floats resolutions =
        get_model()->get_attribute(get_resolutions_key(), get_particle_index());
    for (unsigned int i = 0; i < types.size(); ++i) {
      if (types[i] == type) {
        ret.push_back(resolutions[i]);
      }
    }
  }
  return ret;
}

void Representation::remove_representation(ParticleIndexAdaptor rep) {
  Ints types =
      get_model()->get_attribute(get_types_key(), get_particle_index());
  Floats resolutions =
      get_model()->get_attribute(get_resolutions_key(), get_particle_index());
  ParticleIndexes reps =
      get_model()->get_attribute(get_representations_key(),
                                 get_particle_index());
  for (unsigned int i = 0; i < reps.size(); i++) {
    if (reps[i] == rep) {
      types.erase(types.begin() + i);
      resolutions.erase(resolutions.begin() + i);
      reps.erase(reps.begin() + i);
      if (types.empty()) {
        get_model()->remove_attribute(get_types_key(), get_particle_index());
        get_model()->remove_attribute(get_resolutions_key(),
                                      get_particle_index());
        get_model()->remove_attribute(get_representations_key(),
                                     get_particle_index());
      } else {
        get_model()->set_attribute(get_types_key(), get_particle_index(),
                                   types);
        get_model()->set_attribute(get_resolutions_key(), get_particle_index(),
                                   resolutions);
        get_model()->set_attribute(get_representations_key(),
                                   get_particle_index(), reps);
      }
      return;
    }
  }
  IMP_THROW("The requested representation was not found", ValueException);
}

Representation get_representation(Hierarchy h, bool nothrow){
  Hierarchy mhd(h.get_particle());
  do {
    mhd = mhd.get_parent();
    if (mhd == Hierarchy()) {
      if (nothrow)
        return Representation();
      else {
        IMP_THROW("Hierarchy is not the child of a Representation " << h, ValueException);
      }
    }
  } while (!Representation::get_is_setup(mhd.get_particle()));
  Representation rd(mhd.get_particle());
  return rd;
}

void Representation::update_parents() {
  if (get_model()->get_has_attribute(Hierarchy::get_traits().get_parent_key(),
                                     get_particle_index()) &&
      get_model()->get_has_attribute(get_types_key(), get_particle_index())){
    Hierarchies chs = get_children();
    ParticleIndex parent = get_parent().get_particle_index();
    ParticleIndexes reps =
      get_model()->get_attribute(get_representations_key(), get_particle_index());
    for (unsigned int i = 0; i < reps.size(); i++) {
      if (std::find(chs.begin(),chs.end(),Hierarchy(get_model(),reps[i]))==chs.end()){
        if (get_model()->get_has_attribute(Hierarchy::get_traits().get_parent_key(),
                                           reps[i])){
          get_model()->set_attribute(Hierarchy::get_traits().get_parent_key(),
                                     reps[i],
                                     parent);
        }
        else {
          get_model()->add_attribute(Hierarchy::get_traits().get_parent_key(), reps[i],
                                     parent);
        }
      }
    }
  }
}


void Representation::show(std::ostream& out) const { out << "Resolutions: " << get_resolutions(); }

void show_with_representations(Hierarchy h,
                               std::ostream& stream) {
  // search through tree + representations
  Vector<boost::tuple<std::string, std::string, Hierarchy> > stack;
  stack.push_back(boost::make_tuple(std::string(), std::string(), h));
  do {
    boost::tuple<std::string, std::string, Hierarchy> &back = stack.back();
    std::string prefix0 = back.get<0>();
    std::string prefix1 = back.get<1>();
    Hierarchy cur = back.get<2>();
    stack.pop_back();
    stream << prefix0;
    unsigned int nc = cur.get_number_of_children();

    if (nc > 0) stream << " + ";
    else stream << " - ";  // leaf marker
    cur.show(stream);
    stream << std::endl;

    // if you have alternate representations, put on queue at the same tier as this
    if (Representation::get_is_setup(cur)){
      Hierarchies balls = Representation(cur.get_particle()).get_representations(BALLS);
      Hierarchies densities = Representation(cur.get_particle()).get_representations(DENSITIES);
      for (Hierarchies::const_iterator it = balls.begin();it!=balls.end();++it){
        //one of the representations is itself
        if ((*it)!=cur) stack.push_back(boost::make_tuple(prefix0 ,prefix1 , *it));
      }
      for (Hierarchies::const_iterator dit = densities.begin();dit!=densities.end();++dit){
        if ((*dit)!=cur) stack.push_back(boost::make_tuple(prefix0 ,prefix1 , *dit));
      }
    }

    // Children should go next (depth first)
    for (int i=nc-1;i>=0;--i) {
      stack.push_back(boost::make_tuple(prefix1 + " ",prefix1 + " ", cur.get_child(i)));
    }

  } while (!stack.empty());
  stream<<std::endl;
}

IMPATOM_END_NAMESPACE
