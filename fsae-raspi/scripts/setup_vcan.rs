use std::process::Command;

fn main() {
    run("sudo", &["modprobe", "vcan"]);
    run("sudo", &["ip", "link", "add", "dev", "vcan0", "type", "vcan"]);
    run("sudo", &["ip", "link", "set", "vcan0", "up"]);
    println!("vcan0 running");
}

fn run(cmd: &str, args: &[&str]) {
    println!("> Running {} {}", cmd, args.join(" "));
    let status = Command::new(cmd)
        .args(args)
        .status()
        .expect("Failed to execute command");
    if !status.success() {
        eprintln!("Error: {} {}", cmd, args.join(" "));
    }
}
