// VectorNav SDK (v1.2.0)
// Copyright (c) 2024 VectorNav Technologies, LLC
// 
// WARNING - This software contains Controlled Technical Data, export of which
// is restricted by the Export Administration Regulations ("EAR") (ECCN 7D994). 
// Disclosure to foreign persons contrary to the EAR is prohibited. Violations
// of these export laws and regulations are subject to severe civil and criminal
// penalties.
// 
// The MIT License (MIT)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

    using System;
    using System.IO;
    using VNSDK;

namespace SaveNonMatchingConfiguration
{
    class Program
    {
        static void Usage()
        {
            Console.WriteLine("Usage: SaveNonMatchingConfiguration [port] [inputFilePath or \"generic\"] [outputFilePath or \"generic\"]");
        }

        public enum ReadWriteType
        {
            Xml,
            Ascii,
            Generic
        }
    
        static int Main(string[] args)
        {
            /* This save configuration example walks through the C# usage of the SDK to save the configuration on 
            the VectorNav sensor that differs from the configuration specified. It save the configuration as an XML file, ASCII file,
            or generic user-defined lambda expression 
            
            This example will achieve the following:

            1. Handle input arguments
            2. Instantiate a Sensor object and use it to connect to the VectorNav unit
            3. Configure initial settings onto the sensor
            4. Save all non-matching values from the VectorNav unit as specified
            5. Disconnect from the VectorNav unit
            */

            // 1. Handle input arguments
            if (args.Length > 3)
            {
                Usage();
                return 1;
            }
            string portName = "COM1";
            if (args.Length > 0) { portName = args[0]; }
            string sourceDir = Directory.GetParent(Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location)).Parent.FullName;
            string inputFilePath = Path.Combine(sourceDir, "configSettingsIn.txt");
            string outputFilePath = Path.Combine(sourceDir, "configSettingsOut.txt");
            if (args.Length > 1) { inputFilePath = args[1]; }
            if (args.Length > 2) { outputFilePath = args[2]; }

            ReadWriteType rwType;
            if (inputFilePath == "generic") 
            { 
                if (outputFilePath != "generic")
                {
                    Console.WriteLine($"\nError: Output file type does not match input file type.");
                    return 1;
                }
                rwType = ReadWriteType.Generic; 
            }
            else
            {
                if (!File.Exists(inputFilePath))
                {   
                    Console.WriteLine($"\nError: File does not exist: {inputFilePath}.");
                    return 1;
                }
                string inputExtension = System.IO.Path.GetExtension(inputFilePath);
                string outputExtension = System.IO.Path.GetExtension(outputFilePath);
                if (string.IsNullOrEmpty(inputExtension) || string.IsNullOrEmpty(outputExtension))
                {
                    Console.WriteLine($"\nError: One of the file path extensions was not specified.");
                    return 1;
                }
                else if (inputExtension == ".xml") 
                { 
                    if (outputExtension != ".xml")
                    {
                        Console.WriteLine($"\nError: Output file type does not match input file type.");
                        return 1;
                    }
                    rwType = ReadWriteType.Xml; 
                }
                else 
                { 
                    if (outputExtension != ".txt")
                    {
                        Console.WriteLine($"\nError: Output file type does not match input file type.");
                        return 1;
                    }
                    rwType = ReadWriteType.Ascii; 
                }
            }

            // 2. Instantiate a Sensor object and use it to connect to the VectorNav unit
            Sensor sensor = new Sensor();
            try { sensor.AutoConnect(portName, true); } // enables throwing from async error monitoring thread  
            catch (VnException latestError)
            {
                Console.WriteLine($"{latestError.Message} encountered when connecting to {portName}.");
                return 1;
            }
            Console.WriteLine($"\nConnected to {portName} at {sensor.ConnectedBaudRate()}");
            
            // 3. Configure initial settings onto the sensor
            try { sensor.RestoreFactorySettings();}
            catch (VnException latestError)
            {
                Console.WriteLine($"{latestError.Message} encountered when restoring factory settings.");
                return 1;
            }

            VNSDK.Registers.System.AsyncOutputType asyncDataOutputType = new VNSDK.Registers.System.AsyncOutputType();
            asyncDataOutputType.ador = VNSDK.Registers.System.AsyncOutputType.Ador.YPR;
            asyncDataOutputType.serialPort = VNSDK.Registers.System.AsyncOutputType.SerialPort.Serial1;
        
            VNSDK.Registers.IMU.DeltaThetaVelConfig deltaThetaVelConfig = new VNSDK.Registers.IMU.DeltaThetaVelConfig();
            deltaThetaVelConfig.integrationFrame = VNSDK.Registers.IMU.DeltaThetaVelConfig.IntegrationFrame.NED; 
            deltaThetaVelConfig.gyroCompensation = VNSDK.Registers.IMU.DeltaThetaVelConfig.GyroCompensation.Bias; 
            deltaThetaVelConfig.accelCompensation = VNSDK.Registers.IMU.DeltaThetaVelConfig.AccelCompensation.Gravity;
            deltaThetaVelConfig.earthRateCompensation = VNSDK.Registers.IMU.DeltaThetaVelConfig.EarthRateCompensation.None; 
            deltaThetaVelConfig.resv = 0;
            try 
            { 
                sensor.WriteRegister(asyncDataOutputType);
                sensor.WriteRegister(deltaThetaVelConfig); 
            }
            catch (VnException latestError)
            {
                Console.WriteLine($"{latestError.Message} encountered when configuring register.");
                return 1;
            }
            Console.WriteLine($"Configured sensor initial settings.");

            // 4. Save all non-matching values from the VectorNav unit as specified
            if (rwType == ReadWriteType.Xml)
            {
                RegisterScan.XmlConfigReader configReader = new RegisterScan.XmlConfigReader(inputFilePath);
                RegisterScan.XmlConfigWriter configWriter = new RegisterScan.XmlConfigWriter(outputFilePath, sensor);
                try { RegisterScan.SaveNonMatchingConfiguration(sensor, configWriter,configReader);}
                catch (VnException latestError)
                {
                    Console.Write($"{latestError.Message} occured when saving configuration.");
                    return 1;
                }
            }
            else if (rwType ==  ReadWriteType.Ascii)
            {
                RegisterScan.AsciiConfigReader configReader = new RegisterScan.AsciiConfigReader(inputFilePath);
                RegisterScan.AsciiConfigWriter configWriter = new RegisterScan.AsciiConfigWriter(outputFilePath);
                try { RegisterScan.SaveNonMatchingConfiguration(sensor, configWriter,configReader);}
                catch (VnException latestError)
                {
                    Console.Write($"{latestError.Message} occured when saving configuration.");
                    return 1;
                }
            }
            else
            {
                Console.WriteLine("Generic SaveNonMatchingConfiguration example unimplemented.");
                return 1;
            }

            // 5. Disconnect from the VectorNav unit
            sensor.Disconnect();
            Console.WriteLine("SaveNonMatchingConfiguration example complete.");
            return 0;
        }
    }
}
    
    
