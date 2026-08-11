# VectorNav SDK (v1.2.0)
# Copyright (c) 2024 VectorNav Technologies, LLC
# 
# WARNING - This software contains Controlled Technical Data, export of which
# is restricted by the Export Administration Regulations ("EAR") (ECCN 7D994). 
# Disclosure to foreign persons contrary to the EAR is prohibited. Violations
# of these export laws and regulations are subject to severe civil and criminal
# penalties.
# 
# The MIT License (MIT)
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

import os
import sys
from enum import Enum

from vectornav import Registers, Sensor, VnException
from vectornav.Plugins import RegisterScan


usage = '[port] [inputFilePath (or "generic")] [outputFilePath (or "generic")]'

class ReadWriteType(Enum):
    Xml = 1
    Ascii = 2
    Generic = 3

def main(argv):
    """"
    This save configuration example walks through the Python usage of the SDK to save the configuration on 
    the VectorNav sensor that differs from the configuration specified. It save the configuration as an XML file, ASCII file.
    This example will achieve the following:

    1. Handle input arguments
    2. Instantiate a Sensor object and use it to connect to the VectorNav unit
    3. Configure initial settings onto the sensor
    4. Save all non-matching values from the VectorNav unit as specified
    5. Disconnect from the VectorNav unit
    """

    # 1. Handle input arguments
    current_folder = os.getcwd()
    if len(argv) > 3:
        print("Usage: python saveNonDefaultConfiguration", usage)
        sys.exit(1)
    if len(argv) > 0:
        portName = argv[0]
    else:
        portName = "COM1"  # Pass in the portname as a positional argument or edit here

    if len(argv) > 1:
        inputFilePath = argv[1]
    else:
        inputFilePath = os.path.join(current_folder, "configSettingsIn.txt")

    if len(argv) > 2:
        outputFilePath = argv[2]
    else:
        outputFilePath = os.path.join(current_folder, "configSettingsOut.txt")

    if inputFilePath == "generic":
        if outputFilePath != "generic":
            print("Error: Output file type does not match input file type.")
            sys.exit(1)
        rwType = ReadWriteType.Generic
    else:
        if not os.path.exists(inputFilePath):
            print("Error: File does not exist: " , inputFilePath)
        _, inputExtension = os.path.splitext(inputFilePath)
        _, outputExtension = os.path.splitext(outputFilePath)
        if inputExtension == "" or outputExtension == "":
            print("Error: One of the file path extensions was not specified: ")
            sys.exit(1)
        elif inputExtension == ".xml":
            if outputExtension != ".xml":
                print("Error: Output file type does not match input file type.")
                sys.exit(1)
            rwType = ReadWriteType.Xml
        else:
            if outputExtension != ".txt":
                print("Error: Output file type does not match input file type.")
                sys.exit(1)
            rwType = ReadWriteType.Ascii

    # 2. Instantiate a Sensor object and use it to connect to the VectorNav unit
    sensor = Sensor()
    try:
        sensor.autoConnect(portName, True)  # enables throwing from async error monitoring thread
    except VnException as latestError:
        print(f"{latestError} encountered when connecting to {portName}.\n")
        sys.exit(1)
    print(f"Connected to {portName} at {sensor.connectedBaudRate()}")

    # 3. Configure initial settings onto the sensor
    try:
        sensor.restoreFactorySettings()
    except VnException as latestError:
        print(f"{latestError} encountered when restoring factory settings.")
        sys.exit(1)
    asyncDataOutputType = Registers.System.AsyncOutputType()
    asyncDataOutputType.ador = Registers.System.AsyncOutputType.Ador.YPR
    asyncDataOutputType.serialPort = Registers.System.AsyncOutputType.SerialPort.Serial1
    
    deltaThetaVelConfig = Registers.IMU.DeltaThetaVelConfig()
    deltaThetaVelConfig.integrationFrame = Registers.IMU.DeltaThetaVelConfig.IntegrationFrame.NED
    deltaThetaVelConfig.gyroCompensation = Registers.IMU.DeltaThetaVelConfig.GyroCompensation.Bias
    deltaThetaVelConfig.accelCompensation = Registers.IMU.DeltaThetaVelConfig.AccelCompensation.Gravity
    deltaThetaVelConfig.earthRateCompensation = Registers.IMU.DeltaThetaVelConfig.EarthRateCompensation.none
    deltaThetaVelConfig.resv = 0

    try:
        sensor.writeRegister(asyncDataOutputType)
        sensor.writeRegister(deltaThetaVelConfig)
    except VnException as latestError:
        print(f"{latestError} encountered when configuring register.\n")
        sys.exit(1)
    print(f"Configured sensor initial settings.")
    
    # 4. Save all non-matching values from the VectorNav unit as specified
    if rwType == ReadWriteType.Xml:
        configReader = RegisterScan.XmlConfigReader(inputFilePath)
        configWriter = RegisterScan.XmlConfigWriter(sensor, outputFilePath)
    elif rwType == ReadWriteType.Ascii:
        configReader = RegisterScan.AsciiConfigReader(inputFilePath)
        configWriter = RegisterScan.AsciiConfigWriter(outputFilePath)
    else:
        print("Generic saveNonMatchingConfiguration example unimplemented.")
        sensor.disconnect()
        sys.exit(1)

    try:
        RegisterScan.saveNonMatchingConfiguration(sensor, configWriter, configReader)
    except VnException as latestError:
        print(f"{latestError} occured when saving configuration.")
        sys.exit(1)

    # 5. Disconnect from the VectorNav unit
    sensor.disconnect()
    print("SaveNonMatchingConfiguration example complete")

if __name__ == "__main__":
    main(sys.argv[1:])
