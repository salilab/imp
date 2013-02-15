/**
 *  \file mol2.cpp   \brief A class for reading mol2 files
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */
#include <IMP/atom/mol2.h>
#include <IMP/atom/internal/mol2.h>
#include <IMP/atom/Atom.h>
#include <IMP/atom/Mass.h>
#include <IMP/atom/Residue.h>
#include <IMP/atom/Hierarchy.h>
#include <IMP/atom/Charged.h>
#include <IMP/atom/force_fields.h>

#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <sstream>


IMPATOM_BEGIN_NAMESPACE

Mol2Selector::~Mol2Selector(){}




namespace {

  struct Less {
    bool operator()(Particle* pa, Particle* pb) const {
      Bond a(pa), b(pb);
      int a0= Atom(a.get_bonded(0)).get_input_index();
      int a1= Atom(a.get_bonded(1)).get_input_index();
      int b0= Atom(b.get_bonded(0)).get_input_index();
      int b1= Atom(b.get_bonded(1)).get_input_index();
      if (a0 < b0) return true;
      else if (b0 < a0) return false;
      else return a1 < b1;
    }
  };

  std::string mol2_string(Atom a) {
    std::ostringstream mol2_file;
    mol2_file.setf(std::ios::right,std::ios::adjustfield);
    Int atomid = a.get_input_index();
    mol2_file << std::setw(7) << atomid << "  ";
    mol2_file.setf(std::ios::left,std::ios::adjustfield);
    mol2_file << std::setw(1)
              << get_element_table().get_name(a.get_element())[0];
    mol2_file << std::setw(6) << atomid;
    mol2_file.setf(std::ios::right,std::ios::adjustfield);
    mol2_file.setf(std::ios::fixed,std::ios::floatfield);
    core::XYZ xyz(a);
    mol2_file << std::setw(10) << std::setprecision(4) << xyz.get_x();
    mol2_file << std::setw(10) << std::setprecision(4) << xyz.get_y();
    mol2_file << std::setw(10) << std::setprecision(4) << xyz.get_z() << " ";
    mol2_file.setf(std::ios::left,std::ios::adjustfield);
    mol2_file << std::setw(10) << internal::get_mol2_name(a) << "1 MOLE";
    mol2_file.setf(std::ios::right,std::ios::adjustfield);
    mol2_file << std::setw(14) << std::setprecision(4)
              << Charged(a).get_charge() << std::endl;
    return mol2_file.str();
  }


  std::string mol2_string(Bond b, unsigned int count) {
    std::ostringstream oss;
    oss.setf(std::ios::right,std::ios::adjustfield);
    oss << std::setw(6) << (count+1);
    Particle *pa = b.get_bonded(0).get_particle();
    Particle *pb = b.get_bonded(1).get_particle();
    if (Atom::particle_is_instance(pa) && Atom::particle_is_instance(pb)) {
      Atom da(pa);
      Atom db(pb);
      Int atom_aid = da.get_input_index();
      Int atom_bid = db.get_input_index();
      if (atom_aid < atom_bid) {
        oss << std::setw(5) << atom_aid << std::setw(5) << atom_bid;
      }
      else {
        oss << std::setw(5) << atom_bid << std::setw(5) << atom_aid;
      }
      switch (b.get_type()) {
      case Bond::SINGLE:
      case Bond::DOUBLE:
      case Bond::TRIPLE:
        oss << " " << std::setw(1) << b.get_type() << std::endl;
        break;
      case Bond::AMIDE :
        oss << " " << std::setw(2) << "am" << std::endl;
        break;
      case Bond::AROMATIC :
        oss << " " << std::setw(2) << "ar" << std::endl;
        break;
      case Bond::NONBIOLOGICAL :
        oss << " " << std::setw(2) << "du" << std::endl;
        break;
      case Bond::UNKNOWN :
        oss << " " << std::setw(2) << "un" << std::endl;
        break;
      default:
        IMP_WARN("error in bond types. got  " << b.get_type() << std::endl);
      }
    }
    else {
      IMP_THROW("bonded particle(s) is not atom "
                << *pa << " " << *pb, IOException);
    }
    return oss.str();
  }
  std::string mol2_string(Residue r) {
    std::ostringstream oss;
    oss << "@<TRIPOS>MOLECULE" << std::endl;
    oss << r->get_name() << std::endl;
    oss << get_by_type(r, ATOM_TYPE).size() << " "
        << get_internal_bonds(r).size() << std::endl;
    oss << r.get_residue_type().get_string() << std::endl;
    oss << "USER_CHARGES" << std::endl;
    oss << std::endl;
    return oss.str();
  }


  Particle* m2_atom_particle(Model *m, const std::string& mol2_atomline)
  {
    Int atom_number, molecule_number;
    std::string atom_name_field, type_field, molecule_name_field;
    Float x_coord, y_coord, z_coord, atom_charge;

    Particle* p = new Particle(m);

    std::istringstream ins;
    ins.str(mol2_atomline);

    ins >> atom_number >> atom_name_field >> x_coord >> y_coord >> z_coord
        >> type_field >> molecule_number >> molecule_name_field
        >> atom_charge;
    algebra::Vector3D v(x_coord, y_coord, z_coord);

    std::pair<AtomType, internal::Subtype> atom_type
      = internal::get_atom_type_from_mol2(type_field);
    // atom decorator
    Atom d = Atom::setup_particle(p, atom_type.first);
    if (atom_type.second != internal::ST_NONE) {
      p->add_attribute(internal::get_subtype_key(), atom_type.second);
    }
    d->set_name(atom_name_field);
    core::XYZ::setup_particle(p, v);
    d.set_input_index(atom_number);
    Charged::setup_particle(d, atom_charge);
    // get the element from the Sybyl atom type field
    // split on '.' to get the element
    base::Vector<std::string> split_results;
    boost::split(split_results, type_field, boost::is_any_of("."));
    Element e= get_element_table().get_element(split_results[0]);
    d.set_element(e);
    Mass(p).set_mass(get_element_table().get_mass(e));
    return p;
  }

  void bond_particle(Model *, const String& mol2_bondline,
                     const base::map<Int, Particle*>& molecule_atoms)
  {
    //  Particle* p = new Particle(m);
    std::istringstream ins(mol2_bondline);

    Int bond_number, atom_a_id, atom_b_id;
    String bond_type;
    ins >> bond_number >> atom_a_id >> atom_b_id >> bond_type;

    Bond::Type type;
    if (bond_type.compare("1") == 0) {
      type = Bond::SINGLE;
    }
    else if (bond_type.compare("2") == 0) {
      type = Bond::DOUBLE;
    }
    else if (bond_type.compare("3") == 0) {
      type = Bond::TRIPLE;
    }
    else if (bond_type.compare("am") == 0) {
      type = Bond::AMIDE;
    }
    else if (bond_type.compare("ar") == 0) {
      type = Bond::AROMATIC;
    }
    else if (bond_type.compare("du") == 0) {
      type = Bond::NONBIOLOGICAL;
    }
    else if (bond_type.compare("un") == 0) {
      type = Bond::UNKNOWN;
    }
    else {
      IMP_THROW("Error in bond types on " << mol2_bondline,
                IOException);
    }

    // get atom particles back
    Particle* ap = molecule_atoms.find(atom_a_id)->second;
    Particle* bp = molecule_atoms.find(atom_b_id)->second;

    Bonded ad, bd;

    // bonded decorator
    if (Bonded::particle_is_instance(ap)) {
      ad = Bonded(ap);
    }
    else {
      ad = Bonded::setup_particle(ap);
    }

    if (Bonded::particle_is_instance(bp)) {
      bd = Bonded(bp);
    }
    else {
      bd = Bonded::setup_particle(bp);
    }

    // bond decorator
    /*Bond ab_d =*/ create_bond(ad, bd, type);

  }

  Hierarchy molecule_particle(Model *m, const std::string& molecule_name,
                              const std::string& molecule_type)
  {
    Particle* p = new Particle(m);
    ResidueType rt(molecule_type);
    Residue r= Residue::setup_particle(p, rt);
    r->set_name(molecule_name);
    return r;
  }

  Hierarchy root_particle(Model *m, const String& mol2_file_name)
  {
    Particle* p = new Particle(m);

    p->set_name(mol2_file_name);

    // hierarchy decorator
    Hierarchy hd = Hierarchy::setup_particle(p);
    return hd;
  }

  Hierarchy read_molecule_mol2(Model *model, std::istream& mol2_file,
                               Hierarchy& root_d)
  {
    std::string line;
    std::string molecule_name, molecule_type;
    char c;

    for(Int i = 0;i < 6;i++){
      mol2_file.get(c);
      if(c == '@'){
        mol2_file.putback(c);
        break;
      }
      mol2_file.putback(c);
      getline(mol2_file, line);
      if(line.length() == 0){
        break;
      }
      if(i == 0){
        molecule_name = internal::get_molecule_name(line);
      }
      else if(i == 2){
        molecule_type = internal::get_molecule_type(line);
      }
      else{
      }
    }
    Hierarchy molecule_d = molecule_particle(model,
                                             molecule_name,
                                             molecule_type);
    root_d.add_child(molecule_d);

    return molecule_d;
  }

  void read_atom_mol2(Model *model, std::istream& mol2_file,
                      Hierarchy& molecule_d,
                      base::map<Int, Particle*>& molecule_atoms,
                      Mol2Selector* mol2sel)
  {
    IMP::OwnerPointer<Mol2Selector> sel(mol2sel);
    std::string line;
    char c;

    //  while(!mol2_file.eof()){
    do{
      getline(mol2_file, line);
      if(line.length() == 0){
        break;
      }
      else{
        if (mol2sel->get_is_selected(line)){
          Particle* atom_p = m2_atom_particle(model, line);
          Hierarchy atom_d = Hierarchy(atom_p);
          molecule_d.add_child(atom_d);
          Atom ad = Atom(atom_p);
          molecule_atoms[ad.get_input_index()] = atom_p;
        }
      }
      mol2_file.get(c);
      if(c == '@'){
        mol2_file.putback(c);
        break;
      }
      mol2_file.putback(c);
    }
    while(!mol2_file.eof());
  }

  void read_bond_mol2(Model *m, std::istream &mol2_file,
                      Hierarchy& /*molecule_d*/,
                      const base::map<Int, Particle*>& molecule_atoms)
  {
    std::string line;
    char c;
    Int count = 0;

    do{
      mol2_file.get(c);
      if (mol2_file.eof()) {
        break;
      }
      if(c == '@'){
        mol2_file.putback(c);
        break;
      }
      else{
        mol2_file.putback(c);
        getline(mol2_file, line);
        if(line.length() == 0){
          break;
        }
        else{
          if(! internal::is_ATOM_del(line, molecule_atoms)){
            bond_particle(m, line, molecule_atoms);
            count++;
          }
        }
      }
    }
    while(!mol2_file.eof());
  }

  // input molecule decorator, output compound informations to mol2 file
  void write_molecule_mol2(Hierarchy chd, std::ostream& mol2_file) {
    // check if current mhd is molecule
    if (!Residue::particle_is_instance(chd)) {
      IMP_THROW("not a residue" << chd,
                ValueException);
    }

    // get current molecule particle
    Particle* molecule = chd.get_particle();

    // get the MoleculeDecorator of the molecule particle, output mol2 head
    // lines
    // get_mol2head_line should be in MoleculeDecorator.h, .cpp
    if (Residue::particle_is_instance(molecule)) {
      Residue molecule_d = Residue(molecule);
      mol2_file << mol2_string(molecule_d);
    }

    // get AtomDecorator of the atom particles, output atom section
    // get_mol2atom_line should be in AtomDecorator.h, .cpp - to be improved
    ParticlesTemp atoms = get_leaves(chd);
    mol2_file << "@<TRIPOS>ATOM" << std::endl;
    for (unsigned int i=0; i< atoms.size(); i++) {
      if (Atom::particle_is_instance(atoms[i])) {
        Atom atom_d(atoms[i]);
        mol2_file << mol2_string(atom_d);
      }
    }

    // get BondDecorator of the bond particles, output bond section
    // get_mol2bond_line should be in bond_decorators.h, .cpp - to be improved
    Bonds bonds(get_internal_bonds(chd));
    base::Vector<Bond> bds(bonds.begin(), bonds.end());
    std::sort(bds.begin(), bds.end(), Less());
    bonds= Bonds(bds.begin(), bds.end());
    mol2_file << "@<TRIPOS>BOND" << std::endl;
    for (unsigned int i=0; i< bonds.size(); i++) {
      mol2_file << mol2_string(bonds[i], i);
    }
  }

}


Hierarchy read_mol2(base::TextInput mol2_file,
                    Model *model,  Mol2Selector* mol2sel)
{
  if (!mol2sel) {
    mol2sel=new AllMol2Selector();
  }
  IMP::OwnerPointer<Mol2Selector> sel(mol2sel);
  // create a map to save atom_index and atom particle pairs
  base::map<Int, Particle*>molecule_atoms;

  // create root particle
  Hierarchy root_d = root_particle(model, mol2_file.get_name());
  std::string line;
  Hierarchy molecule_d;
  while (std::getline(mol2_file.get_stream(), line)) {
    // check the line is the title line @<TRIPOS>MOLECULE
    if (internal::is_MOLECULE_rec(line)) {
      molecule_atoms.clear();
      molecule_d = read_molecule_mol2(model, mol2_file, root_d);
    }
    // check the starting line of atom block @<TRIPOS>ATOM
    else if (internal::is_MOL2ATOM_rec(line)) {
      if (!molecule_d) {
        IMP_THROW("Atom seen before molecule on line " << line,
                  IOException);
      }
      read_atom_mol2(model, mol2_file, molecule_d, molecule_atoms, mol2sel);
    }
    // check the starting line of bond block @<TRIPOS>BOND
    else if (internal::is_BOND_rec(line)) {
      read_bond_mol2(model, mol2_file, molecule_d, molecule_atoms);
    }
    else {
      IMP_LOG_TERSE( "Couldn't parse line " << line << std::endl);
    }
  }
  //Hierarchies mps = get_by_type(root_d, RESIDUE_TYPE);
  //  std::cout << "check " << mps.size() << std::endl;
  add_radii(root_d);
  IMP_INTERNAL_CHECK(root_d.get_is_valid(true), "Invalid hierarchy produced");
  return root_d;
}

// argv[1] file_name_type should contain file type e.g. ".mol2"
void write_mol2(Hierarchy rhd, base::TextOutput file) {
  Hierarchies hs= get_by_type(rhd, RESIDUE_TYPE);
  for(unsigned int i=0; i<hs.size(); ++i) {
    write_molecule_mol2(hs[i], file);
  }
}

IMPATOM_END_NAMESPACE
