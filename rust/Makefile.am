# nbd client library in userspace
# Copyright (C) 2013-2019 Red Hat Inc.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

include $(top_srcdir)/subdir-rules.mk

clean-local:
	-cargo clean

generator_built = \
  libnbd-sys/Cargo.toml \
  libnbd-sys/src/lib.rs \
  libnbd-rs/Cargo.toml \
  libnbd-rs/src/glue.rs

if HAVE_RUST

all: cargo.stamp

cargo.stamp: $(generator_built)
	$(top_builddir)/run cargo build --release
	$(top_builddir)/run cargo build --release --example hello-nbd
	$(top_builddir)/run cargo doc

TESTS = run-tests

endif HAVE_RUST
