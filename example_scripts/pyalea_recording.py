import time
from alea import AleaTracker

# Add your Alea API key here:
alea_api_key = "API-KEY-HERE"

# Initialise the connection to the IntelliGaze Server.
# Toggle the alea_logging keyword to swap between the Alea SDK
# and the PyAlea methods.
tracker = AleaTracker(alea_api_key, alea_logging=True)

# Calibrate the eye tracker, using the default options.
tracker.calibrate()

# Start recording.
tracker.start_recording()

# Log an event every 2 seconds.
for i in range(5):
    tracker.log("Trigger {}".format(i+1))
    time.sleep(2.0)

# Stop recording.
tracker.stop_recording()

# Close the connection to the eye tracker.
tracker.close()
