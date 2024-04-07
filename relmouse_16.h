/*
 * Copyright (c) 2018-2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// from https://github.com/arduino/ArduinoCore-mbed
// modified: header and source file combined
// modified: docs removed for brevity
// modified: made 16-bit

#include "USBHID.h"
#include "PlatformMutex.h"

#define REPORT_ID_MOUSE   2

#define X_MIN_REL    (-32767)
#define Y_MIN_REL    (-32767)
#define X_MAX_REL    (32767)
#define Y_MAX_REL    (32767)

class USBMouse16: public USBHID {
public:
    USBMouse16(bool connect_blocking = true, uint16_t vendor_id = 0x8234, uint16_t product_id = 0x0001, uint16_t product_release = 0x0001);

    USBMouse16(USBPhy *phy, uint16_t vendor_id = 0x8234, uint16_t product_id = 0x0001, uint16_t product_release = 0x0001);

    virtual ~USBMouse16();

    bool update(int16_t x, int16_t y, uint8_t buttons, int8_t z);
    
    virtual const uint8_t *report_desc();

protected:
    virtual const uint8_t *configuration_desc(uint8_t index);

private:
    uint8_t _configuration_descriptor[41];
    PlatformMutex _mutex;

};

#include "stdint.h"
#include "ThisThread.h"
#include "usb_phy_api.h"

using namespace arduino;
using namespace std::chrono_literals;

USBMouse16::USBMouse16(bool connect_blocking, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
    USBHID(get_usb_phy(), 0, 0, vendor_id, product_id, product_release)
{
}

USBMouse16::USBMouse16(USBPhy *phy, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
    USBHID(get_usb_phy(), 0, 0, vendor_id, product_id, product_release)
{
}

USBMouse16::~USBMouse16()
{
}

bool USBMouse16::update(int16_t x, int16_t y, uint8_t buttons, int8_t z)
{
    _mutex.lock();

    HID_REPORT report;
    report.data[0] = buttons;
    report.data[1] = (uint8_t)(x);
    report.data[2] = (uint8_t)(((uint16_t)x) >> 8);
    report.data[3] = (uint8_t)(y);
    report.data[4] = (uint8_t)(((uint16_t)y) >> 8);
    report.data[5] = -z; // >0 to scroll down, <0 to scroll up

    report.length = 6;

    bool ret = send(&report);

    _mutex.unlock();
    return ret;
}

const uint8_t *USBMouse16::report_desc()
{
    static const uint8_t report_descriptor[] = {
        USAGE_PAGE(1),      0x01,       // Genric Desktop
        USAGE(1),           0x02,       // Mouse
        COLLECTION(1),      0x01,       // Application
        USAGE(1),           0x01,       // Pointer
        COLLECTION(1),      0x00,       // Physical

        // Buttons
        REPORT_COUNT(1),    0x05,
        REPORT_SIZE(1),     0x01,
        USAGE_PAGE(1),      0x09,
        USAGE_MINIMUM(1),       0x01,
        USAGE_MAXIMUM(1),       0x05,
        LOGICAL_MINIMUM(1),     0x00,
        LOGICAL_MAXIMUM(1),     0x01,
        INPUT(1),           0x02,
        // Padding
        REPORT_COUNT(1),    0x01,
        REPORT_SIZE(1),     0x03,
        INPUT(1),           0x01,

        REPORT_COUNT(1),    2,
        REPORT_SIZE(1),     16,
        USAGE_PAGE(1),      1,
        USAGE(1),           0x30,       // X
        USAGE(1),           0x31,       // Y
        LOGICAL_MINIMUM(2),     0x01, 0x80,
        LOGICAL_MAXIMUM(2),     0xff, 0x7f,
        INPUT(1),           0x06,       // Relative data
        
        REPORT_COUNT(1),    1,
        REPORT_SIZE(1),     8,
        USAGE_PAGE(1),      1,
        USAGE(1),           0x38,       // scroll
        LOGICAL_MINIMUM(1),     0x81,
        LOGICAL_MAXIMUM(1),     0x7f,
        INPUT(1),           0x06,       // Relative data

        END_COLLECTION(0),
        END_COLLECTION(0),
    };
    reportLength = sizeof(report_descriptor);
    return report_descriptor;
}

#define DEFAULT_CONFIGURATION (1)
#define TOTAL_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (1 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (1 * HID_DESCRIPTOR_LENGTH) \
                               + (2 * ENDPOINT_DESCRIPTOR_LENGTH))

const uint8_t *USBMouse16::configuration_desc(uint8_t index)
{
    if (index != 0) {
        return NULL;
    }
    uint8_t configuration_descriptor_temp[] = {
        CONFIGURATION_DESCRIPTOR_LENGTH,    // bLength
        CONFIGURATION_DESCRIPTOR,           // bDescriptorType
        LSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (LSB)
        MSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (MSB)
        0x01,                               // bNumInterfaces
        DEFAULT_CONFIGURATION,              // bConfigurationValue
        0x00,                               // iConfiguration
        C_RESERVED | C_SELF_POWERED,        // bmAttributes
        C_POWER(0),                         // bMaxPower

        INTERFACE_DESCRIPTOR_LENGTH,        // bLength
        INTERFACE_DESCRIPTOR,               // bDescriptorType
        0x00,                               // bInterfaceNumber
        0x00,                               // bAlternateSetting
        0x02,                               // bNumEndpoints
        HID_CLASS,                          // bInterfaceClass
        HID_SUBCLASS_BOOT,                  // bInterfaceSubClass
        HID_PROTOCOL_MOUSE,                 // bInterfaceProtocol
        0x00,                               // iInterface

        HID_DESCRIPTOR_LENGTH,              // bLength
        HID_DESCRIPTOR,                     // bDescriptorType
        LSB(HID_VERSION_1_11),              // bcdHID (LSB)
        MSB(HID_VERSION_1_11),              // bcdHID (MSB)
        0x00,                               // bCountryCode
        0x01,                               // bNumDescriptors
        REPORT_DESCRIPTOR,                  // bDescriptorType
        (uint8_t)(LSB(report_desc_length())), // wDescriptorLength (LSB)
        (uint8_t)(MSB(report_desc_length())), // wDescriptorLength (MSB)

        ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
        ENDPOINT_DESCRIPTOR,                // bDescriptorType
        _int_in,                            // bEndpointAddress
        E_INTERRUPT,                        // bmAttributes
        LSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
        MSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)
        1,                                  // bInterval (milliseconds)

        ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
        ENDPOINT_DESCRIPTOR,                // bDescriptorType
        _int_out,                           // bEndpointAddress
        E_INTERRUPT,                        // bmAttributes
        LSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
        MSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)
        1,                                  // bInterval (milliseconds)
    };
    MBED_ASSERT(sizeof(configuration_descriptor_temp) == sizeof(_configuration_descriptor));
    memcpy(_configuration_descriptor, configuration_descriptor_temp, sizeof(_configuration_descriptor));
    return _configuration_descriptor;
}
