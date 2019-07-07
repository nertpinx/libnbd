/*
 * We have to have a build.rs script in order to use `links` in Cargo.toml.
 * Ideally this should figure out whether we can build with a library installed
 * in the system and if not, it should add C files to the build and build it
 * locally or basically build it without the library being installed in the
 * system.
 */

fn main() {}
