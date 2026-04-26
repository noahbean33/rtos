use defmt::info;
use embassy_sync::{blocking_mutex::raw::ThreadModeRawMutex, watch::{DynReceiver, Watch}};
use microbit_bsp::embassy_nrf::{bind_interrupts, Peri, peripherals::{P0_26, P1_00, TWISPI0}, twim::{self, Twim}};
use embassy_time::{Delay, Timer};
use libscd::asynchronous::scd4x::Scd4x;
use static_cell::ConstStaticCell;

const CO2_CONSUMERS: usize = 1;
static CO2: Watch<ThreadModeRawMutex, u16, CO2_CONSUMERS> = Watch::new();

pub fn get_receiver() -> Option<DynReceiver<'static, u16>> {
    CO2.dyn_receiver()
}

#[embassy_executor::task]
pub async fn sense_task(twi: Peri<'static, TWISPI0>, sda: Peri<'static, P1_00>, scl: Peri<'static, P0_26>) {
    bind_interrupts!(struct Irqs {
        TWISPI0 => twim::InterruptHandler<TWISPI0>;
    });
    static RAM_BUFFER: ConstStaticCell<[u8; 4]> = ConstStaticCell::new([0; 4]);
    let i2c = Twim::new(twi, Irqs, sda, scl, Default::default(), RAM_BUFFER.take());

    let mut scd = Scd4x::new(i2c, Delay);

    Timer::after_millis(30).await;

    // When re-programming, the controller will be restarted,
    // but not the sensor. We try to stop it in order to
    // prevent the rest of the commands failing.
    _ = scd.stop_periodic_measurement().await;

    info!("Sensor serial number: {:?}", scd.serial_number().await);
    if let Err(e) = scd.start_periodic_measurement().await {
        defmt::panic!("Failed to start periodic measurement: {:?}", e);
    }

    let tx = CO2.sender();
    loop {
        if scd.data_ready().await.unwrap() {
            let m = scd.read_measurement().await.unwrap();
            info!(
                "CO2: {}, Humidity: {}, Temperature: {}",
                m.co2 as u16, m.humidity as u16, m.temperature as u16
            );
            tx.send(m.co2 as u16);
        }

        Timer::after_millis(1000).await;
    }
}