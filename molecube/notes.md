# Notes for use of ZC702 Zynq development board from Xilinx

### Boot configuration via light blue DIP switch (SW16)
To boot via JTAG, all DIP switches are down
To boot via flash, DIP switch 4 is up and the rest are down
To boot via SD card, DIP switches 3 and 4 are up and the rest are down
Up is with respect to numbers on sswitch

### Digilent JTAG cable on Linux:
Set DIP switch (SW10) to positions 1=down, 2=up

See https://bbs.archlinux.org/viewtopic.php?id=132394
Edit `/etc/udev/52-digilent-usb.rules` to contain:

    ATTRS{idVendor}=="1443", MODE="666"
    ACTION=="add", ATTRS{idVendor}=="0403", ATTRS{manufacturer}=="Digilent", MODE="666", RUN+="/usr/sbin/dftdrvdtch %s{busnum} %s{devnum}"

Run:

    sudo udevadm control --reload-rules

Check that USB/Digilent programming cable works:

    djtgcfg enum

### Error when programming FPGA from SDK:

    Failed to connect to Xilinx hw_server. Check if the hw_server is running and correct TCP port is used.

Solution:
Select "Configure JTAG Setting", "Digilent USB cable" in SDK
See: http://www.xilinx.com/support/answers/55431.htm

### Linux terminal coneection to FPGA:

    screen -h 1000 /dev/ttyUSB0 115200,cs8,-ixon,-ixon,istrip
    screen -h 1000 /dev/ttyUSB2 115200,cs8,-ixon,-ixon,istrip

### Program Zynq flash via impact (this seems very flaky, and it may work better to copy BOOT.bin onto the SD card)

Launch xilinx shell, navigate to folder that contains boot image BOOT.mcs

    impact -batch program_flash.cmd

or run `impact -batch`
then enter these commands:

    setMode -bs
    setCable -port auto
    setCableSpeed -speed 2500000
    Identify -inferir
    readTemperatureAndVoltage -p 1

    erase -image BOOT.mcs -qspi -p 1
    program -image BOOT.mcs -qspi -v -p 1
    checksum
    closeCable

### If we want background network transfer, we probably need to use both ARM cores.
This would require:

1. Adding pulse_controller HW to .bit file for Linux boot
2. Re-write device driver to run memory-mapped in user-space
3. Add network thread for background transfer
4. Make sure main thread has highest priority.

Or we could let the pulse controller execute SPI commands and
gain enough free time to execute a network transfer before the processor is needed.

## 1/26/2013

* Try to get Zynq & Ionizer to work together.
  Previously, the programs would connect, but scans didn't seem to work.

* `SPI_init` hangs. Disable by setting `NSPI=0`. Need to fix later.
* ionizerFPGA boots now. `IP=1.2.3.4`, `eth2` is `1.2.3.100`

* Add `#define ZYNQ_FPGA` to Ionizer PC app for FPGA IP address
* Add `C2S_NUM_MOTORS` message, to get rid of `HAS_MOTORS` macro,
  which reduces portability.
* Now the PC / FPGA startup gets to the voltages page, and hangs there.
  Presumably because SPI is missing.
* So try to put SPI back. Hangs at Zynq boot when trying to communicate
  from processor to SPI.
* Move SPI from AXI interconnect 1 to 0. Pulse controller and SPI are now
  on the same bus.
* SPI still hangs. Try to use Zynq native SPI (built into processor,
  not AXI peripheral).

* Disable SPI for now.
* Code runs.  PMT values in scan don't make sense.

* Suddenly linker settings got messed up in Eclipse.
  Added environment variables to end of linker cmd line

        LIBS2 = -Wl,--start-group,-lxil,-lgcc,-lc,-lstdc++,--end-group
        LIBS3 = -Wl,--start-group,-lxil,-llwip4,-lgcc,-lc,--end-group

* Linker is fixed.

## 1/27/2013
* PMT values appear to be incorrect when running a scan.
  Expect 0 PMT counts, since nothing is connected.
* Observe the PMT register incrementing by 1 every shot.
  However, the PMT average is reported as "inf".

## 5/8/2013
* Updated to Xilinx 14.5 software.

## 5/9/2013
* Merge Zynq code with operating Virtex 4 code.
  A few platform specific files remain in `platform/zynq`
* All other files are symbolic links to `control_trunk/FPGA/SDK/ionizerFPGA`.
* UDP transfer to PC is not working. Can see some transfer via netcat,
  but transmit buffer overflows after 400 to 800 records.
* Nothing shows up on PC ionizer.

## 5/10/2013
* Rebuild HW (`.bit` file) from PlanAhead. Export to SDK does not update the
  `.bit` file that gets sent to FPGA from SDK.
* To update, go to `Al/control_trunk/FPGA/Zynq/ionizer` and run:

        cp ./Ionizer_Zynq.runs/impl_1/zc702_stub.bit ./Ionizer_Zynq.sdk/SDK/SDK_Export/zc702_hw_platform/system.bit

What about other files in `zc702_hw_platform` ?

Remaining issues:
* UDP transfer
* TCP transfer of long debug messages
* SPI, GPIO on Zynq
* ZC702 breakout board
* See some pulses on scope, verify timing [seems ok]
* Check PMT readout
