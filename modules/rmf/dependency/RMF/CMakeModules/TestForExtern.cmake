INCLUDE (CheckCXXSourceCompiles)

CHECK_CXX_SOURCE_COMPILES("    template <class T>
    class Foo{};
    extern template class Foo<int>;
    template class Foo<int>;
    int main()
        {
            return 0;
        }
"
 RMF_HAS_EXTERN)

if(${RMF_HAS_EXTERN} MATCHES 1)
# does not work on for some reason with gcc 4.6
set(RMF_HAS_EXTERN 0)
else()
set(RMF_HAS_EXTERN 0)
endif()
