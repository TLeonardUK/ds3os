import os

input = "data.txt"

unknown_count = 1

with open(input) as f:
    lines = f.readlines()
    for line in lines:
        line_split = line.strip().split(":")
        id = line_split[0]
        display_name = line_split[1]

        if display_name == "":
            display_name = "Unknown " + str(unknown_count)
            unknown_count = unknown_count + 1

        ident = display_name.replace("'", "").replace("-", "_").replace(",", "").replace(" ", "_").replace("__", "_").replace("__", "_")

        print("ENTRY(" + (ident + ",").ljust(60) + ("\"" + display_name + "\",").ljust(60) + id + ")")
