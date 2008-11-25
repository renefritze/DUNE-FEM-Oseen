AC_DEFUN([DUNE_FEM_PARSE_VERSION],[
  DUNE_FEM_VERSION="$PACKAGE_VERSION"
<<<<<<< HEAD:dune-fem/m4/version.m4
  DUNE_FEM_VERSION_MAJOR="`echo $DUNE_FEM_VERSION | awk -F. -v FIELD=1 '{ print int($FIELD) }'`"
  DUNE_FEM_VERSION_MINOR="`echo $DUNE_FEM_VERSION | awk -F. -v FIELD=2 '{ print int($FIELD) }'`"
  DUNE_FEM_VERSION_REVISION="`echo $DUNE_FEM_VERSION | awk -F. -v FIELD=3 '{ print int($FIELD) }'`"
=======
  DUNE_FEM_VERSION_MAJOR="`echo $DUNE_FEM_VERSION | awk '{ split($[]0,s,"."); match(s[[1]],/[[[[:digit:]]]]*/,n); print n[[0]] }'`"
  DUNE_FEM_VERSION_MINOR="`echo $DUNE_FEM_VERSION | awk '{ split($[]0,s,"."); match(s[[2]],/[[[[:digit:]]]]*/,n); print n[[0]] }'`"
  DUNE_FEM_VERSION_REVISION="`echo $DUNE_FEM_VERSION | awk '{ split($[]0,s,"."); match(s[[3]],/[[[[:digit:]]]]*/,n); print n[[0]] }'`"
>>>>>>> reverts all post 1.1.1 commits except adding a new build target for suse10.3:dune-fem/m4/version.m4

  if test x$DUNE_FEM_VERSION_MAJOR = x -o x$DUNE_FEM_VERSION_MINOR = x -o DUNE_FEM_VERSION_REVISION = x ; then
    AC_MSG_ERROR([Cannot parse dune-fem version: $DUNE_FEM_VERSION])
  fi
  AC_MSG_NOTICE([Parsed dune-fem version: $DUNE_FEM_VERSION_MAJOR.$DUNE_FEM_VERSION_MINOR.$DUNE_FEM_VERSION_REVISION])
  AC_SUBST(DUNE_FEM_VERSION_MAJOR,$DUNE_FEM_VERSION_MAJOR)
  AC_SUBST(DUNE_FEM_VERSION_MINOR,$DUNE_FEM_VERSION_MINOR)
  AC_SUBST(DUNE_FEM_VERSION_REVISION,$DUNE_FEM_VERSION_REVISION)

<<<<<<< HEAD:dune-fem/m4/version.m4
  DUNE_GRID_VERSION_MAJOR="`echo $DUNE_GRID_VERSION | awk -F. -v FIELD=1 '{ print int($FIELD) }'`"
  DUNE_GRID_VERSION_MINOR="`echo $DUNE_GRID_VERSION | awk -F. -v FIELD=2 '{ print int($FIELD) }'`"
=======
  DUNE_GRID_VERSION_MAJOR="`echo $DUNE_GRID_VERSION | awk '{ split($[]0,s,"."); match(s[[1]],/[[[[:digit:]]]]*/,n); print n[[0]] }'`"
  DUNE_GRID_VERSION_MINOR="`echo $DUNE_GRID_VERSION | awk '{ split($[]0,s,"."); match(s[[2]],/[[[[:digit:]]]]*/,n); print n[[0]] }'`"
>>>>>>> reverts all post 1.1.1 commits except adding a new build target for suse10.3:dune-fem/m4/version.m4
  if test x$DUNE_GRID_VERSION_MAJOR = x -o DUNE_GRID_VERSION_MINOR = x ; then
    AC_MSG_ERROR([Cannot parse dune-grid version: $DUNE_GRID_VERSION])
  fi
  AC_MSG_NOTICE([Parsed dune-grid version: $DUNE_GRID_VERSION_MAJOR.$DUNE_GRID_VERSION_MINOR])
<<<<<<< HEAD:dune-fem/m4/version.m4
  if test \( $DUNE_GRID_VERSION_MAJOR -lt 1 \) -o \( \( $DUNE_GRID_VERSION_MAJOR -eq 1 \) -a \( $DUNE_GRID_VERSION_MINOR -lt 1 \) \) ; then
    AC_MSG_ERROR([Your 'dune-grid' version too old; at least version 1.1 is required.])
  fi
=======
>>>>>>> reverts all post 1.1.1 commits except adding a new build target for suse10.3:dune-fem/m4/version.m4
  AC_SUBST(DUNE_GRID_VERSION_MAJOR,$DUNE_GRID_VERSION_MAJOR)
  AC_SUBST(DUNE_GRID_VERSION_MINOR,$DUNE_GRID_VERSION_MINOR)
])
