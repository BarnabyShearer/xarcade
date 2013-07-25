/*
  Copyright 2012  Dean Camera (dean [at] fourwalledcubicle [dot] com)
  Copyright 2013  Barnaby <b@Zi.iS>, Paul <cynarblackmane@gmail.com>

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaim all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Simple adapter for the Xarcade controller.
 */

#include "Xarcade.h"

const uint8_t Mapping[] PROGMEM = {
    HID_KEYBOARD_SC_A,
    HID_KEYBOARD_SC_B,
    HID_KEYBOARD_SC_C,
    HID_KEYBOARD_SC_D,
    HID_KEYBOARD_SC_E,
    HID_KEYBOARD_SC_F,
    HID_KEYBOARD_SC_G,
    HID_KEYBOARD_SC_H,
    HID_KEYBOARD_SC_I,
    HID_KEYBOARD_SC_J,
    HID_KEYBOARD_SC_K,
    HID_KEYBOARD_SC_L,
    HID_KEYBOARD_SC_M,
    HID_KEYBOARD_SC_N,
    HID_KEYBOARD_SC_O,
    HID_KEYBOARD_SC_P,
    HID_KEYBOARD_SC_Q,
    HID_KEYBOARD_SC_R,
    HID_KEYBOARD_SC_S,
    HID_KEYBOARD_SC_T,
    HID_KEYBOARD_SC_U,
    HID_KEYBOARD_SC_V,
    HID_KEYBOARD_SC_W,
    HID_KEYBOARD_SC_X,
    HID_KEYBOARD_SC_Y,
    HID_KEYBOARD_SC_Z,
    HID_KEYBOARD_SC_1_AND_EXCLAMATION,
    HID_KEYBOARD_SC_2_AND_AT,
    HID_KEYBOARD_SC_3_AND_HASHMARK,
    HID_KEYBOARD_SC_4_AND_DOLLAR,
    HID_KEYBOARD_SC_5_AND_PERCENTAGE,
    HID_KEYBOARD_SC_6_AND_CARET,
};


/** Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevKeyboardHIDReportBuffer[sizeof(USB_KeyboardReport_Data_32_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
     {
        .Config =
            {
                .InterfaceNumber              = 0,
                .ReportINEndpoint             =
                    {
                        .Address              = KEYBOARD_EPADDR,
                        .Size                 = KEYBOARD_EPSIZE,
                        .Banks                = 1,
                    },
                .PrevReportINBuffer           = PrevKeyboardHIDReportBuffer,
                .PrevReportINBufferSize       = sizeof(PrevKeyboardHIDReportBuffer),
            },
    };


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
    SetupHardware();

    GlobalInterruptEnable();

    for (;;)
    {
        HID_Device_USBTask(&Keyboard_HID_Interface);
        USB_USBTask();
    }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    DDRD = 0b00000000; //Set all port D pins to input
    PORTD = 0b11111111; //Enable pullups on all port D pins
    DDRB = 0b11111111; //Set all port B pins to output
    USB_Init();
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean true to force the sending of the report, false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
    USB_KeyboardReport_Data_32_t* KeyboardReport = (USB_KeyboardReport_Data_32_t*)ReportData;

    uint8_t UsedKeyCodes = 0;

    for(uint8_t key=0; key<8; key++) {
        PORTB = ~(1<<key);
        for(uint8_t x=0; x<4; x++) {
            if((PIND & (1<<x)) ^ (1<<x)) {
                KeyboardReport->KeyCode[UsedKeyCodes++] = pgm_read_byte(&(Mapping[x*8 + key]));
            }
        }
    }

    *ReportSize = sizeof(USB_KeyboardReport_Data_32_t);
    return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
    USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
    HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
}

