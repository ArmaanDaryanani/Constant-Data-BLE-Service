import cb
import ui
import time
import sound
from sound import Player

class MyView(ui.View):
    def will_close(self):
        print("SLEEPING")
        cb.central_manager.cancel_peripheral_connect(delegate.peripheral)

class MyCentralManagerDelegate(object):
    def __init__(self):
        self.peripheral = None
        self.characteristic = None
        self.button = None

        # UI View
        self.view = MyView()
        self.view.name = 'BLE Toggle'
        self.view.background_color = '#dddddd'

        # toggle button
        self.button = ui.Button(title='LOADING...')
        self.button.name = "button"
        self.button.border_width = 1
        self.button.bg_color = '#32a852'
        self.button.font_color = 'black'
        self.button.enabled = False

        # center button
        self.button.center = (self.view.width * 0.5, self.view.height * 0.5)

        # flexable margins
        self.button.flex = 'LRTB'

        # when clicked call this function
        # self.button.action = self.button_click

        # add the button to the view
        self.view.add_subview(self.button)
        self.button = self.view['button']

        # show view
        self.view.present('sheet')

        # data from file
        self.received_data = bytearray()
        self.FILE_SIZE = 100000

    # device discovered
    def did_discover_peripheral(self, p):
        # print(p.name)
        if p.name and 'Arduino' in p.name and not self.peripheral:
            self.peripheral = p
            print(f'Discovered: {p}')
            print("Connecting...")
            cb.connect_peripheral(self.peripheral)
# ----------------------------------------------------------#
    # connected
    def did_connect_peripheral(self, p):
        print('Connected to peripheral ' + p.name)
        print('Discovering services...')
        p.discover_services()

    #not connected
    def did_fail_to_connect_peripheral(self, p, error):
        print('Failed to connect: %s' % (error,))

    #disconnected
    def did_disconnect_peripheral(selfself, p, error):
        print(f'Disconnected, error: {error}')
# ----------------------------------------------------------#
    # services discovered
    def did_discover_services(self, p, error):
        print("DISCOVERED SERVICES")
        print(p.services)
        for s in p.services:
            print(s)
            if s.uuid == '19B10010-E8F2-537E-4F6C-D104768A1214':
                print('Found service ' + s.uuid)
                print('Looking for Characteristics...')
                p.discover_characteristics(s)
            else:
                print("Could not find services matching your UUID")

    # characteristics discovered
    def did_discover_characteristics(self, s, error):
        for c in s.characteristics:
            if c.uuid == "19B10011-E8F2-537E-4F6C-D104768A1214":
                print('Found Characteristic ' + c.uuid)
                # whenever the characteristic's value changes
                self.peripheral.set_notify_value(c, True)
                self.reading_sensor_data(c)
                # once connected, alter UI elements
                #print(self.button)
                #self.button.enabled = True
                #self.button.title = "Testing"
                #print("end if list")

    def reading_sensor_data(self, c):
        while self.peripheral.state is 2:
            self.peripheral.read_characteristic_value(c)


    # Once the desired service and its characteristics are connected, initiate read from sensor
    def did_update_value(self, c, error):
        # termination_sequence = b'\xFF\xFF\xFF\xFF'
        print(c.value)
        # store as wav file, following
        '''if self.received_data.endswith(termination_sequence):
            with open('received.wav', 'wb') as f:
                f.write(self.received_data[:-len(termination_sequence)])
            self.ai_processing('received.wav')'''

    # plays audio file.
    '''def audio_playback(self, file_path):
      for x in range(5):
        print("running ai processing")
        sound.set_volume(1)
        player = Player(file_path)
        player.play()
        while player.playing:
        	pass
        time.sleep(1)'''


# initialize
delegate = MyCentralManagerDelegate()
cb.set_central_delegate(delegate)
print('Scanning for peripherals...')
cb.scan_for_peripherals()

try:
    while True: pass
except KeyboardInterrupt:
    cb.reset()
