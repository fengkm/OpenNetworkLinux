from onl.platform.base import *
from onl.platform.ufispace import *
from struct import *
from ctypes import c_int, sizeof
import os
import sys
import subprocess
import commands
import time
import fcntl

def msg(s, fatal=False):
    sys.stderr.write(s)
    sys.stderr.flush()
    if fatal:
        sys.exit(1)

class IPMI_Ioctl(object):
    _IONONE = 0
    _IOWRITE = 1
    _IOREAD = 2

    IPMI_MAINTENANCE_MODE_AUTO = 0
    IPMI_MAINTENANCE_MODE_OFF  = 1
    IPMI_MAINTENANCE_MODE_ON   = 2

    IPMICTL_GET_MAINTENANCE_MODE_CMD = _IOREAD << 30 | sizeof(c_int) << 16 | \
        ord('i') << 8 | 30  # from ipmi.h
    IPMICTL_SET_MAINTENANCE_MODE_CMD = _IOWRITE << 30 | sizeof(c_int) << 16 | \
        ord('i') << 8 | 31  # from ipmi.h

    def __init__(self):
        self.ipmidev = None
        devnodes=["/dev/ipmi0", "/dev/ipmi/0", "/dev/ipmidev/0"]
        for dev in devnodes:
            try:
                self.ipmidev = open(dev, 'rw')
                break
            except Exception as e:
                print("open file {} failed, error: {}".format(dev, e))

    def __del__(self):
        if self.ipmidev is not None:
            self.ipmidev.close()

    def get_ipmi_maintenance_mode(self):
        input_buffer=pack('i',0)
        out_buffer=fcntl.ioctl(self.ipmidev, self.IPMICTL_GET_MAINTENANCE_MODE_CMD, input_buffer)
        maintanence_mode=unpack('i',out_buffer)[0]

        return maintanence_mode

    def set_ipmi_maintenance_mode(self, mode):
        fcntl.ioctl(self.ipmidev, self.IPMICTL_SET_MAINTENANCE_MODE_CMD, c_int(mode))

class OnlPlatform_x86_64_ufispace_s9501_28smt_r0(OnlPlatformUfiSpace):
    PLATFORM='x86-64-ufispace-s9501-28smt-r0'
    MODEL="S9501-28SMT"
    SYS_OBJECT_ID=".9501.28"
    PORT_COUNT=28
    PORT_CONFIG="20x1 + 8x10"

    def check_bmc_enable(self):
        return 1

    def init_i2c_mux_idle_state(self, muxs):
        IDLE_STATE_DISCONNECT = -2

        for mux in muxs:
            i2c_addr = mux[1]
            i2c_bus = mux[2]
            sysfs_idle_state = "/sys/bus/i2c/devices/%d-%s/idle_state" % (i2c_bus, hex(i2c_addr)[2:].zfill(4))
            if os.path.exists(sysfs_idle_state):
                with open(sysfs_idle_state, 'w') as f:
                    f.write(str(IDLE_STATE_DISCONNECT))

    def baseconfig(self):

        bmc_enable = self.check_bmc_enable()
        msg("bmc enable : %r\n" % (True if bmc_enable else False))

        # record the result for onlp
        os.system("echo %d > /etc/onl/bmc_en" % bmc_enable)

        ########### initialize I2C bus 0 ###########
        # i2c_i801 is built-in

        # add i2c_ismt
        #self.insmod("i2c-ismt") #module not found
        os.system("modprobe i2c-ismt")

        #CPLD
        self.insmod("x86-64-ufispace-s9501-28smt-lpc")

        # init PCA9548
        bus_i801=0
        bus_ismt=1
        i2c_muxs = [
            ('pca9546', 0x75, bus_ismt),   #9546_ROOT_TIMING
            ('pca9546', 0x76, bus_ismt),   #9546_ROOT_SFP
            ('pca9548', 0x71, bus_ismt+8), #9548_CHILD_SFP_4_11
            ('pca9548', 0x72, bus_ismt+8), #9548_CHILD_SFP_12_19
            ('pca9548', 0x73, bus_ismt+8), #9548_CHILD_SFP_20_27
        ]
        self.new_i2c_devices(i2c_muxs)
        # init idle state on mux
        self.init_i2c_mux_idle_state(i2c_muxs)

        self.insmod("x86-64-ufispace-eeprom-mb")
        self.insmod("optoe")

        # init SYS EEPROM devices
        self.new_i2c_devices(
            [
                #  on cpu board
                ('mb_eeprom', 0x57, bus_ismt),
            ]
        )

        # init SFP/SFP+ EEPROM
        self.init_eeprom(bus_ismt)

        # init Temperature
        #self.insmod("jc42") #module not found
        os.system("modprobe jc42")

        # init GPIO sysfs
        self.init_gpio()

        #enable ipmi maintenance mode
        self.enable_ipmi_maintenance_mode()

        return True

    def enable_ipmi_maintenance_mode(self):
        ipmi_ioctl = IPMI_Ioctl()

        mode=ipmi_ioctl.get_ipmi_maintenance_mode()
        msg("Current IPMI_MAINTENANCE_MODE=%d\n" % (mode) )

        ipmi_ioctl.set_ipmi_maintenance_mode(IPMI_Ioctl.IPMI_MAINTENANCE_MODE_ON)

        mode=ipmi_ioctl.get_ipmi_maintenance_mode()
        msg("After IPMI_IOCTL IPMI_MAINTENANCE_MODE=%d\n" % (mode) )

    def init_gpio(self):

        # init GPIO sysfs
        self.new_i2c_devices(
            [
                ('pca9535', 0x20, 4), #9535_BOARD_ID
                ('pca9535', 0x22, 6), #9535_TX_DIS_0_15
                ('pca9535', 0x24, 6), #9535_TX_DIS_16_31
                ('pca9535', 0x26, 7), #9535_TX_FLT_0_15
                ('pca9535', 0x27, 7), #9535_TX_FLT_16_31
                ('pca9535', 0x25, 7), #9535_RATE_SELECT_0_15
                ('pca9535', 0x23, 7), #9535_RATE_SELECT_16_31
                ('pca9535', 0x20, 8), #9535_MOD_ABS_0_15
                ('pca9535', 0x22, 8), #9535_MOD_ABS_16_31
                ('pca9535', 0x21, 8), #9535_RX_LOS_0_15
                ('pca9535', 0x24, 8)  #9535_RX_LOS_0_15
            ]
        )

        # init all GPIO direction to "in"
        gpio_dir = ["in"] * 512

        # get board id
        cmd = "cat /sys/devices/platform/x86_64_ufispace_s9501_28smt_lpc/mb_cpld/board_id_0"
        status, output = commands.getstatusoutput("cat /sys/devices/platform/x86_64_ufispace_s9501_28smt_lpc/mb_cpld/board_id_0")
        if status != 0:
            msg("Get board id from LPC failed, status=%s, output=%s, cmd=%s\n" % (status, output, cmd))
            return

        board_id = int(output, 10)
        model_id = board_id & 0b00001111
        hw_rev = (board_id & 0b00110000) >> 4
        build_rev = (board_id & 0b11000000) >> 6
        hw_build_rev = (hw_rev << 2) | (build_rev)

        #Alpha 1
        if hw_build_rev == 4:
            # init GPIO direction to output low
            for i in range(495, 487, -1) + \
                     range(483, 475, -1) + \
                     range(471, 463, -1):
                gpio_dir[i] = "low"

            # init GPIO direction to output high
            for i in range(431, 423, -1) + \
                     range(419, 411, -1) + \
                     range(407, 399, -1):
                gpio_dir[i] = "high"

            msg("Alpha 1 GPIO init\n")
        #Alpha 2 and later
        else:
            # init GPIO direction to output low
            for i in range(495, 487, -1) + \
                     range(483, 475, -1) + \
                     range(471, 463, -1) + \
                     range(431, 423, -1) + \
                     range(419, 415, -1) + \
                     range(407, 403, -1):
                gpio_dir[i] = "low"

            # init GPIO direction to output high
            for i in range(415, 411, -1) + \
                     range(403, 399, -1):
                gpio_dir[i] = "high"
            msg("Alpha 2 and later GPIO init\n")

        # export GPIO and configure direction
        for i in range(336, 512):
            os.system("echo {} > /sys/class/gpio/export".format(i))
            os.system("echo {} > /sys/class/gpio/gpio{}/direction".format(gpio_dir[i], i))

    def init_eeprom(self, bus_ismt):
        port = 4

        # init SFP EEPROM
        for bus in range(bus_ismt + 9, bus_ismt + 33):
            self.new_i2c_device('optoe2', 0x50, bus)
            # update port_name
            subprocess.call("echo {} > /sys/bus/i2c/devices/{}-0050/port_name".format(port, bus), shell=True)
            port = port + 1
