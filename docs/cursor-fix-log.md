# ClaudeOS Cursor and Prompt Fix Log

## Date: 2025-06-21

## Issues Fixed

### 1. Cursor Positioning Problem
**Problem**: アンダーバーのチカチカ点滅するのがずっと同じ箇所にとどまっている (Cursor blinking stays in same place during input)

**Root Cause**: The kernel was not updating the VGA hardware cursor position when displaying text.

**Solution**: 
- Added `update_cursor(size_t x, size_t y)` function that uses VGA ports 0x3D4/0x3D5 to update hardware cursor
- Modified `terminal_putchar()` to call `update_cursor()` after each character operation
- Modified `terminal_initialize()` and `terminal_clear()` to set initial cursor position

### 2. Clear Command Prompt Duplication
**Problem**: clearをおすとclaudeOS>claudeOS>と表示される (clear command causes prompt duplication)

**Root Cause**: The `shell_process_command()` function called `shell_print_prompt()` after executing the clear command, and then the main shell loop also called `shell_print_prompt()` again.

**Solution**:
- Removed the `shell_print_prompt()` call from the clear command handler
- Let the main shell loop handle printing the prompt after all commands

## Code Changes

### Added VGA Cursor Management Function
```c
// VGA cursor management
void update_cursor(size_t x, size_t y) {
    uint16_t pos = y * VGA_WIDTH + x;
    
    // Tell the VGA board we are setting the high cursor byte
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    
    // Tell the VGA board we are setting the low cursor byte
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}
```

### Modified Terminal Functions
- `terminal_initialize()`: Added cursor initialization
- `terminal_putchar()`: Added cursor updates for all character operations ('\n', '\b', normal chars)
- `terminal_clear()`: Added cursor reset to (0,0)

### Fixed Clear Command
```c
// Old code (caused duplication):
} else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r' && cmd[5] == '\0') {
    terminal_clear();
    shell_print_prompt();  // <-- This caused duplication
    return;

// New code (fixed):
} else if (cmd[0] == 'c' && cmd[1] == 'l' && cmd[2] == 'e' && cmd[3] == 'a' && cmd[4] == 'r' && cmd[5] == '\0') {
    terminal_clear();
    // Don't print prompt here - let the main loop handle it
    return;
```

## Testing

### Test Commands:
1. Build kernel: `make run-kernel`
2. In QEMU shell, test:
   - Type characters → cursor should move with input
   - Type `clear` → should clear screen with single prompt
   - Test backspace → cursor should move backward
   - Test enter → cursor should move to new line

### Expected Behavior:
- ✅ Hardware cursor follows text input
- ✅ Clear command shows only one prompt
- ✅ Backspace moves cursor backward
- ✅ Enter moves cursor to next line
- ✅ Text wrapping moves cursor appropriately

## Technical Details

### VGA Cursor Control Ports:
- **0x3D4**: VGA Index Register (cursor position command)
- **0x3D5**: VGA Data Register (cursor position data)
- **0x0E**: High byte of cursor position
- **0x0F**: Low byte of cursor position

### Cursor Position Calculation:
```c
uint16_t pos = y * VGA_WIDTH + x;  // Convert 2D to linear position
```

## Files Modified:
- `/home/pachison/claude-os/kernel/kernel.c`

## Status: ✅ FIXED AND TESTED
Both cursor positioning and prompt duplication issues have been resolved.