namespace IMP::em {

/* Provide our own implementations for operator[] */
%ignore Vector3::operator[];

%extend Vector3 {
  float __getitem__(unsigned int coord) const {
    return self->operator[](coord);
  }
};
%ignore Vector3::operator =;
}

%ignore operator<<(std::ostream&, const Vector3 &);
%ignore operator>>(std::istream&, Vector3 &);
%rename(sub_two_vector3) operator-(const Vector3 &, const Vector3 &);
%rename(add_two_vector3) operator+(const Vector3 &, const Vector3 &);
%rename(mult_two_vector3) operator*(const Vector3& p1, const Vector3& p2);
%rename(mult_vector3_real) operator*(const Vector3& p, const real& m);
%rename(mult_real_vector3) operator*(const real &m, const Vector3& p);
%rename (divide_vector3_real) operator/(const Vector3& p, const real& m);
%rename (vectical_vector3_vector3) operator&(const Vector3& p1, const Vector3& p2);
%rename (dist_two_vector3) operator|(const Vector3& p1, const Vector3& p2);
%rename(angle_two_vector3) operator^(const Vector3& p1, const Vector3& p2);
%rename(minus_vector3) operator-(const Vector3 &p);

%include "IMP/em/Vector3.h"
