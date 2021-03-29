# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool dockerfile opensuse-152 libnbd
#
# https://gitlab.com/libvirt/libvirt-ci/-/commit/94c25bde639eb31ff2071fb6abfd3d5c777f4ab2

FROM registry.opensuse.org/opensuse/leap:15.2

RUN zypper update -y && \
    zypper install -y \
           autoconf \
           automake \
           bash-completion \
           ca-certificates \
           ccache \
           clang \
           coreutils \
           diffutils \
           fuse-devel \
           gcc \
           git \
           glibc-devel \
           glibc-locale \
           go \
           libgnutls-devel \
           libtool \
           libxml2-devel \
           make \
           ocaml \
           ocaml-findlib \
           perl \
           pkgconfig \
           python3-devel \
           qemu-tools \
           sed && \
    zypper clean --all && \
    rpm -qa | sort > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/$(basename /usr/bin/gcc)

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"