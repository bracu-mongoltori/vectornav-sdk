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

#include "vectornav/HAL/Serial_Base.hpp"
#include "vectornav/Implementation/AsciiHeader.hpp"
#include "vectornav/Implementation/CoreUtils.hpp"
#include "vectornav/Interface/Registers.hpp"

#include "PyTemplates.hpp"

namespace py = pybind11;
namespace VN
{

void init_utils(py::module& m)
{
  declare_string<AsciiMessage>(m, "AsciiMessage");
  declare_string<AsciiParameter>(m, "AsciiParameter");
  declare_string<Serial_Base::PortName>(m, "PortName");
  declare_string<AsciiHeader>(m, "AsciiHeader");
  declare_string<RegisterString>(m, "RegisterString");

  py::implicitly_convertible<py::str, AsciiMessage>();
  py::implicitly_convertible<py::str, AsciiParameter>();
  py::implicitly_convertible<py::str, AsciiHeader>();
  py::implicitly_convertible<py::str, Serial_Base::PortName>();
  py::implicitly_convertible<py::str, RegisterString>();

  m.def("calculateChecksum", [](py::bytes buffer) -> uint8_t {
        std::string raw_buffer = buffer;
        return calculateChecksum(reinterpret_cast<uint8_t*>(raw_buffer.data()), raw_buffer.size());
    }, "Calculates 8-bit checksum from byte buffer");

  m.def("calculateCRC", [](py::bytes buffer) -> uint16_t {
        std::string raw_buffer = buffer;
        return calculateCRC(reinterpret_cast<uint8_t*>(raw_buffer.data()), raw_buffer.size());
    }, "Calculates 16-bit CRC from byte buffer");

  m.def("frameVnAsciiString", [](const std::string& unframedString) -> std::string {
    String<500> framedString;
    String<500> unframedVnString(unframedString);

    bool result = frameVnAsciiString(unframedVnString, framedString);

    if (result) {
        throw std::runtime_error("C++ function returned true, triggering exception");
    }

    return framedString.to_string();
  }, "Frames an ASCII command string according to VN protocol");

}}
// clang-format on
