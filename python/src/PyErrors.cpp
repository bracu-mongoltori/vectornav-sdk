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

// clang-format off
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>

#include <sstream>

#include "vectornav/HAL/Timer.hpp"
#include "vectornav/Interface/Stream.hpp"
#include "vectornav/Interface/Exceptions.hpp"

namespace py = pybind11;

namespace VN
{

void init_errors(py::module& m)
{
    auto vn_exception = py::register_exception<VnException>(m, "VnException");
    
    py::register_exception<VnSynchronousException>(m, "VnSynchronousException", vn_exception);
    py::register_exception<VnAsynchronousException>(m, "VnAsynchronousException", vn_exception);
    py::register_exception<VnCommandProcessorException>(m, "VnCommandProcessorException", vn_exception);
    py::register_exception<VnSensorException>(m, "VnSensorException", vn_exception);
    py::register_exception<VnSerialException>(m, "VnSerialException", vn_exception);
    py::register_exception<VnPacketSyncException>(m, "VnPacketSyncException", vn_exception);
    py::register_exception<VnFileException>(m, "VnFileException", vn_exception);
    py::register_exception<VnUnknownException>(m, "VnUnknownException", vn_exception);
#if __FIRMWARE_PROGRAMMER__ 
    py::register_exception<VnBLException>(m, "VnBLException", vn_exception);
#endif
#if __CALIBRATION__ 
    py::register_exception<VnCalException>(m, "VnCalException", vn_exception);
#endif

#if __TESTS__
  m.def("testExceptionThrow", &testExceptionThrow, py::arg("type"), py::arg("code"));
#endif

}
}
// clang-format on
