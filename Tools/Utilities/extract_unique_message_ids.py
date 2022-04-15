import os

message_path = "Z:\ds3os\Temp\MessageId_Trace"

unique_ids = {}

for root, dirs, files in os.walk(message_path):
    for filename in files:
        message_values = filename.split("_")
        packet_id = message_values[1]
        packet_name = message_values[4].split(".")[0]

        if (packet_name not in unique_ids):
            unique_ids[packet_name] = packet_id

for key in unique_ids:
    print(str(key) + " = " + str(unique_ids[key]))
        