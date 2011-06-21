/**
 *  \file Particle.cpp   \brief Classes to handle individual model particles.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#include "IMP/Particle.h"
#include "IMP/log.h"
#include "IMP/Model.h"
#include "IMP/internal/utility.h"
#include "IMP/internal/PrefixStream.h"


#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#if BOOST_VERSION > 103900
#include <boost/property_map/property_map.hpp>
#else
#include <boost/property_map.hpp>
#include <boost/vector_property_map.hpp>
#endif

IMP_BEGIN_INTERNAL_NAMESPACE
ReadLockedParticleException
::ReadLockedParticleException(const Particle *p): p_(p){}
ReadLockedParticleException
::~ReadLockedParticleException() throw(){}

WriteLockedParticleException
::WriteLockedParticleException(const Particle *p): p_(p){}
WriteLockedParticleException
::~WriteLockedParticleException() throw(){}


ParticleStorage::~ParticleStorage(){
}
IMP_END_INTERNAL_NAMESPACE

IMP_BEGIN_NAMESPACE

Particle::Particle(Model *m, std::string name):
  Container(m, name),
  ps_(new internal::ParticleStorage()),
  dirty_(false)
{
  m->add_particle_internal(this);
}

void Particle::zero_derivatives()
{
  derivatives_.fill(0);
  ps_->derivatives_.fill(0);
}

void Particle::do_show(std::ostream& out) const
{
  internal::PrefixStream preout(&out);
  preout << (get_is_active()? " (active)":" (dead)");
  preout << std::endl;

  if (get_has_model()) {
    preout << "float attributes:" << std::endl;
    preout.set_prefix("  ");
    for (FloatKeyIterator it= float_keys_begin(); it != float_keys_end();
         ++it) {
      FloatKey k =*it;
      preout << k << ": " << get_value(k);
      if (k.get_index() < IMP_NUM_INLINE) {
        if (derivatives_.fits(k.get_index())){
          preout << " ("
                 << derivatives_.get(k.get_index()) << ") ";
        } else {
          preout << " (-) ";
        }
      } else {
        if (ps_->derivatives_.fits(k.get_index())){
          preout << " ("
                 << ps_->derivatives_.get(k.get_index()) << ") ";
        } else {
          preout << " (-) ";
        }
      }
      preout << (get_is_optimized(k)?" (optimized)":"");
      preout << std::endl;
    }


    preout.set_prefix("");
    out << "int attributes:" << std::endl;
    preout.set_prefix("  ");
    for (IntKeyIterator it= int_keys_begin(); it != int_keys_end();
         ++it) {
      preout << *it << ": " << get_value(*it) << std::endl;
    }

    preout.set_prefix("");
    out << "string attributes:" << std::endl;
    preout.set_prefix("  ");
    for (StringKeyIterator it= string_keys_begin(); it != string_keys_end();
         ++it) {
      preout << *it << ": " << get_value(*it) << std::endl;
    }

    preout.set_prefix("");
    out << "particle attributes:" << std::endl;
    preout.set_prefix("  ");
    for (ParticleKeyIterator it= particle_keys_begin();
         it != particle_keys_end();
         ++it) {
      preout << *it << ": " << get_value(*it)->get_name() << std::endl;
    }

    preout.set_prefix("");
    out << "object attributes:" << std::endl;
    preout.set_prefix("  ");
    for (ObjectKeyIterator it= object_keys_begin(); it != object_keys_end();
         ++it) {
      preout << *it << ": " << get_value(*it)->get_name() << std::endl;
    }
  }
}



ContainersTemp Particle::get_input_containers() const {return ContainersTemp();}
bool Particle::get_contained_particles_changed() const {
  return false;
}
ParticlesTemp Particle::get_contained_particles() const {
  ParticlesTemp ret;
  for (ParticleKeyIterator it= particle_keys_begin();
       it != particle_keys_end(); ++it) {
    ret.push_back(get_value(*it));
  }
  return ret;
}

void Particle::clear_caches() {
  for (unsigned int i=0; i< ps_->cache_objects_.size(); ++i) {
    ps_->objects_.remove(ps_->cache_objects_[i].get_index());
  }
  ps_->cache_objects_.clear();
}

#if 0
namespace {
  const std::size_t num_blocks=15000;
  const std::size_t int_size= sizeof(int);
  struct Chunk {
    char particles[num_blocks*sizeof(Particle)];
    std::vector<unsigned int> free_list;
    unsigned int next_to_allocate;
    Chunk(): next_to_allocate(0){
#if IMP_BUILD == IMP_DEBUG
      // ramdomize the particle order in debug so that
      // assuming the particles are ordered will not work
      // most of the time
      for (unsigned int i=0; i< num_blocks; ++i) {
        free_list.push_back(i);
        next_to_allocate=i+1;
      }
      std::random_shuffle(free_list.begin(), free_list.end());
#endif
    }
  };
  std::vector<Chunk*> *chunks=NULL;
  unsigned int block_size() {
    return sizeof(Particle);
  }
  unsigned int offset(unsigned int i, void *p) {
    return static_cast<char*>(p)- (*chunks)[i]->particles;
  }
  unsigned int index(unsigned int i, void *p) {
    IMP_INTERNAL_CHECK(offset(i, p) % block_size() ==0,
               "There are alignment issues");
    return offset(i, p)/block_size();
  }
  void *address(unsigned int i, unsigned int j) {
    return (*chunks)[i]->particles+j*block_size();
  }
}


void *Particle::operator new(std::size_t sz) {
  IMP_INTERNAL_CHECK(sz <= block_size(),
             "Expected request of size " << block_size()
             << " got request of size " << sz);
  if (!chunks) {
    chunks=new std::vector<Chunk*>();
    IMP_LOG(MEMORY, "Created particle chunks" << std::endl);
  }
  unsigned int i=0;
  for (; i< chunks->size(); ++i) {
    if ((*chunks)[i]->free_list.empty()
        && (*chunks)[i]->next_to_allocate==num_blocks) {
      continue;
    } else {
      break;
    }
  }
  if (i== chunks->size()) {
    chunks->push_back(new Chunk());
    IMP_LOG(MEMORY, "Creating particle chunk "
            << chunks->size()-1 << std::endl);
  }
  unsigned int slot;
  if (!(*chunks)[i]->free_list.empty()) {
    slot= (*chunks)[i]->free_list.back();
    (*chunks)[i]->free_list.pop_back();
  } else {
    slot= (*chunks)[i]->next_to_allocate;
    ++(*chunks)[i]->next_to_allocate;
  }
  return address(i, slot);
}

void *Particle::operator new(std::size_t, void*p) {
  return p;
}

void Particle::operator delete(void *p) {
  for (unsigned int i=0; i< chunks->size(); ++i) {
    if (&(*chunks)[i]->particles[0] <= p
        && &(*chunks)[i]->particles[(num_blocks-1)*sizeof(Particle)]>= p) {
      (*chunks)[i]->free_list.push_back(index(i, p));
      if ((*chunks)[i]->free_list.size() == (*chunks)[i]->next_to_allocate) {
        IMP_LOG(MEMORY, "Deleting particle chunk " << i << std::endl);
        delete (*chunks)[i];
        chunks->erase(chunks->begin()+i);
        if (chunks->empty()) {
          delete chunks;
          chunks=NULL;
          IMP_LOG(MEMORY, "Deleted particle chunks" << std::endl);
        }
      } else {
        IMP_LOG(MEMORY, "Chunk " << i << " has "
                << (*chunks)[i]->next_to_allocate
                - (*chunks)[i]->free_list.size()<< std::endl);
      }
      return;
    }
  }
  IMP_FAILURE("Particle being deleted was not allocated properly.");
}
#endif

/*
namespace internal {
  Particle* create_particles(Model *m, unsigned int n) {
    IMP_USAGE_CHECK(n>0, "Can't create 0 particles");
    if (!chunks) {
      chunks=new std::vector<Chunk*>();
    }
    unsigned int i=0;
    for (; i< chunks->size(); ++i) {
      if ((*chunks)[i]->next_to_allocate + n > num_blocks) {
        continue;
      } else {
        break;
      }
    }
    if (i== chunks->size()) {
      chunks->push_back(new Chunk());
      IMP_LOG(MEMORY, "Creating particle chunk "
              << chunks->size()-1 << std::endl);
    }
    for (unsigned int j=0; j< n; ++j) {
      Particle *cur= new(address(i, (*chunks)[i]->next_to_allocate+j))
        Particle(m);
      if (0) std::cout << cur;
    }
    Particle *ret
      = static_cast<Particle*>(address(i, (*chunks)[i]->next_to_allocate));
    (*chunks)[i]->next_to_allocate+=n;
    return ret;
  }
  }*/

namespace {
  IMP_GRAPH(ParticleGraph, directed, Particle*, int);
}

class CycleVisitor: public boost::default_dfs_visitor {
  std::vector<unsigned int> stack_;
  boost::property_map<ParticleGraph,
                      boost::vertex_name_t>::const_type vm_;
public:
  CycleVisitor(const ParticleGraph &g):
    vm_(boost::get(boost::vertex_name, g)){}
  void discover_vertex( boost::graph_traits<ParticleGraph>::vertex_descriptor u,
                       const ParticleGraph&) {
    stack_.push_back(u);
  }
  void finish_vertex( boost::graph_traits<ParticleGraph>::vertex_descriptor u,
                       const ParticleGraph&) {
    IMP_USAGE_CHECK(stack_.back()==u, "Expected " << stack_.back()
                   << " but found " << u << std::endl);
    stack_.pop_back();
  }
  void back_edge( boost::graph_traits<ParticleGraph>::edge_descriptor e,
                       const ParticleGraph&g) {
    std::cout << "Ref counted cycle: ";
    //unsigned int source= boost::source(e, g);
    unsigned int target= boost::target(e, g);
    bool print=false;
    for (unsigned int i=0; i < stack_.size(); ++i) {
      if (stack_[i]== target) {
        print=true;
      }
      if (print) {
        std::cout << vm_[stack_[i]]->get_name() << "--";
      }
    }
    std::cout << vm_[target]->get_name() << "--" << std::endl;
  }
};

void show_ref_counting_cycles(const ParticlesTemp &ps) {
  ParticleGraph pg;
  internal::Map<Particle*, int> pvm;
  boost::property_map<ParticleGraph, boost::vertex_name_t>::type
    vpm = boost::get(boost::vertex_name, pg);
  for (unsigned int i=0; i < ps.size(); ++i) {
    int vi= boost::add_vertex(pg);
    vpm[vi]= ps[i];
    pvm[ps[i]] = vi;
  }
  for (unsigned int i=0; i< ps.size(); ++i) {
    for (Particle::ParticleKeyIterator pki= ps[i]->particle_keys_begin();
         pki != ps[i]->particle_keys_end(); ++pki) {
      if (ps[i]->get_is_ref_counted(*pki)) {
        boost::add_edge(pvm[ps[i]], pvm[ps[i]->get_value(*pki)], pg);
      }
    }
  }
  CycleVisitor cv(pg);
  boost::vector_property_map<int> color(boost::num_vertices(pg));
  boost::depth_first_search(pg, cv, color);
}

IMP_END_NAMESPACE
