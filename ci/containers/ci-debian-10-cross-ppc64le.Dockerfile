# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool dockerfile --cross ppc64le debian-10 libnbd
#
# https://gitlab.com/libvirt/libvirt-ci/-/commit/740f5254f607de914a92d664196d045149edb45a
FROM docker.io/library/debian:10-slim

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
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/powerpc64le-linux-gnu-cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/powerpc64le-linux-gnu-$(basename /usr/bin/gcc)

RUN export DEBIAN_FRONTEND=noninteractive && \
    dpkg --add-architecture ppc64el && \
    eatmydata apt-get update && \
    eatmydata apt-get dist-upgrade -y && \
    eatmydata apt-get install --no-install-recommends -y dpkg-dev && \
    eatmydata apt-get install --no-install-recommends -y \
            gcc-powerpc64le-linux-gnu \
            libc6-dev:ppc64el \
            libfuse-dev:ppc64el \
            libgnutls28-dev:ppc64el \
            libxml2-dev:ppc64el && \
    eatmydata apt-get autoremove -y && \
    eatmydata apt-get autoclean -y && \
    mkdir -p /usr/local/share/meson/cross && \
    echo "[binaries]\n\
c = '/usr/bin/powerpc64le-linux-gnu-gcc'\n\
ar = '/usr/bin/powerpc64le-linux-gnu-gcc-ar'\n\
strip = '/usr/bin/powerpc64le-linux-gnu-strip'\n\
pkgconfig = '/usr/bin/powerpc64le-linux-gnu-pkg-config'\n\
\n\
[host_machine]\n\
system = 'linux'\n\
cpu_family = 'ppc64'\n\
cpu = 'powerpc64le'\n\
endian = 'little'" > /usr/local/share/meson/cross/powerpc64le-linux-gnu

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"

ENV ABI "powerpc64le-linux-gnu"
ENV CONFIGURE_OPTS "--host=powerpc64le-linux-gnu"
