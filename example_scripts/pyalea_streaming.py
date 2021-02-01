import time
from alea import AleaTracker

# Add your Alea API key here:
alea_api_key = "API-KEY-HERE"

# Initialise the connection to the IntelliGaze Server.
tracker = AleaTracker(alea_api_key)

# You don't have to specify the following, but you can.
# (The SDK falls back to defaults automatically.)
#tracker = AleaTracker(alea_api_key, target_ip="127.0.0.1", \
#    target_port=27412, listen_ip="127.0.0.1", listen_port=27413)

# Calibrate the eye tracker, using the default options.
tracker.calibrate()

# Run for 10 seconds.
t0 = time.time()
while time.time() - t0 < 10.0:
    # Get the latest sample
    time_stamp, gaze_x, gaze_y, pupil_size = tracker.sample()
    # Print the current sample to the terminal.
    print("t={}, x={}, y={}, s={}".format( \
        round(time_stamp, 3), \
        round(gaze_x, 2), \
        round(gaze_y, 2), \
        round(pupil_size, 2)))
    # Wait for 33 milliseconds.
    time.sleep(0.033)

# Close the connection to the eye tracker.
tracker.close()
