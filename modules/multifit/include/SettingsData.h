/**
 *  \file IMP/multifit/SettingsData.h
 *  \brief stored multifit settings data
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPMULTIFIT_SETTINGS_DATA_H
#define IMPMULTIFIT_SETTINGS_DATA_H
#include <IMP/multifit/multifit_config.h>
#include <IMP/base_types.h>
#include <IMP/container_macros.h>
#include <IMP/base/Pointer.h>
#include <IMP/Model.h>
#include <IMP/base/Pointer.h>
#include <IMP/algebra/Transformation3D.h>

IMPMULTIFIT_BEGIN_NAMESPACE

//! Holds data about a component needed for optimization
class IMPMULTIFITEXPORT ComponentHeader : public IMP::base::Object {
  public:
  ComponentHeader(): Object("ComponentHeader%1%") {
      name_="";
      filename_="";
      surface_fn_="";
      pdb_ap_fn_="";
      pdb_fine_ap_fn_="";
      num_ap_=0;
      transformations_fn_="";
      reference_fn_="";
    }
    void set_name(const std::string &name) {name_=name;}
    inline std::string get_name() const {return name_;}
    inline std::string get_filename() const {return filename_;}
    void set_filename(const std::string &filename){filename_=filename;}
    inline std::string get_surface_fn() const {return surface_fn_;}
    void set_surface_fn(const std::string &fn){surface_fn_=fn;}
    void set_txt_ap_fn(const std::string &pdb_ap_fn) {pdb_ap_fn_=pdb_ap_fn;}
    inline std::string get_txt_ap_fn() const {return pdb_ap_fn_;}
    void set_txt_fine_ap_fn(const std::string &pdb_ap_fn) {
      pdb_fine_ap_fn_=pdb_ap_fn;}
    inline std::string get_txt_fine_ap_fn() const {
      return pdb_fine_ap_fn_;}
    void set_num_ap(int num_ap) {num_ap_=num_ap;}
    inline int get_num_ap() const {return num_ap_;}
    void set_num_fine_ap(int num_ap) {num_fine_ap_=num_ap;}
    inline int get_num_fine_ap() const {return num_fine_ap_;}
    void set_transformations_fn(std::string transformations_fn)
     { transformations_fn_=transformations_fn;}
    std::string get_transformations_fn() const {return transformations_fn_;}
    void set_reference_fn(const std::string &ref_fn){reference_fn_=ref_fn;}
    std::string get_reference_fn() const {return reference_fn_;}
    IMP_OBJECT_METHODS(ComponentHeader);
    /*IMP_OBJECT_INLINE(ComponentHeader, {
        out<<name_<<"|"<<filename_<<"|"<<surface_fn_<<"|";
        out<<pdb_ap_fn_<<"|"<<num_ap_<<"|";
        out<<pdb_fine_ap_fn_<<"|"<<num_fine_ap_<<"|";
        out<<transformations_fn_<<"|"<<reference_fn_<<"|"<<std::endl; }, {});*/
  protected:
    std::string name_;
    std::string filename_;
    std::string surface_fn_;
    std::string pdb_ap_fn_;
    int num_ap_;
    std::string pdb_fine_ap_fn_;
    int num_fine_ap_;
    std::string transformations_fn_;
    std::string reference_fn_;
};
IMP_OBJECTS(ComponentHeader, ComponentHeaders);

//! Holds data about the assembly density needed for optimization
class IMPMULTIFITEXPORT AssemblyHeader : public IMP::base::Object {
  public:
  AssemblyHeader(): Object("AssemblyHeader%1%") {
      dens_fn_="";
      resolution_=0.;
      spacing_=0.;
      threshold_=0.;
      coarse_ap_fn_="";
      coarse_over_sampled_ap_fn_="";
      fine_ap_fn_="";
      fine_over_sampled_ap_fn_="";
    }
    void set_dens_fn(const std::string &dens_fn) {dens_fn_=dens_fn;}
    std::string get_dens_fn() const {return dens_fn_;}
    void set_resolution(float res) {resolution_=res;}
    float get_resolution() const {return resolution_;}
    float get_spacing() const {return spacing_;}
    void set_spacing(float spacing) {spacing_=spacing;}
    void set_threshold(float t) {threshold_=t;}
    float get_threshold() const {return threshold_;}
    algebra::Vector3D get_origin() const {return origin_;}
    void set_origin(algebra::Vector3D origin) {origin_=origin;}
    std::string get_coarse_ap_fn () const {return coarse_ap_fn_;}
    void set_coarse_ap_fn (const std::string &new_fn) {
      coarse_ap_fn_ = new_fn;}
    std::string get_coarse_over_sampled_ap_fn () const {
      return coarse_over_sampled_ap_fn_;}
    void set_coarse_over_sampled_ap_fn (const std::string &new_fn) {
      coarse_over_sampled_ap_fn_=new_fn;}
    std::string get_fine_ap_fn () const {return fine_ap_fn_;}
    void set_fine_ap_fn (const std::string &new_fn) {
      fine_ap_fn_ = new_fn;}
    std::string get_fine_over_sampled_ap_fn () const {
      return fine_over_sampled_ap_fn_;}
    void set_fine_over_sampled_ap_fn (const std::string &new_fn) {
      fine_over_sampled_ap_fn_=new_fn;}
    IMP_OBJECT_METHODS(AssemblyHeader);
    /*IMP_OBJECT_INLINE(AssemblyHeader, {
        out<<dens_fn_<<"|"<<resolution_<<"|"<<spacing_<<"|"<<threshold_
           <<"|"<<origin_[0]<<"|";
        out<<origin_[1]<<"|"<<origin_[2]<<"|"<<coarse_ap_fn_;
        out<<"|"<<coarse_over_sampled_ap_fn_<<"|";
        out<<fine_ap_fn_<<"|"<<fine_over_sampled_ap_fn_<<"|\n";
        }, {});*/
  protected:
    std::string dens_fn_;
    float resolution_;
    float spacing_;
    float threshold_;
    algebra::Vector3D origin_;
    std::string coarse_ap_fn_;
    std::string coarse_over_sampled_ap_fn_;
    std::string fine_ap_fn_;
    std::string fine_over_sampled_ap_fn_;
  };

//! Holds header data for optimization
class IMPMULTIFITEXPORT SettingsData : public IMP::base::Object {
public:
  SettingsData(): Object("SettingsData%1%"){
    data_path_="./";}
  static void show_component_header_line(std::ostream& out = std::cout)  {
    out<<get_component_header_line();
  }
  static std::string get_component_header_line(){
    std::stringstream ss;
    ss<<"name|protein|surface|pdb_anchor_points|number of anchor points|"<<
      "fine pdb_anchor_points|number of fine anchor points|"<<
      "transformations|ref filename|"<<std::endl;
    return ss.str();
  }
  static void show_density_header_line(std::ostream& out = std::cout)  {
    out <<get_density_header_line();
  }
  static std::string get_density_header_line()  {
    std::stringstream ss;
    ss <<"map| resolution| spacing| threshold|x-origin| y-origin| z-origin|";
    ss<<"coarse anchor points|coarse over sampled anchor points|";
    ss<<"fine anchor points|fine over sampled anchor points|"<<std::endl;
    return ss.str();
  }
  void set_assembly_filename(const std::string  &fn) {
    asmb_fn_ = fn;
  }
  const char * get_assembly_filename() const {
    return asmb_fn_.c_str();
  }
  void set_assembly_header(AssemblyHeader *h) {
    dens_data_=h;
  }
  AssemblyHeader *get_assembly_header() const {
    return dens_data_;
  }
  void set_data_path(const std::string &fn) {
    data_path_ = fn;
  }
  std::string get_data_path() const {return data_path_;}

  IMP_OBJECT_METHODS(SettingsData);

  IMP_LIST_ACTION(public, ComponentHeader, ComponentHeaders,
                  component_header, component_headers,
                  ComponentHeader*,
                  ComponentHeaders, obj->set_was_used(true);,,);

protected:
  IMP::base::PointerMember<AssemblyHeader> dens_data_;
  std::string asmb_fn_;
  std::string data_path_;
};

IMPMULTIFITEXPORT SettingsData *read_settings(const char *filename);
IMPMULTIFITEXPORT void write_settings(
   const char *filename, const SettingsData *sd);
IMPMULTIFIT_END_NAMESPACE
#endif /* IMPMULTIFIT_SETTINGS_DATA_H */
