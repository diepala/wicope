import time
import serial
import numpy as np


class Device:

    COM_CODES = {
        "START": b"\x10",
        "TIMEBASE": {
            "20 us": b"\x21",
            "50 us": b"\x22",
            "100 us": b"\x23",
            "200 us": b"\x24",
            "500 us": b"\x25",
            "1 ms": b"\x26",
            "2 ms": b"\x27",
            "5 ms": b"\x28",
            "10 ms": b"\x29",
            "20 ms": b"\x2a",
        },
        "TRIGGER_ENABLE": b"\x31",
        "TRIGGER_DISABLE": b"\x32",
        "TRIGGER_EDGE": {
            "Rising": b"\x33",
            "Falling": b"\x34",
            "Any": b"\x35",
        },
    }
    BUFFER_SIZE = 100  # 512
    BAUDRATE = 115200

    def __init__(self):
        self.serial_port = serial.Serial()
        self.serial_port.baudrate = self.BAUDRATE

        self.timebase = "20 ms"
        self.trigger_on = False
        self.trigger_slope = "Rising"

    def connect(self, port):
        print(port)
        self.serial_port.port = port
        self.serial_port.open()
        time.sleep(2)  # wait until arduino is available
        self.write_all_settings()

    def disconnect(self):
        self.serial_port.close()

    def write_all_settings(self):
        self.write_timebase()
        self.write_trigger_state()
        self.write_trigger_slope()

    def write_timebase(self):
        self.serial_port.write(self.COM_CODES["TIMEBASE"][self.timebase])

    def write_trigger_state(self):
        if self.trigger_on:
            self.serial_port.write(self.COM_CODES["TRIGGER_ENABLE"])
        else:
            self.serial_port.write(self.COM_CODES["TRIGGER_DISABLE"])

    def write_trigger_slope(self):
        self.serial_port.write(self.COM_CODES["TRIGGER_EDGE"][self.trigger_slope])

    def clean_buffers(self):
        self.serial_port.reset_input_buffer()
        self.serial_port.reset_output_buffer()

    def acquire_single(self):
        self.serial_port.write(self.COM_CODES["START"])
        data = self.serial_port.read(size=self.BUFFER_SIZE)
        data = np.frombuffer(data, dtype=np.uint8).astype(float) * 5 / 256
        return data

    def is_connected(self):
        return self.serial_port.is_open
