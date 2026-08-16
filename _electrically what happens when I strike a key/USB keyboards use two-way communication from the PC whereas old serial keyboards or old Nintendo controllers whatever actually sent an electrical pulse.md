# USB keyboards use two-way communication from the PC whereas old serial keyboards or old Nintendo controllers whatever actually sent an electrical pulse

The title captures the intended contrast, but there are two useful corrections:

- USB keyboard traffic is host-controlled: the PC initiates transactions and polls the keyboard for input.
- PS/2 and old Nintendo controllers are both simpler electrically than USB, but they do not work the same way. A PS/2 keyboard normally transmits scan-code data using clock/data lines; a classic NES controller is itself polled by the console using latch and clock signals and shifts button bits back serially.

## Ben Eater: How does a USB keyboard work?

https://www.youtube.com/watch?v=wdgULBpRoXk

Ben Eater puts an oscilloscope on a USB keyboard's D+ and D- data lines and works upward from the electrical waveform rather than starting with a software abstraction. He identifies the idle electrical state, explains differential signalling, then works through how USB bits and packets are encoded and what the packets contain.

The especially important observation for this keyboard project is that **pressing a key does not immediately cause the keyboard to throw a new pulse onto the USB cable**. USB transactions are initiated by the host. The computer repeatedly asks the keyboard endpoint for data; the keyboard responds with its report. That is very different from imagining a switch wire continuing all the way to the computer.

The video then decodes enough of the packet structure to show how the apparently messy oscilloscope trace becomes structured digital communication, and compares this host-polled USB arrangement with the older PS/2 keyboard interface.

For our keyboard, the useful mental model is therefore:

```text
switch closes
    |
keyboard microcontroller notices it
    |
firmware changes its input/report state
    |
PC initiates USB transaction
    |
keyboard replies over D+ / D-
```

The physical key matrix and the USB bus are two different electrical systems joined by the keyboard microcontroller.
