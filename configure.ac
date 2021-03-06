# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([litl], [0.1.9], [litl-devel@fusionforge.int-evry.fr])
AM_INIT_AUTOMAKE([1.10 tar-pax color-tests])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([m], [ceil])
AC_CHECK_LIB([rt], [clock_gettime])

AC_CONFIG_MACRO_DIR([m4])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h stdio.h string.h math.h time.h sys/stat.h \
  unistd.h sys/syscall.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

AC_DEFUN([wrp_CHECK_SIZEOF_AND_SUBST], [
    AC_REQUIRE([AC_PROG_AWK])
    AC_CHECK_SIZEOF([$1])
    m4_pushdef([name],SIZEOF_[]m4_toupper(m4_translit($1,[ *],[_p])))
    name=$($AWK '/name/{print $[]3}' confdefs.h)
    AC_SUBST(name)
    m4_popdef([name])
])

wrp_CHECK_SIZEOF_AND_SUBST([size_t])

#AC_MSG_ERROR(sizeof size_t: $sizeof_size_t / $SIZEOF_SIZE_T)
#AC_SUBST(SIZEOF_SIZE_T)

# Checks for library functions.
AC_FUNC_MALLOC

AC_ARG_ENABLE([gettid],
    AC_HELP_STRING([--enable-gettid],
        [Use syscall(SYS_gettid) to get the thread ID instead of pthread_self().
          This however costs a system call for each trace entry]))
if test "x$enable_gettid" = xyes; then
  use_gettid=yes
fi
if test "x$use_gettid" = xyes ; then
AC_SUBST(USE_GETTID, 1)
else
AC_SUBST(USE_GETTID, 0)
fi

# On many multicore CPUs, clock cycles are not synchronized. Therefore, we 
# prefer to use clock_gettime() instead of time stamp counter (clock cycles).
AC_MSG_CHECKING([for clock_gettime(CLOCK_REALTIME)])
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[
    #include <time.h>
    struct timespec tp;
    ]],
    [[ clock_gettime(CLOCK_REALTIME, &tp);
    ]])],
    have_clock_gettime=yes
  AC_DEFINE(CLOCK_GETTIME_AVAIL, [1], [Use clock_gettime])
  AC_MSG_RESULT([yes]),
  AC_MSG_RESULT([no])
)
if test "x$have_clock_gettime" = xyes ; then
  AC_SUBST(CLOCK_GETTIME_AVAIL, 1)
else
  AC_SUBST(CLOCK_GETTIME_AVAIL, 0)
fi

# Check the architecture type
case "$host" in
        x86_64-*)
        AC_MSG_WARN([ARCH: X86_64])
                arch_str="X86_64"
                ;;
        arm*-*)
        AC_MSG_WARN([ARCH: ARM])
                arch_str="ARM"
                ;;
        i686*-*)
        AC_MSG_WARN([ARCH: i686])
                arch_str="i686"
                ;;
        *)
                AC_MSG_WARN([unknown architecture: falling back to x86_64])
                ;;
esac

# Check whether 32-bit library will be built
AC_ARG_ENABLE([32bit], AC_HELP_STRING([--enable-32bit], [build 32-bit library]))
if test "x$enable_32bit" = xyes; then
  AC_SUBST(FORCE_32BIT, 1)
else
  AC_SUBST(FORCE_32BIT, 0)
fi

build_doc=no
AC_ARG_WITH(doc, [AS_HELP_STRING([--with-doc], [Generate documentation])],
                 [if test x$withval != xno ; then
                    build_doc=yes
                 fi],
                 [build_doc=no])

if test "x$build_doc" = "xyes" ; then
  AC_MSG_RESULT(Documentation will be generated)
  # Check for presence of pdfLaTeX
  AC_CHECK_PROG(PDFLATEX, pdflatex, pdflatex)
  if test -z "$PDFLATEX"; then
    AC_MSG_WARN([Unable to create PDF version of the user manual -- pdflatex is missing!])
  fi

  # Check for presence of latexmk
  AC_CHECK_PROG(LATEXMK, latexmk, latexmk)
  if test -z "$LATEXMK"; then
    AC_MSG_WARN([Unable to create PDF version of the user manual -- latemk is missing!])
  fi

  # Check for presence of doxygen
  AC_CHECK_PROGS([DOXYGEN], [doxygen])
  if test -z "$DOXYGEN";
     then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
  fi

else
  AC_MSG_RESULT(Documentation will not be generated)
fi

AM_CONDITIONAL([WITHDOC], [test "$with_doc" = "yes"])
AM_CONDITIONAL([HAVE_PDFLATEX], test -n "$PDFLATEX")
AM_CONDITIONAL([HAVE_LATEXMK], test -n "$LATEXMK")
AM_CONDITIONAL([HAVE_DOXYGEN],
	[test -n "$DOXYGEN"])AM_COND_IF([HAVE_DOXYGEN],
	[AC_CONFIG_FILES([doc/Doxyfile])])

AC_SUBST(libdir)
AC_SUBST(exec_prefix)
AC_SUBST(prefix)
AC_SUBST(includedir)


AC_OUTPUT([Makefile litl.pc \
           src/Makefile \
	   src/litl_config.h \
           utils/Makefile \
           tests/Makefile \
           doc/Makefile \
           doc/user_manual.tex \
           doc/header.tex ])
