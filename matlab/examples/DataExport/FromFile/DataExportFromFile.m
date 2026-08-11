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

vnsdkAssembly = NET.addAssembly([pwd '\..\..\..\..\net\VnSdk_Net.dll']); % change to file path
import VNSDK.* % Get rid of 'VNSDK' namespace qualifications

%{
 This data export example walks through the Matlab usage of the SDK to export a binary file to a CSV or ASCII file.

This example will achieve the following:
1. Create a FileExporter object
2. Add exporters for CSV, ASCII, and skipped bytes
3. Process file
4. Print parsing stats
%}


% Parse command line arguments
if exist('filename', 'var')
    outputDirectory = fileparts(filename);
    filePath = filename;
else
    outputDirectory = fileparts(mfilename('fullpath'));
    filePath = fullfile(outputDirectory, 'DataExportFromFile.bin');
end

% Verify the file exists
if ~exist(filePath, 'file')
    error('Input file does not exist: %s', filePath);
end

fprintf('Exporting %s\n', filePath);
fprintf('Outputting to %s\n\n', outputDirectory);

%% 1. Create a Sensor object for file processing
pushToCompositeData = MeasQueueMode.Off; 
mainBuffer = ByteBuffer(65536);
fbBuffer = ByteBuffer(2048);

sensor = Sensor(mainBuffer, fbBuffer, pushToCompositeData);

%% 2. Add exporters for CSV, ASCII, and and skipped bytes and subscribe to sensor messages
asciiExporter = VNSDK.("DataExport+ExporterAscii")(outputDirectory, VNSDK.("DataExport+PacketQueueMode").Retry);
sensor.SubscribeToMessage(asciiExporter.GetManagedQueuePointer(),"VN");

csvExporter = VNSDK.("DataExport+ExporterCsv")(outputDirectory, VNSDK.("DataExport+PacketQueueMode").Retry);
sensor.SubscribeToMessage(csvExporter.GetManagedQueuePointer(), "VN");

sensor.SubscribeToMessage(csvExporter.GetManagedQueuePointer(), SyncByte.FA);

skippedByteExporter = VNSDK.("DataExport+ExporterSkippedByte")(outputDirectory);
sensor.SubscribeToMessage(skippedByteExporter.GetManagedQueuePointer(), SyncByte.None);

%%% 3. Kick off exporter threads
asciiExporter.Start();
csvExporter.Start();
skippedByteExporter.Start();

%%% 4. Connect to file, monitoring AsyncError queue for FileReadFailed error to indicate end of file reached
sensor.Connect(filePath);
while (true)
    pause(0.001);
    % Handle async errors
    try
        sensor.ThrowIfAsyncError();
    catch asyncError
        if contains(asyncError.message, 'FileReadFailed')
            break;
        else
            fprintf('Received async error: %s\n',  asyncError.message);
        end
    end
end

sensor.Disconnect();

%%% 5. Stop Exporters
asciiExporter.Stop();
csvExporter.Stop();
skippedByteExporter.Stop();

disp('DataExportFromFile example complete');