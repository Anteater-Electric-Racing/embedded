use rand::Rng;

pub fn dummy() -> u32 {
    let speed: u32 = rand::thread_rng().gen_range(0..140);

    speed
}
