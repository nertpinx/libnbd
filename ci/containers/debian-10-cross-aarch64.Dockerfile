# THIS FILE WAS AUTO-GENERATED
#
#  $ lcitool dockerfile --cross aarch64 debian-10 libnbd
#
# https://gitlab.com/libvirt/libvirt-ci/-/commit/94c25bde639eb31ff2071fb6abfd3d5c777f4ab2

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
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/aarch64-linux-gnu-cc && \
    ln -s /usr/bin/ccache /usr/libexec/ccache-wrappers/aarch64-linux-gnu-$(basename /usr/bin/gcc)

RUN export DEBIAN_FRONTEND=noninteractive && \
    dpkg --add-architecture arm64 && \
    eatmydata apt-get update && \
    eatmydata apt-get dist-upgrade -y && \
    eatmydata apt-get install --no-install-recommends -y dpkg-dev && \
    eatmydata apt-get install --no-install-recommends -y \
            gcc-aarch64-linux-gnu \
            libc6-dev:arm64 \
            libfuse-dev:arm64 \
            libgnutls28-dev:arm64 \
            libxml2-dev:arm64 && \
    eatmydata apt-get autoremove -y && \
    eatmydata apt-get autoclean -y && \
    mkdir -p /usr/local/share/meson/cross && \
    echo "[binaries]\n\
c = '/usr/bin/aarch64-linux-gnu-gcc'\n\
ar = '/usr/bin/aarch64-linux-gnu-gcc-ar'\n\
strip = '/usr/bin/aarch64-linux-gnu-strip'\n\
pkgconfig = '/usr/bin/aarch64-linux-gnu-pkg-config'\n\
\n\
[host_machine]\n\
system = 'linux'\n\
cpu_family = 'aarch64'\n\
cpu = 'aarch64'\n\
endian = 'little'" > /usr/local/share/meson/cross/aarch64-linux-gnu

ENV LANG "en_US.UTF-8"
ENV MAKE "/usr/bin/make"
ENV CCACHE_WRAPPERSDIR "/usr/libexec/ccache-wrappers"

ENV ABI "aarch64-linux-gnu"
ENV CONFIGURE_OPTS "--host=aarch64-linux-gnu"
