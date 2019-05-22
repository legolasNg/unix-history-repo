# $FreeBSD$

usage()
{
    cat <<__EOF__ >&2
usage: $(basename $0)

This script regenerates the DTrace test suite makefiles. It should be run
whenever \$srcdir/cddl/contrib/opensolaris/cmd/dtrace/test/tst is modified.
__EOF__
    exit 1
}

# Format a file list for use in a make(1) variable assignment: take the
# basename of each input file and append " \" to it.
fmtflist()
{
    awk 'function bn(f) {
        sub(".*/", "", f)
        return f
    }
    {print "    ", bn($1), " \\"}'
}

genmakefile()
{
    local class=$1
    local group=$2

    local tdir=${CONTRIB_TESTDIR}/${class}/${group}
    local tfiles=$(find $tdir -type f -a \
        \( -name \*.d -o -name \*.ksh -o -name \*.out \) | sort | fmtflist)
    local tcfiles=$(find $tdir -type f -a -name \*.c | sort | fmtflist)
    local texes=$(find $tdir -type f -a -name \*.exe | sort | fmtflist)

    # One-off variable definitions.
    local special
    case "$group" in
    proc)
        special="
LIBADD.tst.sigwait.exe+= rt
"
        ;;
    raise)
	special="
TEST_METADATA.t_dtrace_contrib+=	required_memory=\"4g\"
"
        ;;
    safety)
	special="
TEST_METADATA.t_dtrace_contrib+=	required_memory=\"4g\"
"
        ;;
    uctf)
        special="
WITH_CTF=YES
"
        ;;
    esac

    local makefile=$(mktemp)
    cat <<__EOF__ > $makefile
# \$FreeBSD$

#
# This Makefile was generated by \$srcdir${ORIGINDIR#${TOPDIR}}/genmakefiles.sh.
#

PACKAGE=	tests

\${PACKAGE}FILES= \\
$tfiles

TESTEXES= \\
$texes

CFILES= \\
$tcfiles

$special
.include "../../dtrace.test.mk"
__EOF__

    mv -f $makefile ${ORIGINDIR}/../${class}/${group}/Makefile
}

set -e

if [ $# -ne 0 ]; then
    usage
fi

export LC_ALL=C

readonly ORIGINDIR=$(realpath $(dirname $0))
readonly TOPDIR=$(realpath ${ORIGINDIR}/../../../../..)
readonly CONTRIB_TESTDIR=${TOPDIR}/cddl/contrib/opensolaris/cmd/dtrace/test/tst

for class in common i386; do
    for group in $(find ${CONTRIB_TESTDIR}/$class -mindepth 1 -maxdepth 1 -type d); do
        genmakefile $class $(basename $group)
    done
done
