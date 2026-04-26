import serial.tools.list_ports


def get_ports():
    ports =  list(serial.tools.list_ports.comports())
    for port in ports:
        print(port.manufacturer)

        if port.manufacturer.startswith("STM"):
            port_number = port.device
    return port_number



print(get_ports())