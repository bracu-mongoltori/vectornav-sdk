% VectorNav SDK (v1.2.0)
% Copyright (c) 2024 VectorNav Technologies, LLC
% 
% WARNING - This software contains Controlled Technical Data, export of which
% is restricted by the Export Administration Regulations ("EAR") (ECCN 7D994). 
% Disclosure to foreign persons contrary to the EAR is prohibited. Violations
% of these export laws and regulations are subject to severe civil and criminal
% penalties.
% 
% The MIT License (MIT)
% 
% Permission is hereby granted, free of charge, to any person obtaining a copy
% of this software and associated documentation files (the "Software"), to deal
% in the Software without restriction, including without limitation the rights
% to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
% copies of the Software, and to permit persons to whom the Software is
% furnished to do so, subject to the following conditions:
% 
% The above copyright notice and this permission notice shall be included in
% all copies or substantial portions of the Software.
% 
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
% IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
% FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
% AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
% LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
% OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
% THE SOFTWARE.

% Add the VNSDK assembly (change to your file path)
vnsdkAssembly = NET.addAssembly([pwd '\..\..\..\..\net\VnSdk_Net.dll']);
import VNSDK.* % Get rid of 'VNSDK' namespace qualifications

%{
This save configuration example walks through the MATLAB usage of the SDK to save the configuration on 
the VectorNav sensor that differs from the configuration specified. It save the configuration as an XML file, ASCII file,
or generic user-defined lambda expression 

This example will achieve the following:

1. Handle input arguments
2. Instantiate a Sensor object and use it to connect to the VectorNav unit
3. Configure initial settings onto the sensor
4. Save all non-matching values from the VectorNav unit as specified
5. Disconnect from the VectorNav unit
%}

%% 1. Handle input arguments
if ~exist('portName', 'var')
    portName = 'COM1';
end

if ~exist('inputFilePath', 'var')
    outputDirectory = fileparts(mfilename("fullpath"));
    inputFilePath = fullfile(outputDirectory, 'configSettingsIn.xml');
end

if ~exist('outputFilePath', 'var')
    outputDirectory = fileparts(mfilename("fullpath"));
    outputFilePath = fullfile(outputDirectory, 'configSettingsOut.xml');
end

if strcmpi(inputFilePath, 'generic')
    if ~strcmpi(outputFilePath, 'generic')
        fprintf('\nError: Output file type does not match input file type.\n');
        return;
    end
    rwType = 'generic';
else
    if ~isfile(inputFilePath)
        fprintf('\nError: File does not exist: %s\n', inputFilePath);
        return;
    end
    [~,~,inputExtension] = fileparts(inputFilePath);
    [~,~,outputExtension] = fileparts(outputFilePath);
    if isempty(inputExtension) ||  isempty(outputExtension)
        fprintf('\nError: One of the file path extensions was not specified.\n');
        return;
    elseif strcmpi(inputExtension, '.xml')
        if ~strcmpi(outputExtension, '.xml')
            fprintf('\nError: Output file type does not match input file type.\n');
            return;
        end
        rwType = 'xml';
    else
        if ~strcmpi(outputExtension, '.txt')
            fprintf('\nError: Output file type does not match input file type.\n');
            return;
        end
        rwType = 'ascii';
    end
end

%% 2. Instantiate a Sensor object and use it to connect to the VectorNav unit
disp('Instantiating a sensor and connecting...');
if exist('sensor','var')
    sensor.Disconnect();
else
    sensor = Sensor();
end
try
    sensor.AutoConnect(portName, true);
catch latestError
    error('Error encountered when connecting to %s.\n%s\n', portName, latestError.message);
end
fprintf('\nConnected to %s at %d\n\n', portName, sensor.ConnectedBaudRate());

%% 3. Configure initial settings onto the sensor
try
    sensor.RestoreFactorySettings();
catch latestError
    error('Error: %s encountered when restoring factory settings.', latestError);
end
asyncDataOutputType = Registers.System.AsyncOutputType();
asyncDataOutputType.ador = Registers.System.('AsyncOutputType+Ador').YPR;
asyncDataOutputType.serialPort = Registers.System.('AsyncOutputType+SerialPort').Serial1;

deltaThetaVelConfig = Registers.IMU.DeltaThetaVelConfig();
deltaThetaVelConfig.integrationFrame = Registers.IMU.('DeltaThetaVelConfig+IntegrationFrame').NED;
deltaThetaVelConfig.gyroCompensation = Registers.IMU.('DeltaThetaVelConfig+GyroCompensation').Bias;
deltaThetaVelConfig.accelCompensation = Registers.IMU.('DeltaThetaVelConfig+AccelCompensation').Gravity;
deltaThetaVelConfig.earthRateCompensation = Registers.IMU.('DeltaThetaVelConfig+EarthRateCompensation').None;
deltaThetaVelConfig.resv = 0;
try
    sensor.WriteRegister(asyncDataOutputType);
    sensor.WriteRegister(deltaThetaVelConfig);
catch latestError
    error('Error encountered when configuring register');
end
disp('Configured sensor initial settings.');

%% 4. Save all non-matching values from the VectorNav unit as specified
if strcmpi(rwType, 'xml')
    configReader = VNSDK.('RegisterScan+XmlConfigReader')(inputFilePath);
    configWriter = VNSDK.('RegisterScan+XmlConfigWriter')(outputFilePath, sensor);
elseif strcmpi(rwType, 'ascii')
    configReader = VNSDK.('RegisterScan+AsciiConfigReader')(inputFilePath);
    configWriter = VNSDK.('RegisterScan+AsciiConfigWriter')(outputFilePath);
else
    fprintf('Generic SaveNonMatchingConfiguration example unimplemented.\n');
    sensor.Disconnect();
    return;
end

try
    RegisterScan.SaveNonMatchingConfiguration(sensor, configWriter, configReader);
catch latestError
    error('Error occured when saving configuration. \n%s\n', latestError.message);
end

%% 6. Disconnect from the VectorNav unit
sensor.Disconnect();
fprintf('SaveNonMatchingConfiguration example complete.');