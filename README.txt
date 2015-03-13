resp - An experimental protocol for embedded devices

RESP is the REdis Serialization Protocol. 
It is simple to implement, fast to parse, and human readable.

I wanted to see what it would look like on small embedded CPUs like
8051, ATmega, Cortex M, and so on. This implementation is incomplete
and ugly. But it definitely shows the benefit of using a human readable
protocol instead of something like Firmata and other binary protocols.

The two things I wanted to try are controlling a GPIO and using the
EEPROM as a key-value store. Note that this version only uses RAM
for the key-value store.


GETTING IT RUNNING

1. Load the sketch on an Arduino. Any model should work.
2. Open the Serial Monitor.
3. Baud setting is 57600.
4. Select any line ending setting EXCEPT "No line ending".


COMMAND SET

DOUT pin value - Sets the value of a digital output pin.
SET key value - Store the value in key.
DEL key - Delete the key and its value.
GET key - Lookup the value of key.


WALKTHROUGH

Most Arduino boards have a LED on pin 13. Let's turn that on.
Enter this command in the Serial Monitor:

    dout 13 1

The LED should turn on and you'll get a success message from the board:

    +OK

Let's try the key-value store next. Here's some commands and the expected
responses if you do everything in order:

    get foo
    -ERROR not found
    set foo bar
    +OK
    get foo
    +bar
    set vendor AE9RB
    +OK
    get vendor
    +AE9RB
    del foo
    +OK
    get foo
    -ERROR not found

That's all for now. If you have ever used a Redis database this all looks
very familiar. With a bit more work, all the client libraries for Redis
will be able to control your embedded processor.
