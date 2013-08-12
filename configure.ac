AC_INIT(fastprefix, 0.1)
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile
                datamanager/Makefile
                csvimport/Makefile
                src/Makefile])

OLD_CXXFLAGS=$CXXFLAGS
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
CXXFLAGS=$OLD_CXXFLAGS

AC_OUTPUT

