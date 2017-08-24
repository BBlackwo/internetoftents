var five = require('johnny-five');
var board = new five.Board();

board.on("ready", function () {
  var piezo = new five.Piezo(8);

  board.repl.inject({
    piezo: piezo
  });

  piezo.play({
    song: [
      ["C4", 1 / 4],
      ["D4", 1 / 4],
    ],
    tempo: 100
  });
});