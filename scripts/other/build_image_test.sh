#!/bin/bash
set -e

# Create build directory
mkdir -p build/test_image_classes

# Compile the MIDlet
javac -source 1.8 -target 1.8 \
    -cp stubs/rt.jar \
    -d build/test_image_classes \
    tests/TestImageMIDlet.java

# Copy the image to the classes directory (so it's at the root)
cp tests/graphics/title.gif build/test_image_classes/title.gif

# Create Manifest
echo "MIDlet-1: TestImageMIDlet, , TestImageMIDlet" > build/test_image_classes/MANIFEST.MF
echo "MIDlet-Name: TestImageMIDlet" >> build/test_image_classes/MANIFEST.MF
echo "MIDlet-Vendor: Trae" >> build/test_image_classes/MANIFEST.MF
echo "MIDlet-Version: 1.0" >> build/test_image_classes/MANIFEST.MF
echo "MicroEdition-Configuration: CLDC-1.1" >> build/test_image_classes/MANIFEST.MF
echo "MicroEdition-Profile: MIDP-2.0" >> build/test_image_classes/MANIFEST.MF

# Package the JAR
jar cfm tests/TestImage.jar build/test_image_classes/MANIFEST.MF -C build/test_image_classes .

echo "tests/TestImage.jar created successfully"
