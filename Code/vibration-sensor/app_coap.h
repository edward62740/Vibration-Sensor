/*
 * app_coap.h
 *
 *  Created on: Jun 2, 2023
 *      Author: edward62740
 */

#ifndef APP_COAP_H_
#define APP_COAP_H_

#include <openthread/coap.h>
#include <openthread/ip6.h>
#include "app_main.h"
#ifdef __cplusplus
extern "C" {
#endif




typedef enum {
	MSG_LIMITED_SPECTRUM, MSG_FULL_SPECTRUM, MSG_ALIVE,
} app_msg_t;

const uint16_t COAP_MAX_PAYLOAD_LENGTH = 64U; // max entries of X(k)
const uint16_t COAP_MAX_ITEM_SIZE = sizeof(uint32_t) + sizeof(uint16_t);
const uint16_t COAP_MAX_PAYLOAD_SIZE = COAP_MAX_ITEM_SIZE
		* COAP_MAX_PAYLOAD_LENGTH; // size of COAP_MAX_PAYLOAD_LENGTH in bytes
const static char *ack = "ack";
const static char *nack = "nack";

#define PERMISSIONS_URI "permissions"

class coapSender {

public:
	coapSender();
	~coapSender() { };
	otIp6Address selfAddr; // address of self
	otIp6Address brAddr; // address of target border router
	bool connectionEstablished = false; // indicates that the "connection" with upstream server is active


	char payload_buf[COAP_MAX_PAYLOAD_SIZE]; // application payload buffer
	bool payload_buf_valid = false;


	otCoapResource mResource_PERMISSIONS; // default resource for external server to allocate resource
	char resource_name[32]; // name of resource authorized to
	const char *mPERMISSIONSUriPath = PERMISSIONS_URI; // uri of default resource

	uint32_t failCtr = 0;
	uint32_t txCtr = 0;
	const uint8_t device_type = 2U;


	/**
		 @brief Handler that is called when external server sends unique address to the resource
		 @param aContext opaque pointer to context
		 @param aMessage otMessage containing payload
		 @param aMessageInfo otMessageInfo misc info
		 */
	void permissionsHandler(void *aContext, otMessage *aMessage,
			const otMessageInfo *aMessageInfo);
	/**
		 @brief Checks if the buffer is valid for transmission, i.e. it has valid contents
		 and has not been previously successfully sent.
		 @return true if valid false otherwise
		 */
	bool checkBuffer(void);
	/**
		 @brief Function that the application calls to trigger a transmission of the payload_buf.
		 @param require_ack Whether to request a coap ack from receiver.
		 @return true if sent successfully, false otherwise
		 */
	bool send(bool require_ack);
	/**
		 @brief Parse app_data_t magnitude and index data into a char buffer payload_buf.
		 @param data pointer to app_data_t
		 @param type type of message app_msg_t
		 @return true if successful false otherwise
		 */
	bool parseIntoBuffer(app_data_t *data, app_msg_t type);
	/**
		 @brief Checks if the current openthread connection is valid for sending a coap message.
		 @return true if valid false otherwise.
		 */
	bool checkConnectionValid(void);



	static void permissionsHandlerStaticWrapper(void *ctx, otMessage *msg,
			const otMessageInfo *msgInfo) {
		coapSender *instance = static_cast<coapSender*>(ctx);
		instance->permissionsHandler(ctx, msg, msgInfo);
	}


};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APP_COAP_H_ */
