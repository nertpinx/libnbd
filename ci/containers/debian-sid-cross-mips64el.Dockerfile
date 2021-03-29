# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool dockerfile --cross mips64el debian-sid libnbd
#
# https://gitlab.com/libvirt/libvirt-ci/-/commit/94c25bde639eb31ff2071fb6abfd3d5c777f4ab2

FROM docker.io/library/debian:sid-slim

RUN export DEBIAN_FRONTEND=noninteractive && \
    apt-get update && \
    apt-get install -y eatmydata && \
    eatmydata apt-get dist-upgrade -y && \
    eatmydata apt-get install --no-install-recommends -y \
            autoconf \
            automake \
            bash-completion \
            ca-certificates \
            ccache \
            clang \
            coreutils \
            diffutils \
            gcc \
            git \
            golang \
            libtool-bin \
            locales \
            make \
            ocaml \
            ocaml-findlib \
            perl \
            pkgconf \
            python3-dev \
            qemu-utils \
            sed && \
    eatmydata apt-get autoremove -y && \
    eatmydata apt-get autoclean -y && \
    sed -Ei 's,^# (en_US\.UTF-8 .*)$,\1,' /etc/locale.gen && \
    dpkg-reconfigure locales && \
    dpkg-query --showformat '${Package}_${Version}_${Architecture}\n' --show > /packages.txt && \
    mkdir -p /usr/libexec/ccache-wrappers && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/mips64el-linux-gnuabi64-cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/mips64el-linux-gnuabi64-$(basename /usr/bin/gcc)

RUN export DEBIAN_FRONTEND=noninteractive && \
    dpkg --add-architecture mips64el && \
    eatmydata apt-get update && \
    eatmydata apt-get dist-upgrade -y && \
    eatmydata apt-get install --no-install-recommends -y dpkg-dev && \
    eatmydata apt-get install --no-install-recommends -y \
            gcc-mips64el-linux-gnuabi64 \
            libc6-dev:mips64el \
            libfuse-dev:mips64el \
            libgnutls28-dev:mips64el \
            libxml2-dev:mips64el && \
    eatmydata apt-get autoremove -y && \
    eatmydata apt-get autoclean -y && \
    mkdir -p /usr/local/share/meson/cross && \
    echo "[binaries]\n\
c = '/usr/bin/mips64el-linux-gnuabi64-gcc'\n\
ar = '/usr/bin/mips64el-linux-gnuabi64-gcc-ar'\n\
strip = '/usr/bin/mips64el-linux-gnuabi64-strip'\n\
pkgconfig = '/usr/bin/mips64el-linux-gnuabi64-pkg-config'\n\
\n\
[host_machine]\n\
system = 'linux'\n\
cpu_family = 'mips64'\n\
cpu = 'mips64el'\n\
endian = 'little'" > /usr/local/share/meson/cross/mips64el-linux-gnuabi64

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"

ENV ABI "mips64el-linux-gnuabi64"
ENV CONFIGURE_OPTS "--host=mips64el-linux-gnuabi64"
