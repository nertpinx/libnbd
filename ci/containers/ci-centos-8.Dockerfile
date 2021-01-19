# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool dockerfile centos-8 libnbd
#
# https://gitlab.com/libvirt/libvirt-ci/-/commit/740f5254f607de914a92d664196d045149edb45a
FROM docker.io/library/centos:8

RUN dnf update -y && \
    dnf install 'dnf-command(config-manager)' -y && \
    dnf config-manager --set-enabled -y powertools && \
    dnf install -y centos-release-advanced-virtualization && \
    dnf install -y epel-release && \
    dnf install -y \
        autoconf \
        automake \
        bash-completion \
        ca-certificates \
        ccache \
        clang \
        coreutils-single \
        diffutils \
        fuse-devel \
        gcc \
        git \
        glibc-devel \
        glibc-langpack-en \
        gnutls-devel \
        golang \
        libtool \
        libxml2-devel \
        make \
        ocaml \
        ocaml-findlib \
        perl \
        pkgconfig \
        python3-devel \
        qemu-img \
        sed && \
    dnf autoremove -y && \
    dnf clean all -y && \
    rpm -qa | sort > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/$(basename /usr/bin/gcc)

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
