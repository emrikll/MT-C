# scripts
Scripts for communicating with the MD407 via the terminal.

## Dependencies
* A bash-capable terminal emulator.
* GNU coreutils:
    * `stty`
    * `cat`
    * `echo`
    * `sleep`

## Usage
Two terminal windows are required: one for communicating with the MD407 and one
for sending the `.s19` binary file to the MD407.

To communicate with the MD407 (by means of mashing keys and pressing \<Enter>),
run `rtshow <conn>`, where \<conn> is the absolute or relative path to the file
denoting the connection to the MD407. Typically, \<conn> is `/dev/ttyUSB...`
(Linux) or `/dev/cu.usbserial...` (MacOS). If the connection is refused due to
file permissions, add the current user to the `dialout` group via `adduser
USERNAME dialout` or run the script with superuser privileges.

To send the `.s19` binary file to the MD407, run `rtsend <conn> <s19>`, where
\<conn> and \<s19> are absolute or relative paths to the MD407 and binary file
respectively. The binary file can be sent to multiple MD407s simultaneously by
supplying space-separated paths to \<conn>.

## Example
Assume \<conn> is `/dev/ttyUSB0`.

From terminal window 0 (`pwd` is `/path/to/rts-utils/scripts`):

```bash
./rtshow /dev/ttyUSB0
```

From terminal window 1 (`pwd` is `/path/to/RTS-Lab`):

```bash
/path/to/rts-utils/scripts/rtsend /dev/ttyUSB0 Debug/RTS-Lab.s19
```
