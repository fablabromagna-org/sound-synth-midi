from machine import Pin, Timer
from time import sleep_ms
import bluetooth
from ble_advertising import advertising_payload



class BLE():

    def __init__(self, name):
        
        self.name = name
        self.ble = bluetooth.BLE()
        self.ble.active(True)
        
        self.ble_client_connected = False
        
        self.ble.irq(self.ble_irq)
        sleep_ms(500)


    def connected(self):
        print("---> Connected")
        self.ble_client_connected = True
        pass

    def disconnected(self):
        print("-X-> Disconnected")
        self.ble_client_connected = False
 
    def ble_irq(self, event, data):
        print("-?-> IRQ", event, data)
        if event == 1:
            '''Central disconnected'''
            self.connected()
        
        elif event == 2:
            '''Central disconnected'''
            self.disconnected()
            
        elif event == 4:
            '''New message received'''
            
            print("MIDI IN RECEIVED")
#             buffer = self.ble.gatts_read(self.rx)
#             message = buffer.decode('UTF-8')[:-1]
#             print(message)

            
    def register(self):

        SERVICE_MIDI_UUID = "03b80e5a-ede8-4b33-a751-6ce34ec4c700"  # The MIDI Service
        CHARACTERISTIC_MIDI_UUID = "7772e5db-3868-4112-a1a9-f2669d106bf3"  ## The MIDI Characteristic
        
        BLE_MIDI_SERVICE = bluetooth.UUID(SERVICE_MIDI_UUID)
        BLE_MIDI_CHAR = (bluetooth.UUID(CHARACTERISTIC_MIDI_UUID), bluetooth.FLAG_WRITE_NO_RESPONSE | bluetooth.FLAG_READ | bluetooth.FLAG_NOTIFY)
        
        BLE_MIDI = (BLE_MIDI_SERVICE, (BLE_MIDI_CHAR, ))
        
        _ADV_TYPE_UUID128_MORE = const(0x6)
        self._adv_payload = advertising_payload(name="ESP32",
                                                services=[bluetooth.UUID("03b80e5a-ede8-4b33-a751-6ce34ec4c700")])
                                                #appearance=0x06)
        SERVICES = ( BLE_MIDI, )
        
        ((self.midi_tx,), ) = self.ble.gatts_register_services(SERVICES,)


    def send(self, data):
        self.ble.gatts_notify(0, self.midi_tx, data)


    def advertiser(self):
        name = bytes(self.name, 'UTF-8')
        
        self.ble.gap_advertise(100,  self._adv_payload)
        #self.ble.gap_advertise(100, bytearray('\x02\x01\x02') + bytearray((len(name) + 1, 0x09)) + name)



if __name__ == '__main__':

    ble = BLE("ESP32")
    print("--->BLE OK")

    ble.register()

    ble.advertiser()

    while (not ble.ble_client_connected) :
        ble.advertiser()

        print("...waiting connection")
        sleep_ms(5000)
        


    print("--->BLE SENDING")

    while (ble.ble_client_connected) :
        print("Sending MIDI command")
        
        array_note_on = [ 0x80, 0x80, 0x90, 60, 100 ]
        ble.send(bytes(array_note_on))
        sleep_ms(1000)

        
        array_note_off = [ 0x80, 0x80, 0x80, 60, 100 ]
        ble.send(bytes(array_note_off))
        sleep_ms(2000)

        print("I'm connected")    
        sleep_ms(100)



        