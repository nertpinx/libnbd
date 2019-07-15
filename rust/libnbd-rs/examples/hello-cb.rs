use libnbd_rs::*;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let mut nbd = Nbd::create()?;
    nbd.set_debug(true)?;
    nbd.set_debug_callback(Some(Box::new(|a, b| eprintln!("ASDF: {}: {}", a, b))))?;
    println!("{}", nbd.get_version());
    //    nbd.set_debug_callback(None)?;
    Ok(())
}
