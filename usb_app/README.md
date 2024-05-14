# MIMXRT1060_EVKB USB Zephyr & USB Host + fatfs functionality

Porting MCUXpresso's example `evkbmimxrt1060_host_msd_fatfs_bm` to Zephyr. 

## Building
Ensure paths in `zephyr-linux.code-workspace` are correct, do <kbd>ctrl</kbd>+<kbd>shift</kbd>+<kbd>b</kbd> and select `West Configurable Build`.  
The code has been built with `west v1.2.0`, Zephyr commit 74ddfbc8a2e4f1932bf2b6a160f83a27a6c135a5, and `zephyr-sdk-0.16.5-1`. 

## Flashing
JLink Pro has been used for flashing the binary.

## How to use
Power the board via barrel jack J45. Build, flash, connect to serial port on J1, open a terminal with setting 115200 8N1. Then connect USB drive to port J48 and observe console output. Note that test fails with some USB drives, but not all the time.
```
*** Booting Zephyr OS build v3.6.0-3374-g74ddfbc8a2e4 ***
host init done
USB Thread started
enumeration failed
mass storage device attached:pid=0x917vid=0x1f75 address=1
............................fatfs test.....................
fatfs mount as logiacal driver 1......success
test f_mkfs......error
............................test done......................
mass storage device detached
mass storage device attached:pid=0x1234vid=0xabcd address=1
............................fatfs test.....................
fatfs mount as logiacal driver 1......success
test f_mkfs......success
test f_getfree:
    FAT type = FAT16
    bytes per cluster = 32768; number of clusters=60827 
    The free size: 1946464KB, the total size:1946464KB
directory operation:
list root directory:

create directory "dir_1"......success
create directory "dir_2"......success
create sub directory "dir_2/sub_1"......success
list root directory:
    dir - ___ - DIR_1 - 0Bytes - 2018-1-1 0:0:0
    dir - ___ - DIR_2 - 0Bytes - 2018-1-1 0:0:0
list directory "dir_1":
    dir - ___ - SUB_1 - 0Bytes - 2018-1-1 0:0:0
rename directory "dir_1/sub_1" to "dir_1/sub_2"......success
delete directory "dir_1/sub_2"......success
get directory "dir_1" information:
    dir - ___ - DIR_1 - 0Bytes - 2018-1-1 0:0:0
change "dir_1" timestamp to 2015.10.1, 12:30:0......success
get directory "dir_1" information:
    dir - ___ - DIR_1 - 0Bytes - 2015-10-1 12:30:0
file operation:
create file "f_1.dat"......success
test f_write......success
test f_printf......success
test f_puts......success
test f_putc......success
test f_seek......success
test f_gets......ABCDEFGHI
test f_read......JKLMNOPQRS
test f_truncate......success
test f_close......success
get file "f_1.dat" information:
    fil - ___ - F_1.DAT - 19Bytes - 2018-1-1 0:0:0
change "f_1.dat" timestamp to 2015.10.1, 12:30:0......success
change "f_1.dat" to readonly......success
get file "f_1.dat" information:
    fil - R__ - F_1.DAT - 19Bytes - 2015-10-1 12:30:0
remove "f_1.dat" readonly attribute......success
get file "f_1.dat" information:
    fil - ___ - F_1.DAT - 19Bytes - 2015-10-1 12:30:0
rename "f_1.dat" to "f_2.dat"......success
delete "f_2.dat"......success
............................test done......................


```

## Issues
1. Code does not work when using SDRAM. For possible solution, see [here](https://community.nxp.com/t5/MCUXpresso-SDK/IMXRT1060-USB-Host-MSD-Example-Fails-to-Enumerate-When-Using/m-p/1520093).