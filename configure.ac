AC_INIT([genem], [0.1], [pdc09@pdc.me.uk])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_PROG_CC_C99
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_C_BIGENDIAN

#CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
#CC="$PTHREAD_CC"])


# function checks

AC_CHECK_HEADERS([sys/stat.h], [], [])
AC_CHECK_HEADERS([pthread.h],
	[PTHREAD_CFLAGS="-pthread"],
	[AC_MSG_ERROR(Cannot find pthread.h)])

CFLAGS="-O0 -ggdb -Wall -Werror"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"

#AC_DEFINE(VARIABLE, VALUE, DESCRIPTION)

AC_OUTPUT
