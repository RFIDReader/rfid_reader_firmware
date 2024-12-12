PORT=/dev/tty.usbmodem1201

# Check if PORT is available
if [ ! -e "$PORT" ]; then
  echo "Error: Port $PORT not found."
  exit 1
fi

# Build the project
idf.py build
if [ $? -ne 0 ]; then
  echo "Error: Build failed."
  exit 1
fi

# Flash the project to the device
idf.py flash -p $PORT
if [ $? -ne 0 ]; then
  echo "Error: Flashing failed."
  exit 1
fi

# Monitor the serial output
idf.py monitor -p $PORT
if [ $? -ne 0 ]; then
  echo "Error: Monitor failed."
  exit 1
fi
