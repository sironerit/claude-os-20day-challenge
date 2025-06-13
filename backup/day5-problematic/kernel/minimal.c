// Minimal test kernel for debugging
// This is the simplest possible kernel to verify the build system

void kernel_main(void) {
    // VGA text buffer
    char* video_memory = (char*)0xB8000;
    
    // Write "HI" in white on black
    video_memory[0] = 'H';
    video_memory[1] = 0x07; // White on black
    video_memory[2] = 'I';
    video_memory[3] = 0x07;
    
    // Infinite loop
    while (1) {
        asm volatile ("hlt");
    }
}