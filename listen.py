import sys
import serial
import struct

if __name__ == '__main__':
    ser = serial.Serial(sys.argv[1], 1000000)
    hdr = "<HBBBBHH"
    hdr_len = struct.calcsize(hdr)
    while 1:
        while ser.read(1) != bytes([0xfe]):
            pass
        (length, msgtype, type, device, endpoint, idVendor, idProdcut) = struct.unpack(hdr, ser.read(hdr_len))
        msgbuffer = ser.read(length)
        print(length, msgtype, type, device, endpoint, idVendor, idProdcut, list(msgbuffer))
