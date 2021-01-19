# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool dockerfile --cross mingw32 fedora-rawhide libnbd
#
# https://gitlab.com/libvirt/libvirt-ci/-/commit/740f5254f607de914a92d664196d045149edb45a
FROM registry.fedoraproject.org/fedora:rawhide

RUN dnf install -y nosync && \
    echo -e '#!/bin/sh\n\
if test -d /usr/lib64\n\
then\n\
    export LD_PRELOAD=/usr/lib64/nosync/nosync.so\n\
else\n\
    export LD_PRELOAD=/usr/lib/nosync/nosync.so\n\
fi\n\
exec "$@"' > /usr/bin/nosync && \
    chmod +x /usr/bin/nosync && \
    nosync dnf update -y --nogpgcheck fedora-gpg-keys && \
    nosync dnf update -y && \
    nosync dnf install -y \
        autoconf \
        automake \
        bash-completion \
        ca-certificates \
        ccache \
        coreutils \
        diffutils \
        git \
        glibc-langpack-en \
        golang \
        libtool \
        make \
        ocaml \
        ocaml-findlib \
        perl \
        python3-devel \
        qemu-img \
        sed && \
    nosync dnf autoremove -y && \
    nosync dnf clean all -y && \
    rpm -qa | sort > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/i686-w64-mingw32-cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/i686-w64-mingw32-$(basename /usr/bin/gcc)

RUN nosync dnf install -y \
        mingw32-gcc \
        mingw32-gnutls \
        mingw32-headers \
        mingw32-libxml2 \
        mingw32-pkg-config && \
    nosync dnf clean all -y

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"

ENV ABI "i686-w64-mingw32"
ENV CONFIGURE_OPTS "--host=i686-w64-mingw32"
