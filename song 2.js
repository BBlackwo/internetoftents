var five = require('johnny-five');
var board = new five.Board();

board.on("ready", function () {
  var piezo = new five.Piezo(8);

  board.repl.inject({
    piezo: piezo
  });
});

  piezo.play({
    song: "E - C - A - - E - C - A",
    beats: 1 / 2,
    tempo: 100
  });

// Plays a song
  piezo.play({
    // song is composed by an array of pairs of notes and beats
    // The first argument is the note (null means "no note")
    // The second argument is the length of time (beat) of the note (or non-note)
    song: [
      ["E2", 1 / 2],
      [null, 1 / 2],
      ["C4", 1 / 2],
      [null, 1 / 2],
      ["A6", 1 / 2],

      [null, 1],

      ["E2", 1 / 2],
      [null, 1 / 2],
      ["C4", 1 / 2],
      [null, 1 / 2],
      ["A6", 1 / 2],

      [null, 1],

      ["A6", 1 / 4],
      [null, 1/ 8]
      ["A6", 1 / 4],
      [null, 1/ 8]
      ["A6", 1 / 4],
      [null, 1/ 8]
      ["A6", 1 / 4],
      [null, 1/ 8]

      ["C4", 1 / 4],
      [null, 1/ 8]
      ["C4", 1 / 4],
      [null, 1/ 8]
      ["C4", 1 / 4],
      [null, 1/ 8]
      ["C4", 1 / 4],

      ["E2", 1 / 2],
      [null, 1 / 2],
      ["C4", 1 / 2],
      [null, 1 / 2],
      ["A6", 1 / 2],
    ],
    tempo: 100
  });