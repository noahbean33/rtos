import serial.tools.list_ports


def get_ports():
    ports =  list(serial.tools.list_ports.comports())
    for port in ports:
        print(port.manufacturer)

        if port.manufacturer.startswith("STM"):
            port_number = port.device
    return port_number


iot_device = serial.Serial(port =  get_ports(), baudrate=115200, timeout=1)
def receive_data():
    try:
        value =  iot_device.readline()
        print(value)
    except:
        print("Nothing to print")

while True:
    receive_data()

