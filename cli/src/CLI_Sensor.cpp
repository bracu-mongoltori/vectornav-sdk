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

#include "vectornav/CLI_Sensor.hpp"

using namespace msclr::interop;

namespace VNSDK
{

void Sensor::Connect(String ^ portName, UInt32 baudRate) { Connect(portName, static_cast<BaudRate>(baudRate), false); }

void Sensor::Connect(String ^ portName, BaudRate baudRate) { Connect(portName, baudRate, false); }

void Sensor::Connect(String ^ portName, UInt32 baudRate, bool monitorAsyncErrors) { Connect(portName, static_cast<BaudRate>(baudRate), monitorAsyncErrors); }

void Sensor::Connect(String ^ portName, BaudRate baudRate, bool monitorAsyncErrors)
{
    marshal_context ^ context = gcnew marshal_context();
    throwIfError(_sensor->connect(context->marshal_as<const char*>(portName), Registers::System::ToNativeInstance(baudRate), monitorAsyncErrors));
}

void Sensor::Connect(String^ fileName)
{
    marshal_context ^ context = gcnew marshal_context();
    throwIfError(_sensor->connect(context->marshal_as<const char*>(fileName)));
}

void Sensor::Connect(String ^ ipAddress, String ^ portNumber) { Connect(ipAddress, portNumber, false); }

void Sensor::Connect(String ^ ipAddress, String ^ portNumber, bool monitorAsyncErrors)
{
    marshal_context ^ context = gcnew marshal_context();
    throwIfError(_sensor->connect(context->marshal_as<const char*>(ipAddress), context->marshal_as<const char*>(portNumber), monitorAsyncErrors));
}

void Sensor::AutoConnect(String ^ portName) { AutoConnect(portName, false); }

void Sensor::AutoConnect(String ^ portName, bool monitorAsyncErrors)
{
    marshal_context ^ context = gcnew marshal_context();
    throwIfError(_sensor->autoConnect(context->marshal_as<const char*>(portName), monitorAsyncErrors));
    delete context;
}

void Sensor::AutoConnect(String ^ ipAddress, String ^ portNumber) { AutoConnect(ipAddress, portNumber, false); }

void Sensor::AutoConnect(String ^ ipAddress, String ^ portNumber, bool monitorAsyncErrors)
{
    marshal_context ^ context = gcnew marshal_context();
    throwIfError(_sensor->autoConnect(context->marshal_as<const char*>(ipAddress), context->marshal_as<const char*>(portNumber), monitorAsyncErrors));
}



bool Sensor::VerifySensorConnectivity()
{
    return _sensor->verifySensorConnectivity();
}

String ^ Sensor::ConnectedPortName()
{
    auto connectedPortName = _sensor->connectedPortName();
    if (!connectedPortName.has_value()) { throwIfError(VN::Error::SerialPortClosed); }
    String ^ portName = msclr::interop::marshal_as<String ^>(connectedPortName.value().c_str());
    return portName;
}

uint32_t Sensor::ConnectedBaudRate()
{
    auto connectedBaudRate = _sensor->connectedBaudRate();
    if (!connectedBaudRate.has_value()) { throwIfError(VN::Error::SerialPortClosed); }
    return static_cast<uint32_t>(connectedBaudRate.value());
}

void Sensor::ChangeBaudRate(BaudRate baudRate)
{
    throwIfError(_sensor->changeBaudRate(Registers::System::ToNativeInstance(baudRate)));
}

void Sensor::ChangeBaudRate(UInt32 baudRate)
{
    throwIfError(_sensor->changeBaudRate(static_cast<VN::Registers::System::BaudRate::BaudRates>(baudRate)));
}

void Sensor::ChangeBaudRate(BaudRate baudRate, Registers::System::BaudRate::SerialPort serialPort)
{
    throwIfError(_sensor->changeBaudRate(Registers::System::ToNativeInstance(baudRate),
        static_cast<VN::Registers::System::BaudRate::SerialPort>(serialPort)));
}

void Sensor::ChangeBaudRate(UInt32 baudRate, System::Byte serialPort)
{
    throwIfError(_sensor->changeBaudRate(static_cast<VN::Registers::System::BaudRate::BaudRates>(baudRate),
        static_cast<VN::Registers::System::BaudRate::SerialPort>(serialPort)));
}

void Sensor::ChangeHostBaudRate(BaudRate baudRate)
{
    throwIfError(_sensor->changeHostBaudRate(Registers::System::ToNativeInstance(baudRate)));
}

void Sensor::ChangeHostBaudRate(UInt32 baudRate)
{
    throwIfError(_sensor->changeHostBaudRate(static_cast<VN::Registers::System::BaudRate::BaudRates>(baudRate)));
}

void Sensor::Disconnect()
{
    _sensor->disconnect();
}

// ----------------------
// Accessing Measurements
// ----------------------

bool Sensor::HasMeasurement()
{
    return _sensor->hasMeasurement();
}

Nullable<CompositeData> Sensor::GetNextMeasurement()
{
    auto measurement = _sensor->getNextMeasurement();
    if (measurement) { return CompositeData(*measurement); }
    return Nullable<CompositeData>();
}

Nullable<CompositeData> Sensor::GetMostRecentMeasurement()
{
    auto measurement = _sensor->getMostRecentMeasurement();
    if (measurement) { return CompositeData(*measurement); }
    return Nullable<CompositeData>();
}

// ----------------------
// Commands
// ----------------------

void Sensor::ReadRegister(VNSDK::Registers::ConfigRegister ^ reg)
{
    throwIfError(_sensor->readRegister(reg->GetReference()));
}

void Sensor::ReadRegister(VNSDK::Registers::MeasRegister ^ reg)
{
    throwIfError(_sensor->readRegister(reg->GetReference()));
}

void Sensor::WriteRegister(VNSDK::Registers::ConfigRegister ^ reg)
{
    throwIfError(_sensor->writeRegister((VN::ConfigurationRegister*)reg->GetReference()));
}

void Sensor::WriteSettings()
{
    throwIfError(_sensor->writeSettings());
}

void Sensor::Reset()
{
    throwIfError(_sensor->reset(_sensor->asyncErrorThrowingEnabled()));
}

void Sensor::RestoreFactorySettings()
{
    throwIfError(_sensor->restoreFactorySettings());
}

void Sensor::KnownMagneticDisturbance(KnownMagneticDisturbance::State state)
{
    throwIfError(_sensor->knownMagneticDisturbance(ToNativeInstance(state)));
}

void Sensor::KnownMagneticDisturbance(uint8_t state)
{
    throwIfError(_sensor->knownMagneticDisturbance(static_cast<VN::KnownMagneticDisturbance::State>(state)));
}

void Sensor::KnownAccelerationDisturbance(KnownAccelerationDisturbance::State state)
{
    throwIfError(_sensor->knownAccelerationDisturbance(ToNativeInstance(state)));
}

void Sensor::KnownAccelerationDisturbance(uint8_t state)
{
    throwIfError(_sensor->knownAccelerationDisturbance(static_cast<VN::KnownAccelerationDisturbance::State>(state)));
}

void Sensor::SetInitialHeading(float heading)
{
    throwIfError(_sensor->setInitialHeading(heading));
}

void Sensor::SetInitialHeading(Ypr ypr)
{
    VN::Ypr nativeYpr{ypr.yaw, ypr.pitch, ypr.roll};
    throwIfError(_sensor->setInitialHeading(nativeYpr));
}

void Sensor::SetInitialHeading(Quat quat)
{
    const VN::Quat nativeQuat{VN::Vec3f{quat.vector.x, quat.vector.y, quat.vector.z}, quat.scalar};
    throwIfError(_sensor->setInitialHeading(nativeQuat));
}

void Sensor::AsyncOutputEnable(AsyncOutputEnable::State state)
{
    throwIfError(_sensor->asyncOutputEnable(ToNativeInstance(state)));
}

void Sensor::AsyncOutputEnable(uint8_t state)
{
    throwIfError(_sensor->asyncOutputEnable(static_cast<VN::AsyncOutputEnable::State>(state)));
}

void Sensor::SetFilterBias()
{
    throwIfError(_sensor->setFilterBias());
}

void Sensor::SendCommand(GenericCommand^% commandToSend, Sensor::SendCommandBlockMode waitMode)
{
    VN::BridgeSensor::SendCommandBlockMode waitModeC = static_cast<VN::BridgeSensor::SendCommandBlockMode>(static_cast<int>(waitMode));
    throwIfError(_sensor->sendCommand(commandToSend->GetReference(), waitModeC)); 	
}

void Sensor::SendCommand(GenericCommand ^ % commandToSend, Sensor::SendCommandBlockMode waitMode, UInt64 waitLength)
{
    VN::BridgeSensor::SendCommandBlockMode waitModeC = static_cast<VN::BridgeSensor::SendCommandBlockMode>(static_cast<int>(waitMode));
    VN::Microseconds waitLengthC(waitLength);
    throwIfError(_sensor->sendCommand(commandToSend->GetReference(), waitModeC, waitLengthC)); 	
}

void Sensor::SendCommand(GenericCommand ^ % commandToSend, Sensor::SendCommandBlockMode waitMode, UInt64 waitLength, UInt64 timeoutThreshold)
{
    VN::BridgeSensor::SendCommandBlockMode waitModeC = static_cast<VN::BridgeSensor::SendCommandBlockMode>(static_cast<int>(waitMode));
    VN::Microseconds waitLengthC(waitLength);
    VN::Microseconds timeoutThresholdC(timeoutThreshold);
    throwIfError(_sensor->sendCommand(commandToSend->GetReference(), waitModeC, waitLengthC, timeoutThresholdC)); 	
}

void Sensor::SerialSend(String ^ msgToSend)
{
    marshal_context ^ context = gcnew marshal_context();
    auto msgChar = context->marshal_as<const char*>(msgToSend);
    _sensor->serialSend(msgChar, strlen(msgChar));
    delete context;
}

// ----------------------
// Error Handling
// ----------------------

void Sensor::ThrowIfAsyncError() {
    auto asyncError = _sensor->getNextAsyncError();
    if (asyncError)  { throwIfError(asyncError->error); }

}

System::DateTime Sensor::Now() 
{
    return _clock.now();
}

void Sensor::SetSerialTcpChangeBaudRate(SerialTcpChangeBaudRateDelegate^ callback)
{
    _serialTcpChangeBaudRateDelegate = callback;
    
    if (callback == nullptr)
    {
        _sensor->setSerialTcpChangeBaudRate({});
        return;
    }
    IntPtr functionPointer = System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(callback);
    auto nativeCallback = static_cast<bool(*)(uint32_t)>(functionPointer.ToPointer());
    std::function<bool(uint32_t)> stdCallback = nativeCallback;
    _sensor->setSerialTcpChangeBaudRate(stdCallback);
}

// ----------------------
// Additional logging
// ----------------------

void Sensor::SubscribeToMessage(ManagedQueuePointer^ queueToSubscribe, BinaryOutputMeasurements^ binaryOutputMeasurementFilter)
{
    throwIfError(_sensor->subscribeToMessage(queueToSubscribe->ToNativePointer(), binaryOutputMeasurementFilter->ToNativePointer()));
}

void Sensor::SubscribeToMessage(ManagedQueuePointer^ queueToSubscribe, BinaryOutputMeasurements^ binaryOutputMeasurementFilter, FaSubscriberFilterType filterType)
{
    throwIfError(_sensor->subscribeToMessage(queueToSubscribe->ToNativePointer(), binaryOutputMeasurementFilter->ToNativePointer(), ToNativeInstance(filterType)));
}

void Sensor::SubscribeToMessage(ManagedQueuePointer^ queueToSubscribe, System::String^ asciiHeaderFilter)
{
    std::string nativeString = msclr::interop::marshal_as<std::string>(asciiHeaderFilter);
    VN::AsciiHeader header(nativeString.c_str());
    throwIfError(_sensor->subscribeToMessage(queueToSubscribe->ToNativePointer(), header));
}

void Sensor::SubscribeToMessage(ManagedQueuePointer^ queueToSubscribe, System::String^ asciiHeaderFilter, AsciiSubscriberFilterType filterType)
{
    std::string nativeString = msclr::interop::marshal_as<std::string>(asciiHeaderFilter);
    VN::AsciiHeader header(nativeString.c_str());
    throwIfError(_sensor->subscribeToMessage(queueToSubscribe->ToNativePointer(), header, ToNativeInstance(filterType)));
}

void Sensor::SubscribeToMessage(ManagedQueuePointer^ queueToSubscribe, SyncByte sb)
{
    throwIfError(_sensor->subscribeToMessage(queueToSubscribe->ToNativePointer(), ToNativeInstance(sb)));
}

void Sensor::UnsubscribeFromMessage(ManagedQueuePointer^ queueToUnsubscribe, BinaryOutputMeasurements^ binaryOutputMeasurementFilter)
{
    _sensor->unsubscribeFromMessage(queueToUnsubscribe->ToNativePointer(), binaryOutputMeasurementFilter->ToNativePointer());
}

void Sensor::UnsubscribeFromMessage(ManagedQueuePointer^ queueToUnsubscribe, System::String^ asciiHeaderFilter)
{
    std::string nativeString = msclr::interop::marshal_as<std::string>(asciiHeaderFilter);
    VN::AsciiHeader header(nativeString.c_str());
    _sensor->unsubscribeFromMessage(queueToUnsubscribe->ToNativePointer(), header);
}

void Sensor::UnsubscribeFromMessage(ManagedQueuePointer^ queueToUnsubscribe, SyncByte syncByte)
{
    _sensor->unsubscribeFromMessage(queueToUnsubscribe->ToNativePointer(), ToNativeInstance(syncByte));
}

void Sensor::RegisterReceivedByteBuffer(ByteBuffer^ buffer)
{
    _sensor->registerReceivedByteBuffer(buffer->GetNativePointer());
}

void Sensor::DeregisterReceivedByteBuffer()
{
    _sensor->deregisterReceivedByteBuffer();
}

} // namespace VNSDK
