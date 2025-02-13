mod mqtt;
mod poster;
mod subscriber;
use clap::Parser;
use poster::post;
use subscriber::subscribe;

#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
struct Args {
    #[arg(long)]
    post: bool,

    #[arg(long)]
    subscribe: bool,

    #[arg(long)]
    broker: bool,

    #[arg(long, default_value_t = 8, requires = "post")]
    payload: usize,

    #[arg(long, default_value = "127.0.0.1")]
    ip: String,

    #[arg(long, default_value_t = 100)]
    queue: usize,
}

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    let args = Args::parse();

    if args.broker {
        std::thread::spawn(mqtt::mqttd);
    }
    if args.post {
        tokio::spawn(post(args.queue, args.payload, args.ip.clone()));
    }
    if args.subscribe {
        tokio::spawn(subscribe(args.queue, args.ip));
    }

    tokio::signal::ctrl_c().await?;

    Ok(())
}
