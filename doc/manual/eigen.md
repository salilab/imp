Eigen {#eigen}
=====

We ship a copy of Eigen. To patch an updated version, first commit the unpatched version and then patch it by running all of these. More clever approaches are welcome.

    sed -i.old "s#namespace Eigen#namespace IMP_Eigen#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#,Eigen::#,IMP_Eigen::#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s# Eigen::# IMP_Eigen::#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#(Eigen::#(IMP_Eigen::#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#{Eigen::#{IMP_Eigen::#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#=Eigen::#=IMP_Eigen::#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#<Eigen::#<IMP_Eigen::#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#<EIGEN_#<IMP_EIGEN_#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s# EIGEN# IMP_EIGEN#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#^EIGEN#IMP_EIGEN#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#\!EIGEN#\!IMP_EIGEN#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#(EIGEN#(IMP_EIGEN#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#{EIGEN#{IMP_EIGEN#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#,EIGEN#,IMP_EIGEN#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#>EIGEN#>IMP_EIGEN#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#\!Eigen#\!IMP_Eigen#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#\*Eigen#*IMP_Eigen#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#=EIGEN#=IMP_EIGEN#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#ei_declare_aligned_stack_constructed_variable#imp_ei_declare_aligned_stack_constructed_variable#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#\!Eigen::#\!IMP_Eigen::#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#eigen_assert#imp_eigen_assert#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#eigen_internal_assert#imp_eigen_internal_assert#g" `find  modules/algebra/include/eigen3 -type f`
    sed -i.old "s#eigen_plain_assert#imp_eigen_plain_assert#g" `find  modules/algebra/include/eigen3 -type f`

in addition, the following commits introduced some modifications to the code
- `26a1f70d5faef3f`
- `a81c11affba3889`

Eigen headers can be found at `IMP/algebra/eigen3`.
