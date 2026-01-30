#!/bin/bash

# Compile the test MIDlet
javac -cp "stubs" ResourceTestMIDlet.java

# Create a manifest file
cat > MANIFEST.MF << EOF
Manifest-Version: 1.0
MIDlet-Name: Resource Test
MIDlet-Version: 1.0
MIDlet-Vendor: Test Vendor
MIDlet-1: Resource Test,,ResourceTestMIDlet
EOF

# Create a JAR file
jar cfm resource_test.jar MANIFEST.MF ResourceTestMIDlet.class

# Run the test in J2ME VM
echo "Running resource test in J2ME VM..."
./build/j2me-vm resource_test.jar
