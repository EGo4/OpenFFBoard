/*
 * UsbHidHandler.h
 *
 *  Created on: 12.02.2020
 *      Author: Yannick
 */

#ifndef USBHIDHANDLER_H_
#define USBHIDHANDLER_H_
#include "cppmain.h"
#include "ffb_defs.h"
#include "hid_cmd_defs.h"


class UsbHidHandler {
public:
	static UsbHidHandler* globalHidHandler;
	static std::vector<UsbHidHandler*> hidCmdHandlers; // called only for custom cmd report ids
	UsbHidHandler();
	virtual ~UsbHidHandler();
	virtual void hidOutCmd(HID_Custom_Data_t* data); // Called when the custom command report is received
	virtual uint8_t sendHidCmd(HID_Custom_Data_t* data);
	virtual void hidOut(uint8_t* report);
	virtual void hidGet(uint8_t id,uint16_t len,uint8_t** return_buf);
	void registerHidCallback();
};



#endif /* USBHIDHANDLER_H_ */
