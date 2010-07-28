"""Simple configure checks for NetCDF"""
import checks

def _check(context):
    cgal = context.env['netcdf']
    if cgal is False or cgal is 0:
        context.Message('Checking for NetCDF ...')
        context.Result("disabled")
        return False

    ret= checks.check_lib(context, lib='netcdf_c++', header=['netcdfcpp.h'],
                          body='',
                          extra_libs=['netcdf'])
    context.Message('Checking for NetCDF ...')
    if ret[0]:
        context.Result(" ".join(ret[1]))
    else:
        context.Result(ret[0])
    if ret[0]:
        context.env['NETCDF_LIBS'] = ret[1]
        #context.env.Append(CPPDEFINES=['IMP_USE_CGAL'])
    return ret[0]

def configure_check(env):
    env['NETCDF_LIBS'] = ['']
    custom_tests = {'NetCDFCheck':_check}
    conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
    if conf.NetCDFCheck():
        env.Append(IMP_BUILD_SUMMARY=["NetCDF support enabled."])
    else:
        env.Append(IMP_BUILD_SUMMARY=["NetCDF support disabled."])
    conf.Finish()
