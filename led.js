var five = require("johnny-five");
var board = new five.Board();

board.on("ready", function() {
  var led = new five.Led(11);

  // This will grant access to the led instance
  // from within the REPL that's created when
  // running this program.
  this.repl.inject({
    led: led
  });

  led.blink();
});

// OPTS
// led.blink();
// led.pulse();

// led.stop() // to stop blinking
// led.off()  // to shut it off (stop doesn't mean "off")
// led.on()   // to turn on, but not blink