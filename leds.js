let five = require('johnny-five');
let board = new five.Board();

board.on('ready', () => {
  let leds = new five.Leds([9, 10, 11, 3, 5, 6]);

  // let timeout = 0;
  // leds.forEach((elem, index, array) => {
  //   setTimeout(() => {
  //     console.log(index);
  //     leds[index].blink();
  //     timeout += 100;
  //   }, timeout);
  // })

  leds[0].blink();
  setTimeout(() => leds[1].blink(), 100);
  setTimeout(() => leds[2].blink(), 200);
  setTimeout(() => leds[3].blink(), 300);
  setTimeout(() => leds[4].blink(), 400);
  setTimeout(() => leds[5].blink(), 500);

  // leds.pulse();

});