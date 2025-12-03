import serial
import csv
import time

# Define the serial port and baud rate
SERIAL_PORT = 'COM6'  # Replace with your actual COM port (e.g., 'COM3' on Windows or '/dev/ttyUSB0' on Linux)
BAUD_RATE = 115200

# Open the serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE)

# Open the CSV file for writing
csv_file = open('data_dump.csv', mode='w', newline='')
csv_writer = csv.writer(csv_file)

# Write the header row to CSV
csv_writer.writerow(['Timestamp', 'Angle', 'Voltage1', 'Voltage2'])

print("Data dumping started. Press Ctrl+C to stop.")

try:
    while True:
        # Read the incoming serial data (one line)
        line = ser.readline().decode('utf-8').strip()
        
        # If the line starts with "#", it's an angle data packet
        if line.startswith('#'):
            try:
                # Split the data into angle and voltages (comma-separated)
                parts = line[1:].split(',')  # Removing '#' and then splitting
                if len(parts) == 3:
                    angle = int(parts[0])  # Angle data
                    voltage1 = int(parts[1])  # Voltage1
                    voltage2 = int(parts[2])  # Voltage2

                    # Write the data to the CSV file
                    timestamp = time.time()  # Get the current timestamp
                    csv_writer.writerow([timestamp, angle, voltage1, voltage2])

                    print(f"Angle: {angle}, Voltage1: {voltage1}, Voltage2: {voltage2}")
            except ValueError:
                # If there's a problem with parsing the data, just skip this line
                print("Error parsing angle data.")
        
        # You can also handle error packets here, if needed.
        # If the line starts with "!" or other prefixes, you can process those.
        elif line.startswith('!'):
            # print(f"Received error packet: {line}")
            pass

        elif line.startswith('%'):
            print("Received SHUTDOWN MESSAGE.")
        # Optional: slow down the loop so it doesn't flood the terminal
        
except KeyboardInterrupt:
    print("Data dumping stopped by user.")
    
finally:
    # Close the CSV file and the serial connection
    csv_file.close()
    ser.close()
    print("Data dump complete.")
