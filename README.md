Arduino-irrigation
==================

Arduino-irrigation is a GSM-based remote controller for irrigation systems.

Requirements
------------

This was built on an Arduino Uno with a SIM900-based GSM shield (here's an
[eBay link to the one I used][ebay]), and [the GSMlib library][gsmlib].


[gsmlib]: http://www.gsmlib.org/
[ebay]: http://www.ebay.co.uk/itm/230911052682?ssPageName=STRK:MEWNX:IT&_trksid=p3984.m1497.l2649

Installation
------------

To build and install it, I use [Ino](http://inotool.org/). You may be able to
do it from the Arduino IDE, but I haven't tried it.

To build and install using Ino, copy the GSMSHIELD directory to lib/, tweak the
pin configurations to suit your pins (the defaults should be good, though), and
run:

```
ino build
ino upload
```

That will build and upload the software to the Arduino, and in a minute or so
you will be able to call the Arduino and have it toggle the relay on or off
(as well as send you SMS when the inputs change).

Arduino-irrigation uses authorization, which means that you will have to store
up to two numbers on the SIM card (in positions 1 and 2). Those two numbers are
the only ones that will be able to control it. Whenever an input event happens,
Arduino-irrigation will send an SMS to the number in position 1, so that will
be your main number.

Usage
-----

The main functionality of Arduino-irrigation is that you can call it and have
it toggle your system (a relay) on and off. It supports push-button relay
systems (two relays, each is activated for 0.5 seconds, one turns the system on
and one turns it off), for a system that has two on/off buttons, and
single-relay systems that just keep one relay on while the system is on, and
turn it off to shut the system down.

It also supports an "enable" input that enables SMS sending (you turn it off if
you're changing things manually so you don't get notified all the time), an
"in operation" input that tells the software whether the system is on or off,
and three "error" relays that send SMS if they are triggered (for various
failure conditions).

For a better description of how it all works, the code is (hopefully) easy to
understand and concise, so I will refer you to that.
