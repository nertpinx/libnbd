use libnbd_rs::*;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let nbd = Nbd::create()?;
    println!("libnbd version is {}", nbd.get_version());
    Ok(())
}
