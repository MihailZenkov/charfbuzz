#!/bin/sh

# stolen from ffmpeg configure like a pig

# Prevent locale nonsense from breaking basic text processing.
LC_ALL=C
export LC_ALL

major=0
minor=9
micro=20
slib_file_name=libharfbuzz.so.${major}.${minor}.${micro}
slib_soname=libharfbuzz.so.${major}

api_hdrs='
hb.h
hb-blob.h
hb-buffer.h
hb-common.h
hb-deprecated.h
hb-face.h
hb-font.h
hb-set.h
hb-shape.h
hb-shape-plan.h
hb-unicode.h
hb-ft.h
hb-graphite2.h
hb-coretext.h
hb-uniscribe.h
hb-icu.h
hb-glib.h
hb-gobject.h
hb-gobject-structs.h
hb-ot.h
hb-ot-layout.h
hb-ot-tag.h
hb-version.h
'

srcs='
hb-blob.c
hb-buffer.c
hb-common.c
hb-face.c
hb-fallback-shape.c
hb-font.c
hb-ft.c
hb-glib.c
hb-open-file.c
hb-ot-layout.c
hb-ot-tag.c
hb-shape.c
hb-shape-plan.c
hb-shaper.c
hb-unicode.c
hb-utf-private.c
'

clean_do(){
    rm -f ${slib_file_name}
    for src_file in ${srcs}
    do
      rm -f ${src_file/\.c/.o}
    done
    rm -f harfbuzz.pc hb-version.h
    exit 0
}

install_do(){ 
    eval prefix=${prefix}
    eval libdir=${libdir}
    eval incdir=${incdir}
    
    ${slib_install} ${slib_file_name} ${destdir}${sysroot}${libdir}/${slib_file_name}
    ${ln_s} ${destdir}${sysroot}${libdir}/${slib_file_name} ${destdir}${sysroot}${libdir}/${slib_soname}
    ${ln_s} ${destdir}${sysroot}${libdir}/${slib_file_name} ${destdir}${sysroot}${libdir}/libharfbuzz.so
    ${file_install} harfbuzz.pc  ${destdir}${sysroot}${libdir}/pkgconfig/harfbuzz.pc
    for hdr in ${api_hdrs}
    do
        ${file_install} ${source_path}/${hdr} ${destdir}${sysroot}${incdir}/harfbuzz/${hdr}
    done
    exit 0
}

################################################################################

# find source path
if test -f make; then
    source_path=.
else
    source_path=$(cd $(dirname "$0"); pwd)
    echo "$source_path" | grep -q '[[:blank:]]' &&
        die "Out of tree builds are impossible with whitespace in source path."
    test -e "$source_path/config.h" &&
        die "Out of tree builds are impossible with config.h in source dir."
fi

is_in(){
    value=$1
    shift
    for var in $*; do
        [ $var = $value ] && return 0
    done
    return 1
}

append(){
    var=$1
    shift
    eval "$var=\"\$$var $*\""
}

die_unknown(){
    echo "Unknown option \"$1\"."
    echo "See $0 --help for available options."
    exit 1
}

set_default(){
    for opt; do
        eval : \${$opt:=\$${opt}_default}
    done
}

PATHS_LIST='
    prefix
    incdir
    libdir
    prefix
    sysroot
    destdir
'

CMDLINE_APPEND="
    extra_cflags
    extra_cxxflags
    host_cppflags
"

CMDLINE_SET="
    ${PATHS_LIST}
    slib_cc
    slib_ld
"

# path defaults
prefix_default="/usr/local"
incdir_default='${prefix}/include'
libdir_default='${prefix}/lib'

#command lin set defaults
#slib_cc_default='gcc -Wall -Wextra -Wno-missing-field-initializers -c -fPIC -fpic -Ofast -std=c99'
slib_cc_default="gcc -Wall -Wextra -Wno-missing-field-initializers -c -fPIC -fpic -g -std=c99"
slib_ld_default="gcc -shared -Wl,-soname=${LIB_SONAME}"
slib_install_default='install -D --mode=755'
file_install_default='install -D --mode=644'
ln_s_default='ln --symbolic --force'

set_default ${PATHS_LIST}
set_default slib_cc slib_ld
set_default slib_install file_install ln_s

show_help(){
    cat <<EOF
Usage: make [options] [operations]

Operations: [default is to build the shared library]:
  clean                    clean build products
  install                  install build products


Options: [defaults in brackets after descriptions]

Help options:
  --help                   print this message


Standard options:
  --prefix=PREFIX          install in PREFIX [$prefix_default]
  --libdir=DIR             install libs in DIR [PREFIX/lib]
  --incdir=DIR             install includes in DIR [PREFIX/include]

Advanced options (experts only):
  --sysroot=PATH           root of cross-build tree
  --destdir=PATH           installation root dir override
  --slib-cc=CC             use C compiler command line CC for shared lib object[$slib_cc_default]
  --slib-ld=LD             use linker command line LD for shared lib [$slib_ld_default]
EOF
  exit 0
}

for opt do
    optval="${opt#*=}"
    case "$opt" in
        clean) clean_do
        ;;
        install) install_do
        ;;
        --help|-h) show_help
        ;;
        *)
            optname="${opt%%=*}"
            optname="${optname#--}"
            optname=$(echo "$optname" | sed 's/-/_/g')
            if is_in $optname $CMDLINE_SET; then
                eval $optname='$optval'
            elif is_in $optname $CMDLINE_APPEND; then
                append $optname "$optval"
            else
                die_unknown $opt
            fi
        ;;
    esac
done

#for pkgconfig to pickup the right package files
export PKG_CONFIG_SYSROOT_DIR=${sysroot}

#CPPFLAGS=-I./ -I${source_path} -DHAVE_OT -DHAVE_GLIB
CPPFLAGS="-I./ -I${source_path} -DHAVE_GLIB \
$(pkg-config --cflags freetype2) \
$(pkg-config --cflags glib-2.0)
"

#generate the version header file
sed -e "
s/@HB_VERSION_MAJOR@/${major})/
s/@HB_VERSION_MINOR@/${minor}/
s/@HB_VERSION_MICRO@/${micro}/
s/@HB_VERSION@/${major}.${minor}.${micro}/
" ${source_path}/hb-version.h.in >hb-version.h

#generate the pkgconfig file
sed -e "
s:%prefix%:${prefix}:
s:%exec_prefix%:${prefix}:
s:%libdir%:${libdir}:
s:%includedir%:${incdir}:
s:%VERSION%:${major}.${minor}.${micro}:
" ${source_path}/harfbuzz.pc.in >harfbuzz.pc

#compile source files, build a list of objet files
for src_file in ${srcs}
do
    obj=${src_file/\.c/.o}
    echo "SLIB_CC ${src_file}"
    ${slib_cc} ${CPPFLAGS} -o ${obj} ${source_path}/${src_file}
    objs="${obj} ${objs}"
done

#link the shared lib
echo "SLIB_LD ${slib_file_name}"
echo ${objs}
${slib_ld} -o ${slib_file_name} ${objs}
