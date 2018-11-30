/**
 *  \file mmcif.cpp
 *  \brief Functions to read PDBs in mmCIF format
 *
 *  Copyright 2007-2018 IMP Inventors. All rights reserved.
 *
 */

#include <IMP/atom/mmcif.h>
#include <IMP/atom/Chain.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/internal/pdb.h>
#include "ihm_format.h"

#include <boost/version.hpp>
#if BOOST_VERSION >= 105000
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
#else
#define BOOST_FILESYSTEM_VERSION 2
#include <boost/filesystem/path.hpp>
#endif
#include <boost/lexical_cast.hpp>

extern "C" {
#include "ihm_format.c"
}


IMPATOM_BEGIN_NAMESPACE

namespace {

class Category {
protected:
  struct ihm_category *c_;
public:
  Category(struct ihm_reader *reader, const char *name,
           ihm_category_callback callback) :
    c_(ihm_category_new(reader, name, callback, NULL, NULL, this, NULL)) {}
};


class Keyword {
  struct ihm_keyword *k_;
public:
  Keyword(struct ihm_category *c, std::string name)
      : k_(ihm_keyword_new(c, name.c_str())) {}

  const char *data() { return k_->data; }

  const char *as_str() {
    if (k_->omitted || k_->unknown || !k_->in_file) {
      return "";
    } else {
      return k_->data;
    }
  }

  float as_float(float default_value=0.) {
    if (k_->omitted || k_->unknown || !k_->in_file) {
      return default_value;
    } else {
      return boost::lexical_cast<float>(k_->data);
    }
  }

  int as_int(int default_value=0) {
    if (k_->omitted || k_->unknown || !k_->in_file) {
      return default_value;
    } else {
      return boost::lexical_cast<int>(k_->data);
    }
  }
};


class AtomSiteCategory : public Category {
  std::string name_, filename_;
  Model *model_;
  bool select_first_model_;
  Keyword atom_name_, residue_name_, chain_, element_, seq_id_, group_, id_,
          occupancy_, temp_factor_, ins_code_, x_, y_, z_, model_num_,
          auth_seq_id_;
  Particle *cp_, *rp_, *root_p_;
  Hierarchies *hiers_;
  std::string curr_chain_;
  int curr_seq_id_;
  int curr_auth_seq_id_;
  int curr_model_num_;
  std::string curr_residue_icode_;
  std::string hetatm_;
  std::map<std::pair<Particle *, std::string>, Particle *> chain_map_;
  std::map<int, Particle *> root_map_;

  static void callback(struct ihm_reader *, void *data, struct ihm_error **) {
    ((AtomSiteCategory *)data)->handle();
  }

public:
  AtomSiteCategory(struct ihm_reader *reader, std::string name,
                   std::string filename, Model *model, Hierarchies *hiers,
                   bool select_first_model) :
        Category(reader, "_atom_site", callback),
        name_(name), filename_(filename), model_(model),
        select_first_model_(select_first_model),
        atom_name_(c_, "label_atom_id"),
        residue_name_(c_, "label_comp_id"),
        chain_(c_, "label_asym_id"),
        element_(c_, "type_symbol"),
        seq_id_(c_, "label_seq_id"),
        group_(c_, "group_pdb"),
        id_(c_, "id"),
        occupancy_(c_, "occupancy"),
        temp_factor_(c_, "b_iso_or_equiv"),
        ins_code_(c_, "pdbx_pdb_ins_code"),
        x_(c_, "cartn_x"),
        y_(c_, "cartn_y"),
        z_(c_, "cartn_z"),
        model_num_(c_, "pdbx_pdb_model_num"),
        auth_seq_id_(c_, "auth_seq_id"),
        cp_(nullptr), rp_(nullptr), root_p_(nullptr),
        hiers_(hiers) {
    curr_chain_ = "";
    curr_seq_id_ = 0;
    curr_auth_seq_id_ = 0;
    curr_residue_icode_ = "";
    curr_model_num_ = 0;
    hetatm_ = "HETATM";
  }

  void set_root_particle_name(int model_num) {
    std::ostringstream oss;
    oss << model_num;
    std::string root_name = oss.str();
    root_p_->set_name(name_ + ": " + root_name);
  }

  // Get the particle for the top of this model's hierarchy.
  // Return false if this model should be skipped.
  bool get_root_particle(int model_num) {
    if (root_p_ == nullptr || model_num != curr_model_num_) {
      if (select_first_model_ && root_p_ != nullptr) {
        return false;
      }
      curr_model_num_ = model_num;
      // Check if new model
      if (root_map_.find(model_num) == root_map_.end()) {
        root_p_ = new Particle(model_);
        set_root_particle_name(model_num);
        hiers_->push_back(Hierarchy::setup_particle(root_p_));
        root_map_[model_num] = root_p_;
      } else {
        root_p_ = root_map_[model_num];
      }
      cp_ = nullptr; // make sure we get a new chain
    }
    return true;
  }

  void get_chain_particle(const std::string &chain) {
    if (cp_ == nullptr || chain != curr_chain_) {
      curr_chain_ = chain;
      std::pair<Particle *, std::string> root_chain(root_p_, chain);
      // Check if new chain (for this root)
      if (chain_map_.find(root_chain) == chain_map_.end()) {
        cp_ = internal::chain_particle(model_, chain, filename_);
        Hierarchy(root_p_).add_child(Chain(cp_));
        chain_map_[root_chain] = cp_;
      } else {
        cp_ = chain_map_[root_chain];
      }
      rp_ = nullptr; // make sure we get a new residue
    }
  }

  void handle() {
    if (!get_root_particle(model_num_.as_int())) {
      return;
    }

    Element e = get_element_table().get_element(element_.as_str());
    int seq_id = seq_id_.as_int(1);
    std::string residue_icode = ins_code_.as_str();

    get_chain_particle(chain_.as_str());
    // Check if new residue
    if (rp_ == nullptr || seq_id != curr_seq_id_
        || residue_icode != curr_residue_icode_) {
      curr_seq_id_ = seq_id;
      curr_residue_icode_ = residue_icode;
      // use author-provided seq_id and insertion code if available
      std::string si = auth_seq_id_.as_str();
      const char *start = si.c_str();
      char *endptr;
      int auth_seq_id = strtol(start, &endptr, 10);
      // if auth_seq_id is blank, use seq_id instead
      if (endptr == start) auth_seq_id = seq_id;
      char one_icode = 32; // default insertion code (space)
      // if auth_seq_id is not blank and contains something after the number,
      // use the first character of that as the insertion code
      if (endptr != start && *endptr) {
        one_icode = *endptr;
      }
      curr_auth_seq_id_ = auth_seq_id;
      rp_ = internal::residue_particle(model_, auth_seq_id, one_icode,
                                       residue_name_.as_str());
      Chain(cp_).add_child(Residue(rp_));
    }
    Particle *ap = internal::atom_particle(
                       model_, atom_name_.as_str(), e,
                       group_.as_str() == hetatm_, id_.as_int(),
                       curr_auth_seq_id_, x_.as_float(), y_.as_float(),
                       z_.as_float(), occupancy_.as_float(),
                       temp_factor_.as_float());
    Residue(rp_).add_child(Atom(ap));
  }
};


std::string cif_nicename(std::string name) {
  boost::filesystem::path path(name);
#if BOOST_VERSION >= 105000
  return path.string();
#else
  return path.filename();
#endif
}

ssize_t read_callback(char *buffer, size_t buffer_len,
                      void *data, struct ihm_error **err)
{
  std::istream *in = (std::istream *)data;
  in->read(buffer, buffer_len);
  if (in) {
    return in->gcount();
  } else {
    ihm_error_set(err, IHM_ERROR_IO, "IHM IO error");
    return -1;
  }
}

Hierarchies read_mmcif(std::istream& in, std::string name, std::string filename,
                       Model* model, bool select_first_model)
{
  struct ihm_error *err = nullptr;
  struct ihm_file *fh = ihm_file_new(read_callback, &in, nullptr);

  struct ihm_reader *r = ihm_reader_new(fh);
  Hierarchies ret;

  AtomSiteCategory asc(r, name, filename, model, &ret, select_first_model);

  int more_data;
  if (!ihm_read_file(r, &more_data, &err)) {
    std::string errmsg(err->msg);
    ihm_error_free(err);
    ihm_reader_free(r);
    IMP_THROW(errmsg, IOException);
  }
  ihm_reader_free(r);
  return ret;
}

} // anonymous namespace

Hierarchies read_multimodel_mmcif(TextInput in, Model *model)
{
  Hierarchies ret = read_mmcif(in, cif_nicename(in.get_name()), in.get_name(),
                               model, false);
  if (ret.empty()) {
    IMP_THROW("No molecule read from file " << in.get_name(), ValueException);
  }
  return ret;
}

Hierarchy read_mmcif(TextInput in, Model *model)
{
  Hierarchies ret = read_mmcif(in, cif_nicename(in.get_name()), in.get_name(),
                               model, true);
  if (ret.empty()) {
    IMP_THROW("No molecule read from file " << in.get_name(), ValueException);
  }
  return ret[0];
}

IMPATOM_END_NAMESPACE
