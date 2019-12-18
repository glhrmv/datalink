# datalink

> University project for FEUP, MIEIC (RCOM).

An asynchronous serial port (RS-232) data link protocol, accompanied by a simple application, showcasing the protocol's potential, that sends data complying with the protocol to transfer a file ([`pinguim.gif`](files/pinguim.gif)).

## Building

Run `make` in the root of the project. The `download` executable will be built inside a `bin` directory.

## Usage

The application expects the following usage:

```
./bin/datalink  <send|receive> <serial port device name>
```

where `send` is used for sending a file, and `receive` for receiving a file; `serial port device name` should be one of the serial ports in `/dev/`, e.g. `ttyS0`. Natually, two computers must have a direct serial connection for the program to work successfully.
