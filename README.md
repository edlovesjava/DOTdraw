# DOTdraw

A simple drawing app for the [DOTplatform](https://github.com/edlovesjava/DOTplatform) handheld.

## Controls

| Input | Action |
|-------|--------|
| LEFT button | Rotate direction (cycles: up → right → down → left) |
| RIGHT button | Move cursor one step in current direction |
| CHORD (both) | Toggle draw mode on/off |
| CHORD hold (2s) | Clear screen |

## How It Works

- **Cursor**: Fast blinking LED shows your current position
- **Direction**: Brief blip shows which way you'll move next
- **Draw mode OFF**: Move freely without affecting pixels
- **Draw mode ON**: Moving toggles pixels (off→on, on→off)
- **Edges**: Wrap around to opposite side

## Hardware

Runs on the DOTplatform breadboard setup:
- ATtiny85 + MAX7219 8x8 LED matrix
- 2 buttons (LEFT=PB4, RIGHT=PB3)

See [DOTplatform](https://github.com/edlovesjava/DOTplatform) for hardware details.

## Building

1. Open `DOTdraw.ino` in Arduino IDE
2. Select Board: ATtiny85, Clock: 8MHz internal
3. Upload via USBtinyISP programmer

## Related Projects

- [DOTplatform](https://github.com/edlovesjava/DOTplatform) - Hardware platform
- [DOTinvaders](https://github.com/edlovesjava/dotinvaders) - Space Invaders game
- [DOTris](https://github.com/edlovesjava/DOTris) - Tetris game

## License

MIT License
