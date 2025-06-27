// ClaudeOS Network Foundation - Day 19 Basic Infrastructure Implementation
// Safe network abstraction without complex protocols

#include "network.h"
#include "kernel.h"
#include "timer.h"
#include "string.h"

// Global network state
network_interface_t network_interfaces[MAX_NETWORK_INTERFACES];
network_packet_t packet_buffers[PACKET_BUFFER_COUNT];
int next_interface_id = 0;
bool network_initialized = false;

// Simple string utilities for network
static size_t net_strlen(const char* str) {
    size_t len = 0;
    while (str[len] && len < 64) len++;
    return len;
}

static void net_strcpy(char* dest, const char* src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

static int net_strcmp(const char* str1, const char* str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

// Network system initialization
void network_init(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK));
    terminal_writestring("[NETWORK] Initializing Day 19 network foundation...\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    // Clear network interfaces
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        network_interfaces[i].id = -1;
        network_interfaces[i].enabled = false;
        network_interfaces[i].state = NET_STATE_DOWN;
        network_interfaces[i].packets_sent = 0;
        network_interfaces[i].packets_received = 0;
        network_interfaces[i].bytes_sent = 0;
        network_interfaces[i].bytes_received = 0;
        network_interfaces[i].errors = 0;
        for (int j = 0; j < 16; j++) {
            network_interfaces[i].name[j] = 0;
        }
        for (int j = 0; j < 6; j++) {
            network_interfaces[i].mac_address[j] = 0;
        }
    }
    
    // Clear packet buffers
    for (int i = 0; i < PACKET_BUFFER_COUNT; i++) {
        packet_buffers[i].in_use = false;
        packet_buffers[i].size = 0;
        packet_buffers[i].interface_id = -1;
        packet_buffers[i].timestamp = 0;
        for (int j = 0; j < MAX_PACKET_SIZE; j++) {
            packet_buffers[i].data[j] = 0;
        }
    }
    
    next_interface_id = 0;
    
    // Create loopback interface (always available)
    int lo_id = network_create_interface("lo", NET_INTERFACE_LOOPBACK);
    if (lo_id >= 0) {
        network_interfaces[lo_id].ip_address = 0x7F000001; // 127.0.0.1
        network_interfaces[lo_id].mac_address[0] = 0x00;
        network_interfaces[lo_id].mac_address[1] = 0x00;
        network_interfaces[lo_id].mac_address[2] = 0x00;
        network_interfaces[lo_id].mac_address[3] = 0x00;
        network_interfaces[lo_id].mac_address[4] = 0x00;
        network_interfaces[lo_id].mac_address[5] = 0x01;
        network_enable_interface(lo_id);
    }
    
    // Create virtual ethernet interface
    int eth_id = network_create_interface("eth0", NET_INTERFACE_ETHERNET);
    if (eth_id >= 0) {
        network_interfaces[eth_id].ip_address = 0xC0A80101; // 192.168.1.1
        network_interfaces[eth_id].mac_address[0] = 0x52;
        network_interfaces[eth_id].mac_address[1] = 0x54;
        network_interfaces[eth_id].mac_address[2] = 0x00;
        network_interfaces[eth_id].mac_address[3] = 0x12;
        network_interfaces[eth_id].mac_address[4] = 0x34;
        network_interfaces[eth_id].mac_address[5] = 0x56;
        network_enable_interface(eth_id);
    }
    
    network_initialized = true;
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    terminal_writestring("[NETWORK] Network foundation initialized!\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("  - Loopback interface: lo (127.0.0.1)\n");
    terminal_writestring("  - Ethernet interface: eth0 (192.168.1.1)\n");
    terminal_writestring("  - Packet buffers: 32 available\n");
}

// Interface management
int network_create_interface(const char* name, net_interface_type_t type) {
    if (!name || next_interface_id >= MAX_NETWORK_INTERFACES) {
        return -1;
    }
    
    // Find free interface slot
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        if (network_interfaces[i].id == -1) {
            network_interfaces[i].id = next_interface_id++;
            net_strcpy(network_interfaces[i].name, name, 16);
            network_interfaces[i].type = type;
            network_interfaces[i].state = NET_STATE_DOWN;
            network_interfaces[i].enabled = false;
            return network_interfaces[i].id;
        }
    }
    
    return -1;
}

int network_enable_interface(int interface_id) {
    network_interface_t* iface = network_find_interface(interface_id);
    if (!iface) return -1;
    
    iface->enabled = true;
    iface->state = NET_STATE_UP;
    return 0;
}

int network_disable_interface(int interface_id) {
    network_interface_t* iface = network_find_interface(interface_id);
    if (!iface) return -1;
    
    iface->enabled = false;
    iface->state = NET_STATE_DOWN;
    return 0;
}

network_interface_t* network_find_interface(int interface_id) {
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        if (network_interfaces[i].id == interface_id) {
            return &network_interfaces[i];
        }
    }
    return NULL;
}

network_interface_t* network_find_interface_by_name(const char* name) {
    if (!name) return NULL;
    
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        if (network_interfaces[i].id != -1 && 
            net_strcmp(network_interfaces[i].name, name) == 0) {
            return &network_interfaces[i];
        }
    }
    return NULL;
}

// Packet buffer management
network_packet_t* network_alloc_packet(void) {
    for (int i = 0; i < PACKET_BUFFER_COUNT; i++) {
        if (!packet_buffers[i].in_use) {
            packet_buffers[i].in_use = true;
            packet_buffers[i].size = 0;
            packet_buffers[i].timestamp = get_uptime_seconds();
            packet_buffers[i].interface_id = -1;
            return &packet_buffers[i];
        }
    }
    return NULL; // No free buffers
}

void network_free_packet(network_packet_t* packet) {
    if (packet) {
        packet->in_use = false;
        packet->size = 0;
        packet->interface_id = -1;
    }
}

int network_send_packet(int interface_id, const uint8_t* data, size_t size) {
    if (!data || size == 0 || size > MAX_PACKET_SIZE) return -1;
    
    network_interface_t* iface = network_find_interface(interface_id);
    if (!iface || !iface->enabled) return -1;
    
    // Simulate packet sending (just update statistics)
    iface->packets_sent++;
    iface->bytes_sent += size;
    
    return 0; // Success
}

network_packet_t* network_receive_packet(int interface_id) {
    network_interface_t* iface = network_find_interface(interface_id);
    if (!iface || !iface->enabled) return NULL;
    
    // Simulate packet reception (no actual packets in basic implementation)
    return NULL;
}

// Network statistics
void network_get_stats(network_stats_t* stats) {
    if (!stats) return;
    
    stats->total_packets_sent = 0;
    stats->total_packets_received = 0;
    stats->total_bytes_sent = 0;
    stats->total_bytes_received = 0;
    stats->total_errors = 0;
    stats->active_interfaces = 0;
    
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        if (network_interfaces[i].id != -1 && network_interfaces[i].enabled) {
            stats->total_packets_sent += network_interfaces[i].packets_sent;
            stats->total_packets_received += network_interfaces[i].packets_received;
            stats->total_bytes_sent += network_interfaces[i].bytes_sent;
            stats->total_bytes_received += network_interfaces[i].bytes_received;
            stats->total_errors += network_interfaces[i].errors;
            stats->active_interfaces++;
        }
    }
    
    // Count used packet buffers
    stats->buffer_usage = 0;
    for (int i = 0; i < PACKET_BUFFER_COUNT; i++) {
        if (packet_buffers[i].in_use) {
            stats->buffer_usage++;
        }
    }
}

// Utility functions
const char* network_interface_type_string(net_interface_type_t type) {
    switch (type) {
        case NET_INTERFACE_LOOPBACK: return "Loopback";
        case NET_INTERFACE_ETHERNET: return "Ethernet";
        case NET_INTERFACE_VIRTUAL: return "Virtual";
        default: return "Unknown";
    }
}

const char* network_interface_state_string(net_interface_state_t state) {
    switch (state) {
        case NET_STATE_DOWN: return "DOWN";
        case NET_STATE_UP: return "UP";
        case NET_STATE_TESTING: return "TESTING";
        default: return "UNKNOWN";
    }
}

void network_format_mac_address(const uint8_t* mac, char* buffer, size_t size) {
    if (!mac || !buffer || size < 18) return;
    
    // Simple MAC address formatting: XX:XX:XX:XX:XX:XX
    char hex[] = "0123456789ABCDEF";
    int pos = 0;
    
    for (int i = 0; i < 6; i++) {
        if (i > 0 && pos < (int)size - 1) {
            buffer[pos++] = ':';
        }
        if (pos < (int)size - 2) {
            buffer[pos++] = hex[(mac[i] >> 4) & 0xF];
            buffer[pos++] = hex[mac[i] & 0xF];
        }
    }
    buffer[pos] = '\0';
}

void network_format_ip_address(uint32_t ip, char* buffer, size_t size) {
    if (!buffer || size < 16) return;
    
    // Simple IP address formatting: XXX.XXX.XXX.XXX
    uint8_t a = (ip >> 24) & 0xFF;
    uint8_t b = (ip >> 16) & 0xFF;
    uint8_t c = (ip >> 8) & 0xFF;
    uint8_t d = ip & 0xFF;
    
    char temp[16];
    int pos = 0;
    
    // Convert each octet
    itoa(a, temp, 10);
    for (int i = 0; temp[i] && pos < (int)size - 1; i++) buffer[pos++] = temp[i];
    if (pos < (int)size - 1) buffer[pos++] = '.';
    
    itoa(b, temp, 10);
    for (int i = 0; temp[i] && pos < (int)size - 1; i++) buffer[pos++] = temp[i];
    if (pos < (int)size - 1) buffer[pos++] = '.';
    
    itoa(c, temp, 10);
    for (int i = 0; temp[i] && pos < (int)size - 1; i++) buffer[pos++] = temp[i];
    if (pos < (int)size - 1) buffer[pos++] = '.';
    
    itoa(d, temp, 10);
    for (int i = 0; temp[i] && pos < (int)size - 1; i++) buffer[pos++] = temp[i];
    
    buffer[pos] = '\0';
}

// Network command implementations
void network_show_interfaces(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Network Interfaces:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    terminal_writestring("Name   Type      State  IP Address     MAC Address\n");
    terminal_writestring("-----  --------  -----  -------------  -----------------\n");
    
    for (int i = 0; i < MAX_NETWORK_INTERFACES; i++) {
        if (network_interfaces[i].id != -1) {
            network_interface_t* iface = &network_interfaces[i];
            
            // Interface name
            terminal_writestring(iface->name);
            
            // Pad to align columns
            int name_len = net_strlen(iface->name);
            for (int j = name_len; j < 7; j++) terminal_writestring(" ");
            
            // Interface type
            const char* type_str = network_interface_type_string(iface->type);
            terminal_writestring(type_str);
            int type_len = net_strlen(type_str);
            for (int j = type_len; j < 10; j++) terminal_writestring(" ");
            
            // State
            const char* state_str = network_interface_state_string(iface->state);
            terminal_writestring(state_str);
            int state_len = net_strlen(state_str);
            for (int j = state_len; j < 7; j++) terminal_writestring(" ");
            
            // IP Address
            char ip_str[16];
            network_format_ip_address(iface->ip_address, ip_str, 16);
            terminal_writestring(ip_str);
            int ip_len = net_strlen(ip_str);
            for (int j = ip_len; j < 15; j++) terminal_writestring(" ");
            
            // MAC Address
            char mac_str[18];
            network_format_mac_address(iface->mac_address, mac_str, 18);
            terminal_writestring(mac_str);
            
            terminal_writestring("\n");
        }
    }
}

void network_show_stats(void) {
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("Network Statistics:\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    network_stats_t stats;
    network_get_stats(&stats);
    
    terminal_writestring("  Active Interfaces: ");
    char num_str[16];
    itoa((int)stats.active_interfaces, num_str, 10);
    terminal_writestring(num_str);
    terminal_writestring("\n");
    
    terminal_writestring("  Packets Sent: ");
    itoa((int)stats.total_packets_sent, num_str, 10);
    terminal_writestring(num_str);
    terminal_writestring("\n");
    
    terminal_writestring("  Packets Received: ");
    itoa((int)stats.total_packets_received, num_str, 10);
    terminal_writestring(num_str);
    terminal_writestring("\n");
    
    terminal_writestring("  Bytes Sent: ");
    itoa((int)stats.total_bytes_sent, num_str, 10);
    terminal_writestring(num_str);
    terminal_writestring("\n");
    
    terminal_writestring("  Bytes Received: ");
    itoa((int)stats.total_bytes_received, num_str, 10);
    terminal_writestring(num_str);
    terminal_writestring("\n");
    
    terminal_writestring("  Buffer Usage: ");
    itoa((int)stats.buffer_usage, num_str, 10);
    terminal_writestring(num_str);
    terminal_writestring("/");
    itoa(PACKET_BUFFER_COUNT, num_str, 10);
    terminal_writestring(num_str);
    terminal_writestring(" buffers\n");
}

void network_ping_simulation(const char* target) {
    if (!target) {
        terminal_writestring("Usage: ping <target>\n");
        return;
    }
    
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    terminal_writestring("PING ");
    terminal_writestring(target);
    terminal_writestring(" (Simulation Mode)\n");
    terminal_setcolor(vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    
    // Simulate ping responses
    for (int i = 0; i < 4; i++) {
        terminal_writestring("64 bytes from ");
        terminal_writestring(target);
        terminal_writestring(": icmp_seq=");
        char seq_str[8];
        itoa(i + 1, seq_str, 10);
        terminal_writestring(seq_str);
        terminal_writestring(" time=");
        
        // Simulate random response time (1-10ms)
        int response_time = 1 + (i % 10);
        char time_str[8];
        itoa(response_time, time_str, 10);
        terminal_writestring(time_str);
        terminal_writestring("ms\n");
        
        // Update interface statistics (simulate packet send/receive)
        network_interface_t* eth0 = network_find_interface_by_name("eth0");
        if (eth0) {
            eth0->packets_sent++;
            eth0->packets_received++;
            eth0->bytes_sent += 64;
            eth0->bytes_received += 64;
        }
        
        // Simple delay simulation
        for (int j = 0; j < 100000; j++) {
            asm volatile ("nop");
        }
    }
    
    terminal_writestring("\n--- ");
    terminal_writestring(target);
    terminal_writestring(" ping statistics ---\n");
    terminal_writestring("4 packets transmitted, 4 received, 0% packet loss\n");
}

// Network command handler
void network_command_handler(int argc, char argv[][64]) {
    if (argc < 2) {
        terminal_writestring("Network Commands:\n");
        terminal_writestring("  netinfo  - Show network interface information\n");
        terminal_writestring("  netstat  - Show network statistics\n");
        terminal_writestring("  ping <target> - Ping simulation\n");
        return;
    }
    
    if (net_strcmp(argv[1], "info") == 0) {
        network_show_interfaces();
    }
    else if (net_strcmp(argv[1], "stat") == 0) {
        network_show_stats();
    }
    else if (net_strcmp(argv[1], "ping") == 0) {
        if (argc >= 3) {
            network_ping_simulation(argv[2]);
        } else {
            network_ping_simulation("127.0.0.1");
        }
    }
    else {
        terminal_writestring("Unknown network command: ");
        terminal_writestring(argv[1]);
        terminal_writestring("\n");
    }
}