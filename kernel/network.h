// ClaudeOS Network Foundation - Day 19 Basic Infrastructure
// Safe network abstraction without complex protocols

#ifndef NETWORK_H
#define NETWORK_H

#include "types.h"

// Network configuration constants (no hardcoding)
#define MAX_NETWORK_INTERFACES 4
#define MAX_PACKET_SIZE 1518          // Standard Ethernet frame size
#define PACKET_BUFFER_COUNT 32        // Number of packet buffers
#define NETWORK_QUEUE_SIZE 16         // Network queue depth

// Network interface types
typedef enum {
    NET_INTERFACE_LOOPBACK = 0,
    NET_INTERFACE_ETHERNET = 1,
    NET_INTERFACE_VIRTUAL = 2
} net_interface_type_t;

// Network interface states
typedef enum {
    NET_STATE_DOWN = 0,
    NET_STATE_UP = 1,
    NET_STATE_TESTING = 2
} net_interface_state_t;

// Basic packet structure (simplified)
typedef struct {
    uint8_t data[MAX_PACKET_SIZE];     // Packet data
    size_t size;                       // Actual packet size
    uint32_t timestamp;                // Packet timestamp
    bool in_use;                       // Buffer in use flag
    int interface_id;                  // Source/destination interface
} network_packet_t;

// Network interface structure (basic abstraction)
typedef struct {
    int id;                           // Interface ID
    char name[16];                    // Interface name (e.g., "eth0", "lo")
    net_interface_type_t type;        // Interface type
    net_interface_state_t state;      // Interface state
    uint8_t mac_address[6];           // MAC address (simulated)
    uint32_t ip_address;              // IP address (simulated)
    uint32_t packets_sent;            // Statistics: packets sent
    uint32_t packets_received;        // Statistics: packets received
    uint32_t bytes_sent;              // Statistics: bytes sent
    uint32_t bytes_received;          // Statistics: bytes received
    uint32_t errors;                  // Error count
    bool enabled;                     // Interface enabled flag
} network_interface_t;

// Network statistics
typedef struct {
    uint32_t total_packets_sent;
    uint32_t total_packets_received;
    uint32_t total_bytes_sent;
    uint32_t total_bytes_received;
    uint32_t total_errors;
    uint32_t active_interfaces;
    uint32_t buffer_usage;
} network_stats_t;

// Global variables
extern network_interface_t network_interfaces[MAX_NETWORK_INTERFACES];
extern network_packet_t packet_buffers[PACKET_BUFFER_COUNT];
extern int next_interface_id;
extern bool network_initialized;

// Function declarations

// Network system initialization
void network_init(void);
void network_shutdown(void);

// Interface management
int network_create_interface(const char* name, net_interface_type_t type);
int network_enable_interface(int interface_id);
int network_disable_interface(int interface_id);
network_interface_t* network_find_interface(int interface_id);
network_interface_t* network_find_interface_by_name(const char* name);

// Packet buffer management
network_packet_t* network_alloc_packet(void);
void network_free_packet(network_packet_t* packet);
int network_send_packet(int interface_id, const uint8_t* data, size_t size);
network_packet_t* network_receive_packet(int interface_id);

// Network statistics and monitoring
void network_get_stats(network_stats_t* stats);
void network_get_interface_stats(int interface_id, network_interface_t* stats);
void network_reset_stats(void);

// Network utility functions
const char* network_interface_type_string(net_interface_type_t type);
const char* network_interface_state_string(net_interface_state_t state);
void network_format_mac_address(const uint8_t* mac, char* buffer, size_t size);
void network_format_ip_address(uint32_t ip, char* buffer, size_t size);

// Network commands (safe simulation)
void network_command_handler(int argc, char argv[][64]);
void network_ping_simulation(const char* target);
void network_show_interfaces(void);
void network_show_stats(void);

#endif // NETWORK_H