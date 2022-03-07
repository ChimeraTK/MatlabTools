#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Name: DUMMY3	 Device: sdm://./pci:mtcadummys3	 Firmware: 0	 Date: 	 Map: "$DIR"/mtcadummy.mapp" > ../example/print_all_devices_reference
echo "Name: DUMMY2	 Device: sdm://./pci:mtcadummys1	 Firmware: 0	 Date: 	 Map: "$DIR"/sequences.map" >> ../example/print_all_devices_reference
