# DOTdraw Development Plan

## Fine Tuning

### Cursor Blink Behavior
- [x] Adjust cursor blink to be context-aware:
  - **Unlit pixel**: Long OFF, short ON (cursor mostly invisible, brief flash shows position)
  - **Lit pixel**: Long ON, short OFF (cursor mostly visible, brief dark shows position)
- Current implementation uses equal 100ms on/off timing regardless of pixel state
- Need to check `canvas[cursorY] & (1 << cursorX)` to determine if pixel is lit
- Suggested timing: ~300ms dominant state, ~100ms contrast state

### Direction Blip Behavior
- [x] Direction indicator pixel should be visible over lit pixels:
  - **Unlit pixel**: Turn ON briefly (current behavior)
  - **Lit pixel**: Turn OFF briefly (dim to show direction)
- Current implementation at `DOTdraw.ino:101-114` only adds (ORs) the blip pixel
- Need to check if target pixel is lit and XOR or mask it off instead

## Fixes

### Chord Triggering Single Button Action
- [x] Chording (both buttons) is also triggering a single button action
- Applied `chordedThisPress` pattern from DOTinvaders
- Flag set when chord detected, prevents single-button action on release, reset when all buttons released

## Future Ideas

### Button Interaction Library (DOTplatform)
- [ ] Create a common button management library in DOTplatform
- Reference implementation: DOTinvaders chord/click detection
- Should handle:
  - Single button press/release
  - Chord detection (multiple simultaneous buttons)
  - Hold detection with configurable duration
  - Debouncing
- Location: `DOTplatform/firmware/DotPlatform/src/`

### Pixel Display Utilities Library (DOTplatform)
- [ ] Create common pixel/display routines library in DOTplatform
- Potential routines:
  - Context-aware blinking (contrast against current state)
  - Direction indicators
  - Screen flash/clear animations
  - Pixel state queries
- Location: `DOTplatform/firmware/DotPlatform/src/`
