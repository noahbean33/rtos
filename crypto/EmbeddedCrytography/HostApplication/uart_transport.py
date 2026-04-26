import serial.tools.list_ports


class Uart:
    def __init__(self, manufacturer,baudrate):
        self.manufacturer = manufacturer
        self.baudrate     = baudrate
        self.device       = None

    def get_ports(self):
        ports =  list(serial.tools.list_ports.comports())
        for port in ports:
            if port.manufacturer.startswith(self.manufacturer):
                try:
                    port_number = port.device
                except Exception as e:
                    print("An error occurred :",{e})

                    
                print(port_number)
        return port_number
    
    def device_connect(self):
        self.device  = serial.Serial(port =  self.get_ports(), baudrate=self.baudrate, timeout=1)

    def receive_data(self):
        try:
            value =  self.device.readline()
        except:
            value = -999
        return value 


if __name__ == "__main__":
    try:

        iot_device =  Uart("STM",115200)
    except  Exception as e:
        print("An error occurred :",{e})
    iot_device.device_connect()

    while True:
        print(iot_device.receive_data())