/**
 *  \file Model.cpp \brief Storage of a model, its restraints,
 *                         constraints and particles.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#include "IMP/Model.h"
#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Restraint.h"
#include "IMP/DerivativeAccumulator.h"
#include "IMP/ScoreState.h"
#include <boost/timer.hpp>
#include <set>

#if IMP_BUILD < IMP_FAST
#define WRAP_UPDATE_CALL(restraint, expr, exchange)                     \
  {                                                                     \
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {                                  \
      ParticlesTemp rpl;                                                \
      if (!exchange) rpl= (restraint)->get_input_particles();           \
      else rpl= (restraint)->get_output_particles();                    \
      ParticlesTemp wpl;                                                \
      if (!exchange) wpl= (restraint)->get_output_particles();          \
      else wpl= (restraint)->get_input_particles();                     \
      wpl.insert(wpl.end(), rpl.begin(), rpl.end());                    \
      if (exchange) {                                                   \
        rpl.insert(rpl.end(), wpl.begin(), wpl.end());                  \
      }                                                                 \
      internal::ReadLock rl(particles_begin(), particles_end(),         \
                            rpl.begin(), rpl.end());                    \
      internal::WriteLock wl(particles_begin(), particles_end(),        \
                             wpl.begin(), wpl.end());                   \
      try {                                                             \
        expr;                                                           \
      } catch (internal::ReadLockedParticleException &e) {              \
        std::ostringstream oss;                                         \
        for (unsigned int i=0; i< rpl.size(); ++i) {                    \
          oss << rpl[i]->get_name() << " ";                             \
        }                                                               \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the read particles list of "           \
                  << (restraint)->get_name() << " but should be. "      \
                  << "The list contains " << oss.str() );               \
        throw InternalException("Invalid particle used");               \
      } catch (internal::WriteLockedParticleException &e) {             \
        std::ostringstream oss;                                         \
        for (unsigned int i=0; i< wpl.size(); ++i) {                    \
          oss << wpl[i]->get_name() << " ";                             \
        }                                                               \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the write particles list of "          \
                  << (restraint)->get_name() << " but should be."       \
                  << "The list contains " << oss.str() );               \
        throw InternalException("Invalid particle used");               \
      }                                                                 \
    } else {                                                            \
      expr;                                                             \
    }                                                                   \
  }

#define WRAP_EVALUATE_CALL(restraint, expr)                             \
  {                                                                     \
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {                                  \
      ParticlesTemp rpl= (restraint)->get_input_particles();            \
      internal::ReadLock rl(particles_begin(), particles_end(),         \
                            rpl.begin(), rpl.end());                    \
      internal::WriteLock wl(particles_begin(), particles_end(),        \
                             rpl.begin(), rpl.end());                   \
      try {                                                             \
        expr;                                                           \
      } catch (internal::ReadLockedParticleException e) {               \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the read particles list of "           \
                  << (restraint)->get_name() << " but should be.");     \
        throw InternalException("Invalid particle used");               \
      } catch (internal::WriteLockedParticleException e) {              \
        IMP_ERROR("Particle " << e.p_->get_name()                       \
                  << " is not in the write particles list of "          \
                  << (restraint)->get_name() << " but should be.");     \
        throw InternalException("Invalid particle used");               \
      }                                                                 \
    } else {                                                            \
      expr;                                                             \
    }                                                                   \
  }
#else
#define WRAP_UPDATE_CALL(restraint, expr, exchange) expr
#define WRAP_EVALUATE_CALL(restraint, expr) expr
#endif


IMP_BEGIN_INTERNAL_NAMESPACE
struct ReadLock{
  ParticlesTemp p_;
  std::set<Object *> allowed_;
public:
  template <class It, class It1>
  ReadLock(It1 pa, It1 pb,
           It ab, It ae): p_(pa, pb),
                          allowed_(ab, ae){
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< p_.size(); ++i) {
      if (allowed_.find(p_[i]) == allowed_.end()) {
        p_[i]->ps_->read_locked_=true;
      }
    }
#endif
  }
  ~ReadLock() {
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< p_.size(); ++i) {
      p_[i]->ps_->read_locked_=false;
    }
#endif
  }
};


struct WriteLock{
  ParticlesTemp p_;
  std::set<Object *> allowed_;
public:
  template <class It, class It1>
  WriteLock(It1 pa, It1 pb,
            It ab, It ae): p_(pa, pb),
                           allowed_(ab, ae){
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< p_.size(); ++i) {
      if (allowed_.find(p_[i]) == allowed_.end()) {
        p_[i]->ps_->write_locked_=true;
      }
    }
#endif
  }
  ~WriteLock() {
#if IMP_BUILD < IMP_FAST
    for (unsigned int i=0; i< p_.size(); ++i) {
      p_[i]->ps_->write_locked_=false;
    }
#endif
  }
};

IMP_END_INTERNAL_NAMESPACE

IMP_BEGIN_NAMESPACE

/*

  todo
  - add container base class with method to tell if it changed
  - remove Interaction base class
*/

/*
  types:
  R set of all restraints, a member is r
  S set of all states, a member is s
  C set of all containers, a member is c
  P set of all particles. a member is p

  capitals are sets, lower case it variable

  unqualified variables mean for all

  {} set
  () ordered pair
  f() function  call

  - GRC: build graph of what Restraints require what containers:
     nodes(GRC)= R+C, edges(GRC) from {(r,c)}
  - GRP: build graph of what Restraints require what particles:
     nodes(GRP) = R+P, edges(GRP) from {(r,p)}
  - GSC: build graph of what model states read/write what containers:
     nodes(GSC) = S+C, edges(GSC) from {(s,c)}, {(c,s)}
  - GSP: build graph of what model states read/write what particles:
     nodes(GSP) = S+P, edges(GSP) from {(s,p)}, {(p,s)}
  - method to produce topological sort from a graph sort(GXY)->LXY
  - method to return dependency nodes of a given type from a graph
     given an input set dependencies(GXY,{z}, T)->{t} where T is X or Y
  - method update(LXY, {x}) update all x in {x} and in LXY in the order LXY
  - clear(B) clears all the bits in B

  rules:
  - particles, containers can only be written once
  - the set of outputs cannot change, only the inputs
  - the set of inputs/output objects can only depend on the set of input
    objects, not the particle attributes
  - hierarchies and rigid bodies cannot change dynamically
  - finding the list of input/output containers is fast
  - finding the list of input/output particles is slow

  state bits:
  - BC[i]: dirty bit for each container

  any evaluation:
  when the list of restraints has changed:
  GRC= build_grc()
  LRC= sort(GRC)
  delete GRP
  when the list of score states has changed:
  GSC= build_gsc()
  LSC= sort(GSC)
  delete GSP

  to evaluate all:
  clear(BC[i])
  update(LSC, S)
  if BC[i] or !GSP:
  GSP = build_gsp() // probably patch it, ultimately
  LSP = sort(GSP)
  update(LSP, S)
  evaluate(R)

  to evaluate {e}
  clear(BC[i])
  rc= dependencies(GRC, {e}, C)
  (cc, sc)= dependencies(GSC, rc, (C,S))
  update(LSC, sc)
  if (BC[x] for x in rc+cc) or !GSP:
  GSP= build_gsp()
  LSP = sort(GSP)
  GRP= build_gsr()
  rp= dependencies(GRP, {e}, P)
  sp= dependencies(GSP, {rp}, S)
  update(LSP, sp)
  evaluate({e})


  representation

*/
/*
   try 2:
   state:
   order of all score states
   update when:
   new score added
   any score state says that its dependencies changed: punt
   dependent score states for each restraint
   update when:
   above graph changes
   the restraint reports that its set of inputs has changed
   evaluate all:
   get order of all score states and evaluate them in that order
   evaluate all restraints in that order

   evaluate some:
   merge list of dependent score states for each restraint
   evaluate them in order of all score states
*/

namespace {
  struct Statistics {
    double total_time_;
    double total_time_after_;
    unsigned int calls_;
    double total_value_;
    double min_value_;
    double max_value_;
    Statistics(): total_time_(0), total_time_after_(0),
                  calls_(0), total_value_(0),
                  min_value_(std::numeric_limits<double>::max()),
                  max_value_(-std::numeric_limits<double>::max())
    {}
    void update_state_before(double t) {
      total_time_+=t;
    }
    void update_state_after(double t) {
      total_time_after_+=t;
      ++calls_;
    }
    void update_restraint(double t, double v) {
      total_time_+= t;
      min_value_= std::min(v, min_value_);
      max_value_= std::max(v, max_value_);
      ++calls_;
    }
  };

  std::map<Object*, Statistics> stats_data_;
}


//! Constructor
Model::Model()
{
  cur_stage_=NOT_EVALUATING;
  incremental_update_=false;
  first_incremental_=true;
  gather_statistics_=false;
  score_states_ordered_=false;
  set_was_owned(true);
}


//! Destructor
Model::~Model()
{
  IMP_CHECK_OBJECT(this);
  for (ParticleIterator it= particles_begin();
       it != particles_end(); ++it) {
    (*it)->ps_->model_ = NULL;
    internal::unref(*it);
  }
}

IMP_LIST_IMPL(Model, Restraint, restraint, Restraint*,
              Restraints,
              {IMP_INTERNAL_CHECK(cur_stage_== NOT_EVALUATING,
       "The set of restraints cannot be changed during evaluation.");
                obj->set_model(this);
                obj->set_was_owned(true);
                score_states_ordered_=false;
                first_incremental_=true;},,
              {
                obj->set_model(NULL);
                restraint_deps_.erase(obj);
              });

IMP_LIST_IMPL(Model, ScoreState, score_state, ScoreState*,
              ScoreStates,
              {IMP_INTERNAL_CHECK(cur_stage_== NOT_EVALUATING,
                            "The set of score states cannot be changed during"
                                  << "evaluation.");
                obj->set_model(this);
                score_states_ordered_=false;
                obj->set_was_owned(true);
                IMP_IF_CHECK(USAGE) {
                  std::set<ScoreState*> in(score_states_begin(),
                                           score_states_end());
                  IMP_USAGE_CHECK(in.size() == get_number_of_score_states(),
                                  "Score state already in model "
                                  << obj->get_name(), UsageException);
                }
              },,
              {obj->set_model(NULL);});

#if 0
namespace {
  struct Graph {
    typedef boost::adjacency_list<boost::vecS, boost::vecS,
                                  boost::directedS,
                                  boost::property<boost::vertex_id_t, Object*,
boost::property<boost::mark_t, bool> ,
                                  boost::no_property> BG;
    typedef boost::graph_traits<BG>::edge_descriptor Edge;
    typedef boost::graph_traits<BG>::vertex_descriptor Vertex;
    std::map<Object*, Vertex> ov_;
    BG bg_;
    Vertex get_vertex(Object *a) {
      if (ov_.find(a) == ov_.end()) {
        ov_[a]=boost::add_vertex();
        // set props
      }
      return ov_[a];
    }

    void add_edge(Object *a, Object *b) {
      boost::add_edge(bg_,get_vertex(a), get_vertex(b));
    }
    unsigned int get_number_of_output(Vertex v) {
      return boost::out_degree(bg_, v);
    }
    Vertex get_output_vertex(Vertex v, unsigned int i) {
      return boost::out_edge(v, i);
    }
    unsigned int get_number_of_input(Vertex v) {
      return boost::in_degree(bg_, v);
    }
    Vertex get_input_vertex(Vertex v, unsigned int i) {
      return boost::in_edge(bg_, v, i);
    }
    unsigned int get_number_of_vertices() {
      return boost::vertex_size(bg_);
    }
    Vertex get_vertex(unsigned int i) {
      return boost::vertex(bg_, i);
    }
    Object* get_object(Vertex v) {
      return;
    }
    bool get_mark(Vertex v) {

    }
    void set_mark(Vertex v, bool tf) {

    }
  };

  void build_dependency_graph(const RestraintsTemp &rs,
                              const ScoreStatesTemp &ss,
                              const ParticlesTemp &ps,
                              Graph &g) {
    ContainersTemp cs;
    for (unsigned int i=0; i< rs.size(); ++i) {
      ContainersTemp ccs= rs[i]->get_input_containers();
      cs.insert(cs.end(), ccs.begin(), ccs.end());
      for (unsigned int j=0; j< ccs.size(); ++j) {
        g.add_edge(ccs[j], rs[i])
      }
    }
    for (unsigned int i=0; i< ss.size(); ++i) {
      ContainersTemp ccs= ss[i]->get_input_containers();
      cs.insert(cs.end(), ccs.begin(), ccs.end());
      for (unsigned int j=0; j< ccs.size(); ++j) {
        g.add_edge(ccs[j], ss[i])
      }
      ContainersTemp cos= ss[i]->get_output_containers();
      for (unsigned int j=0; j< cos.size(); ++j) {
        g.add_edge(ss[i], cos[j])
      }
    }
    for (unsigned int i=0; i< cs.size(); ++i) {
      ContainersTemp ccs= cs[i]->get_input_containers();
      cs.insert(cs.end(), ccs.begin(), ccs.end());
      for (unsigned int j=0; j< ccs.size(); ++j) {
        g.add_edge(ccs[j], cs[i])
      }
      ParticlesTemp cps= cs[i]->get_input_particles();
      for (unsigned int j=0; j< cps.size(); ++j) {
        g.add_edge(cps[j], cs[i])
      }
    }
  }

  void sort_score_states(const Graph &g,
                         ScoreStatesTemp &out) {
    std::vector<Graph::Vertex> active;
    for (unsigned int i=0; i < g.get_number_of_vertices(); ++i) {
      Graph::Vertex v= g.get_vertex(i);
      unsigned int d =g.get_number_of_input(v);
      if (d==0){
        active.push_back(n);
      }
    }
    while (!active.empty()) {
      Graph::Vertex c= active.back();
      active.pop_back();
      g.set_marked(c,true);
      for (unsigned int i=0; i< g.get_number_of_output(c); ++i) {
        Graph::Vertex o=g.get_output_vertex(c,i);
        bool unmarked=false;
        for (unsigned int j=0; j< g.get_number_of_input(o); ++i) {
          Graph::Vertex in = g.get_input_vertex(o,j);
          if (!g.get_marked(in) {
            unmarked=true;
            break;
          }
        }
        if (unmarked) {
          active.push_back(o);
        }
      }
      ScoreState *ss=dynamic_cast<ScoreState*>(get_object(g, c));
      if (ss) {
        out.push_back(ss);
      }
    }
  }
  template <class Type>
  void get_all_upstream(const Graph g,
                        Graph::Vertex start,
                        std::vector<Type*> &out) {
    std::vector<Graph::Vertex> front(1,start);
    do {
      Graph::Vertex cur =front.back();
      front.pop_back();
      Type *cs= dynamic_cast<Type*>(g.get_object(cur));
      if (cs) {
        out.push_back(cs);
      }
      for (unsigned int i=0; i< g.get_number_of_input(cur); ++i) {
        front.push_back(g.get_input_vertex(i));
      }
    } while (!front.empty());
  }

  void make_required_states(const ScoreStates &all,
                            const ScoreStates &some,
                            RequiredStates &rs) {
    std::map<ScoreState *, int> map;
    for (unsigned int i=0; i< all.size(); ++i) {
      map[all[i]] = i;
    }
    for (unsigned int i=0; i< some.size(): ++i) {
      rs[map[some[i]]]=true;
    }
  }
}
#endif

void Model::order_score_states() {
  // go away once the ordering below happens
  IMP_IF_CHECK(USAGE) {
    std::set<Object*> read_objects;
    for (ScoreStateConstIterator it = score_states_begin();
         it != score_states_end(); ++it) {
      ObjectsTemp wp= (*it)->get_output_objects();
      ParticlesTemp wpp= (*it)->get_output_particles();
      wp.insert(wp.end(), wpp.begin(), wpp.end());
      for (unsigned int i=0; i< wp.size(); ++i) {
        if (read_objects.find(wp[i]) != read_objects.end()) {
          IMP_WARN("Particle " << wp[i]->get_name()
                   << " was changed by score state "
                   << (*it)->get_name()
                   << " after it was read by another score state."
                   << " This may result in score states not being"
                   << " updated properly. Eventually they will be"
                   << " reordered automatically, but for now "
                   << "you have to do it yourself.");
        }
      }
      ObjectsTemp rp= (*it)->get_input_objects();
      read_objects.insert(rp.begin(), rp.end());
      ParticlesTemp rpp= (*it)->get_input_particles();
      read_objects.insert(rpp.begin(), rpp.end());
    }
  }

  restraint_deps_.clear();
  for (unsigned int i=0; i< get_number_of_restraints(); ++i) {
    restraint_deps_[get_restraint(i)]
      = RequiredStates(get_number_of_score_states(),
                       std::numeric_limits<unsigned long>::max());
  }

  score_states_ordered_=true;
}

FloatRange Model::get_range(FloatKey k) const {
  IMP_CHECK_OBJECT(this);
  if (ranges_.find(k) != ranges_.end()) {
    return ranges_.find(k)->second;
  } else {
    FloatRange r(std::numeric_limits<Float>::max(),
                 -std::numeric_limits<Float>::max());
    for (ParticleConstIterator it= particles_begin();
         it != particles_end(); ++it) {
      if ((*it)->has_attribute(k)) {
        Float v= (*it)->get_value(k);
        r.first = std::min(r.first, v);
        r.second= std::max(r.second, v);
      }
    }
    IMP_LOG(TERSE, "Range for attribute " << k << " is " << r.first
            << " to " << r.second << std::endl);
    return r;
  }
}

void Model::before_evaluate(const ScoreStatesTemp &states) const {
  IMP_USAGE_CHECK(cur_stage_== NOT_EVALUATING,
                  "Can only call Model::before_evaluate() when not evaluating",
                  UsageException);
  IMP_LOG(TERSE,
          "Begin update ScoreStates " << std::endl);
  cur_stage_= BEFORE_EVALUATE;
  boost::timer timer;
  for (unsigned int i=0; i< states.size(); ++i) {
    ScoreState *ss= states[i];
    IMP_CHECK_OBJECT(ss);
    IMP_LOG(TERSE, "Updating " << ss->get_name() << std::endl);
    if (gather_statistics_) timer.restart();
    WRAP_UPDATE_CALL(ss, ss->before_evaluate(), false);
    if (gather_statistics_) {
      stats_data_[ss].update_state_before(timer.elapsed());
    }
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(TERSE, "End update ScoreStates." << std::endl);
}

void Model::after_evaluate(const ScoreStatesTemp &states,
                           bool calc_derivs) const {
  IMP_LOG(TERSE,
          "Begin after_evaluate of ScoreStates " << std::endl);
  DerivativeAccumulator accum;
  cur_stage_= AFTER_EVALUATE;
  boost::timer timer;
  for (int i=states.size()-1; i>=0; --i) {
    ScoreState *ss= states[i];
    IMP_CHECK_OBJECT(ss);
    IMP_LOG(TERSE, "Updating " << ss->get_name() << std::endl);
    if (gather_statistics_) timer.restart();
    WRAP_UPDATE_CALL(ss, ss->after_evaluate(calc_derivs?&accum:NULL), true);
    if (gather_statistics_) {
      stats_data_[ss].update_state_after(timer.elapsed());
    }
    IMP_LOG(VERBOSE, "." << std::flush);
  }
  IMP_LOG(TERSE, "End after_evaluate of ScoreStates." << std::endl);
}

void Model::zero_derivatives(bool st) const {
  for (ParticleConstIterator pit = particles_begin();
       pit != particles_end(); ++pit) {
    (*pit)->zero_derivatives();
    if (st) {
      (*pit)->get_prechange_particle()->zero_derivatives();
    }
  }
}

double Model::do_evaluate_restraints(const Restraints &restraints,
                                     bool calc_derivs,
                                     WhichRestraints incremental_restraints,
                                     bool incremental_evaluation) const {
  IMP_IF_LOG(TERSE) {
    std::string which;
    if (incremental_restraints== ALL) which="all";
    else if (incremental_restraints== NONINCREMENTAL) which="non-incremental";
    else which = "incremental";
    IMP_LOG(TERSE,
            "Begin " << (incremental_evaluation ? "incremental-":"")
            << "evaluate of "
            << which << " restraints "
            << (calc_derivs?"with derivatives":"without derivatives")
            << std::endl);
  }
  double score=0;
  DerivativeAccumulator accum;
  boost::timer timer;
  for (Restraints::const_iterator it= restraints.begin();
       it != restraints.end(); ++it) {
    double value;
    if (gather_statistics_) timer.restart();
    if ((*it)->get_is_incremental()
        && incremental_restraints != NONINCREMENTAL) {
      if (incremental_evaluation) {
        WRAP_EVALUATE_CALL(*it,
                 value=(*it)->unprotected_incremental_evaluate(calc_derivs?
                                                                &accum:NULL));
        IMP_LOG(TERSE, (*it)->get_name() << " score is " << value << std::endl);
      } else {
        WRAP_EVALUATE_CALL(*it,
                      value=(*it)->unprotected_evaluate(calc_derivs?
                                                        &accum:NULL));
        IMP_LOG(TERSE, (*it)->get_name() << " score is " << value << std::endl);
      }
      if (gather_statistics_) {
        stats_data_[*it].update_restraint(timer.elapsed(), value);
      }
      score+= value;
    } else if (!(*it)->get_is_incremental()
               && incremental_restraints != INCREMENTAL) {
      WRAP_EVALUATE_CALL(*it,
                         value=(*it)->unprotected_evaluate(calc_derivs?
                                                           &accum:NULL));
      IMP_LOG(TERSE, (*it)->get_name()<<  " score is " << value << std::endl);
      if (gather_statistics_) {
        stats_data_[*it].update_restraint(timer.elapsed(), value);
      }
      score+= value;
    }
  }
  IMP_LOG(TERSE, "End evaluate restraints." << std::endl);
  return score;
}

void Model::validate_attribute_values() const {
  std::string message;
  for (ParticleConstIterator it= particles_begin();
       it != particles_end(); ++it) {
    Particle *p= *it;
    for (Particle::FloatKeyIterator kit= p->float_keys_begin();
         kit != p->float_keys_end(); ++kit) {
      double d= p->get_value(*kit);
      if (is_nan(d) || std::abs(d) > std::numeric_limits<double>::max()) {
        IMP_IF_CHECK(USAGE) {
          std::ostringstream oss;
          oss << message << "Particle " << p->get_name()
              << " attribute " << *kit << " has derivative of "
              << d << std::endl;
          message= oss.str();
        }
        IMP_IF_CHECK(NONE) {
          message= "Bad particle value";
        }
      }
    }
  }
  if (!message.empty()) {
    throw ModelException(message.c_str());
  }
}


void Model::validate_incremental_evaluate(const RestraintsTemp &restraints,
                                          bool calc_derivs,
                                          double score) {
  IMP_LOG(TERSE, "Begin checking incremental evaluation"<<std::endl);
  {
    IncreaseIndent ii;
    std::vector<internal::ParticleStorage::DerivativeTable>
      derivs;
    derivs.reserve(get_number_of_particles());
    for (ParticleIterator it= particles_begin();
         it != particles_end(); ++it) {
      derivs.push_back((*it)->ps_->derivatives_);
      (*it)->zero_derivatives();
    }
    double nscore= do_evaluate_restraints(restraints,
                                          calc_derivs, ALL, false);
    IMP_INTERNAL_CHECK(std::abs(nscore -score)
                       < .001+.1*std::abs(nscore+score),
                       "Incremental and non-incremental evaluation "
                       << "do not agree."
                       << " Incremental gets " << score
                       << " but non-incremental "
                       << "gets " << nscore);
    if (calc_derivs) {
      unsigned int i=0;
      for (ParticleIterator it= particles_begin();
           it != particles_end(); ++it) {
        for (unsigned int j=0; j< derivs[i].get_length(); ++j) {
          IMP_INTERNAL_CHECK(std::abs(derivs[i].get(j)
                                      -(*it)->ps_->derivatives_.get(j))
                             < .01
                             + .01*std::abs(derivs[i].get(j)
                                            +(*it)->ps_->derivatives_.get(j)),
                             "Derivatives do not match in incremental "
                             << "vs non-incremental " <<
                             "for particle " << (*it)->get_name()
                             << " on attribute "
                             << FloatKey(j) << ". Incremental was "
                             << derivs[i].get(j)
                             << " where as regular was "
                             << (*it)->ps_->derivatives_.get(j));
        }
        (*it)->ps_->derivatives_=derivs[i];
        ++i;
      }
      IMP_INTERNAL_CHECK(i== derivs.size(), "Number of particles changed.");
    }
  }
  IMP_LOG(TERSE, "End checking incremental evaluation"<<std::endl);
}

void Model::validate_computed_derivatives() const {
{
    std::string message;
    for (ParticleConstIterator it= particles_begin();
         it != particles_end(); ++it) {
      Particle *p= *it;
      for (Particle::FloatKeyIterator kit= p->float_keys_begin();
           kit != p->float_keys_end(); ++kit) {
        double d= p->get_derivative(*kit);
        if (is_nan(d) || std::abs(d) > std::numeric_limits<double>::max()) {
          IMP_IF_CHECK(USAGE) {
            std::ostringstream oss;
            oss << message << "Particle " << p->get_name()
                << " attribute " << *kit << " has derivative of "
                << d << std::endl;
            message= oss.str();
          }
          IMP_IF_CHECK(NONE) {
            message= "Bad particle derivative";
          }
        }
      }
    }
    if (!message.empty()) {
      throw ModelException(message.c_str());
    }
  }
}



namespace {
  template <class T, int V>
  struct SetIt {
    T *t_;
    SetIt(T *t): t_(t){}
    ~SetIt() {
      *t_= T(V);
    }
  };
}



double Model::do_evaluate(const RestraintsTemp &restraints,
                          const ScoreStatesTemp &states,
                          bool calc_derivs) {
  // validate values
  validate_attribute_values();

  // make sure stage is restored on an exception
  SetIt<Stage, NOT_EVALUATING> reset(&cur_stage_);
  IMP_OBJECT_LOG;
  IMP_CHECK_OBJECT(this);
  IMP_LOG(TERSE,
          "Begin Model::evaluate "
          << (get_is_incremental() && first_incremental_ ? "first ":"")
          << (get_is_incremental()?"incremental":"")
          << std::endl);

  if (incremental_update_ && !last_had_derivatives_ && calc_derivs) {
    first_incremental_=true;
  }

  last_had_derivatives_= calc_derivs;

  before_evaluate(states);

  cur_stage_= EVALUATE;
  double score;
  if (get_is_incremental()) {
    score = 0.0;
    if (calc_derivs) zero_derivatives(first_incremental_);
    score+= do_evaluate_restraints(restraints,
                                   calc_derivs, INCREMENTAL,
                                   !first_incremental_);
    if (calc_derivs) {
      for (ParticleConstIterator pit = particles_begin();
           pit != particles_end(); ++pit) {
        (*pit)->move_derivatives_to_shadow();
      }
    }
    score+=do_evaluate_restraints(restraints,
                                  calc_derivs, NONINCREMENTAL, false);
    for (ParticleConstIterator pit = particles_begin();
         pit != particles_end(); ++pit) {
      if (calc_derivs) (*pit)->accumulate_derivatives_from_shadow();
    }
    first_incremental_=false;
    IMP_IF_CHECK(USAGE_AND_INTERNAL) {
      validate_incremental_evaluate(restraints, calc_derivs, score);
    }
  } else {
    if (calc_derivs) {
      zero_derivatives();
    }
    score= do_evaluate_restraints(restraints,
                                  calc_derivs, ALL, false);
  }

  after_evaluate(states, calc_derivs);

  if (get_is_incremental()) {
    IMP_LOG(TERSE, "Backing up changed particles" << std::endl);
    for (ParticleConstIterator pit = particles_begin();
         pit != particles_end(); ++pit) {
      (*pit)->set_is_not_changed();
    }
  }

  // validate derivatives
  validate_computed_derivatives();

  IMP_LOG(TERSE, "End Model::evaluate. Final score: " << score << std::endl);
  cur_stage_=NOT_EVALUATING;
  return score;
}




Float Model::evaluate(bool calc_derivs) {
  if (!score_states_ordered_) order_score_states();

  return do_evaluate(access_restraints(), access_score_states(), calc_derivs);
}


Float Model::evaluate(const RestraintsTemp &restraints, bool calc_derivs)
{
  if (!score_states_ordered_) order_score_states();
  RequiredStates rs(get_number_of_score_states(), 0);
  ScoreStatesTemp sss;
  for (unsigned int i=0; i< restraints.size(); ++i) {
    rs |= restraint_deps_[restraints[i]];
  }
  ScoreStatesTemp sst;
  for (unsigned int i=0; i< get_number_of_score_states(); ++i) {
    if (rs[i]) sst.push_back(get_score_state(i));
  }
  return do_evaluate(restraints, sst, calc_derivs);
}


void Model::set_is_incremental(bool tf) {
  DerivativeAccumulator da;
  if (tf && !get_is_incremental()) {
    first_incremental_=true;
    for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
      (*it)->setup_incremental();
    }
  } else if (!tf && get_is_incremental()) {
    for (ParticleIterator it= particles_begin(); it != particles_end(); ++it) {
      (*it)->teardown_incremental();
    }
  }
  incremental_update_=tf;
}

void Model::show(std::ostream& out) const
{
  out << std::endl << std::endl;
  out << "Model:" << std::endl;

  get_version_info().show(out);

  out << get_number_of_particles() << " particles" << std::endl;
  out << get_number_of_restraints() << " restraints" << std::endl;
  out << get_number_of_score_states() << " score states" << std::endl;

  out << std::endl;
  IMP_CHECK_OBJECT(this);
}











void Model::set_gather_statistics(bool tf) {
  gather_statistics_=tf;
}


void Model::show_statistics_summary(std::ostream &out) const {
  out << "ScoreStates: running_time_before running_time_after\n";
  for (ScoreStateConstIterator it= score_states_begin();
       it != score_states_end(); ++it) {
    if (stats_data_.find(*it) != stats_data_.end()) {
      out << "  " << (*it)->get_name() << ": ";
      out << stats_data_[*it].total_time_/ stats_data_[*it].calls_
          << "s "
          << stats_data_[*it].total_time_after_/ stats_data_[*it].calls_
          << "s\n";
    }
  }

  out << "Restraints: running_time min_value max_value average_value\n";
  for (RestraintConstIterator it= restraints_begin();
       it != restraints_end(); ++it) {
    if (stats_data_.find(*it) != stats_data_.end()) {
      out << "  " << (*it)->get_name() << ": ";
      out << stats_data_[*it].total_time_/ stats_data_[*it].calls_
          << "s "
          << stats_data_[*it].min_value_ << " "
          << stats_data_[*it].max_value_ << " "
          << stats_data_[*it].total_value_/ stats_data_[*it].calls_ << "\n";
    }
  }
}



IMP_END_NAMESPACE
