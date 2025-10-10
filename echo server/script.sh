#!/bin/bash

# script to test the server - simple connect / multiple connect / stress connect
# Function to generate a UUID (simple version)
generate_uuid() {
    cat /proc/sys/kernel/random/uuid
}

# Test 1: Single proper client
echo "=== Test 1: Single proper client ==="
{
    # Send metadata in correct format
    echo "testclient!?!?$(generate_uuid)"
    sleep 0.5
    echo "Hello from client 1"
    sleep 0.5
    echo "Message 2"
    sleep 0.5
} | nc localhost 8080 &

sleep 2

# Test 2: Multiple proper clients
echo "=== Test 2: Multiple proper clients ==="
for i in {1..5}; do
    {
        echo "client${i}!?!?$(generate_uuid)"
        sleep 0.5
        echo "Message from client ${i}"
        sleep 0.5
    } | nc localhost 8080 &
done

wait

echo "=== Tests completed ==="

# Test 3: Stress test with rapid connections
echo "=== Test 3: Stress test ==="
for i in {1..10}; do
    (
        echo "stress${i}!?!?$(generate_uuid)"
        echo "test message"
        sleep 0.1
    ) | nc localhost 8080 2>/dev/null &
done

wait
echo "=== Stress test completed ==="
