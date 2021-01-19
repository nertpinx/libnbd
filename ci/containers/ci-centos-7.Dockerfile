# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool dockerfile centos-7 libnbd
#
# https://gitlab.com/libvirt/libvirt-ci/-/commit/740f5254f607de914a92d664196d045149edb45a
FROM docker.io/library/centos:7

RUN yum update -y && \
    echo 'skip_missing_names_on_install=0' >> /etc/yum.conf && \
    yum install -y epel-release && \
    yum install -y \
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
        glibc-common \
        glibc-devel \
        gnutls-devel \
        golang \
        libtool \
        libxml2-devel \
        make \
        ocaml \
        ocaml-findlib \
        perl \
        pkgconfig \
        python36-devel \
        qemu-img \
        sed && \
    yum autoremove -y && \
    yum clean all -y && \
    rpm -qa | sort > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/$(basename /usr/bin/gcc)

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"
