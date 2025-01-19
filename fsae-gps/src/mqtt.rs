/*
 * Subscribt to MQTT
 * 
 */
use iced::futures::{SinkExt, Stream};
use iced::stream;
use rumqttc::{AsyncClient, MqttOptions, EventLoop, MqttOptions, QoS};
use std::time::Duration;

// connection state
struct Connection {
    _client: AsyncClient,
    eventloop: EventLoop,
}

// background state
enum BackgroundState {
    Disconnected,
    Connected(Connection),
}

// message events
#[derive(Debug, Clone)]
pub enum Event {
    // receives message from MQTT
    Message(rumqttc::Event),
}

// set up a constant stream of events from MQTT
pub fn mqtt() -> impl Stream<Item = Event> {
    // set up a channel to send events to the app
    // setting a closure function
    // - takes a mutable input "output"
    // - returns a future (promise to send events)
    stream::channel(100, |mut output| async move{
        let mut state = BackgroundState::Disconnected;

        // constant loop to send events
        loop {
            // find the state to create based on current state
            match &mut state {
                // if disconnected, connect to MQTT broker
                BackgroundState::Disconnected => {
                    // set up mqtt connection
                    let mut mqttoptions = MqttOptions::new("gps-view", "127.0.0.1", 1883); // broker address
                    mqttoptions.set_keep_alive(Duration::from_secs(5));// time to wait before disconnecting
                    let (_client, eventloop) = AsyncClient::new(mqttoptions, 10); // create client and eventloop

                    // subscribe to gps topic
                    if let Err(e) = _client
                        .subscribe("fsae_raspi/can/gps", QoS::AtMostOnce) //! need to change the endpoint in the future
                        .await
                    {
                        eprintln!("Failed ot subsribe to GPS topic: {}", e);
                        continue;
                    };

                    state = BackgroundState::Connected(Connnection {_client, eventloop });
                }
                // if connected, wait for events
                BackgroundState::Connected(connection) => {
                    // handle incoming messages
                    while let Ok(notification) = connection.eventloop.poll().await {
                        if let Err(e) = output.send(Event::Message(notification)).await {
                            eprintln!("Failed to send message: {}", e);
                            break;
                        }
                    }
                    // if connection is lost, retry
                    eprintln!("Connection lost, retrying...");
                    tokio::time::sleep(Duration::from_secs(1)).await;
                    state = BackgroundState::Disconnected;
                }
            }
        }

    })
}
