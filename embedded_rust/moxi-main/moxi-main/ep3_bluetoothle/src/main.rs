#![no_std]
#![no_main]

mod ble;
mod display;
mod sense;

use defmt::info;
use defmt_rtt as _;
use display::display_task;
use embassy_executor::Spawner;
use microbit_bsp::Microbit;
use panic_probe as _;
use sense::sense_task;

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    info!("Starting...");
    let b = Microbit::default();
    spawner.must_spawn(sense_task(b.twispi0, b.p20, b.p19));
    spawner.must_spawn(display_task(b.display));
    let (sdc, mpsl) = b.ble.init(b.timer0, b.rng).unwrap();
    ble::run(sdc, mpsl, spawner).await
}
