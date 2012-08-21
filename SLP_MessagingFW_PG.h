/**
 *
 * @ingroup   SLP_PG
 * @defgroup   MESSAGE MessagingFW
@{
<h1 class="pg">Introduction</h1>
	<h2 class="pg">Purpose</h2>
The purpose of this document is to describe how applications can use the Messaging Framework APIs to send and receive SMS and MMS messages. This document gives programming guidelines to application engineers.

	<h2 class="pg">Scope</h2>
The scope of this document is limited to Messaging Framework APIs usage.

	<h2 class="pg">Abbreviations</h2>
<table>
<tr><td>API</td><td>Application Programming Interface</td></tr>
</table>
@}

@defgroup Message_Architecture 1.Archtecture
@ingroup MESSAGE
@{
<h1>Messaging Framework Architecture</h1>
The Messaging framework supports various messaging services such as SMS, MMS, Cell Broadcast, WAP Push, and Provisioning message. The Messaging framework architecture consists of a Messaging daemon and a library. The messaging library works on application process and provides various APIs to support transport, control, storage, filter and setting services for application. The messaging daemon has three components (transaction manager, transaction handler, and plug-in manager) to provide message services. The socket IPC is used to communicate (request & event) between Messaging daemon and library.
@image html messaging_image001.png

- Transaction Manager
	- Receive the IPC message (request) on socket.
	- Manage list of pairs (request ID, transaction proxy) for synchronous return.
	- Determine the transaction flow based on request/event information. (Mapping the request to one of the handlers)
	- Store the transaction information for asynchronous events (Find which transaction proxy want to receive SMS)
- Transaction Handler
	- Submit handler deals with submit requests.
	- Deliver handler deals with the incoming message from plug-ins.
	- Storage handler deals with the messages, accounts and folder requests
	- Filter handler deals with the filter requests
	- Setting handler deals with the service-specific attributes
- Plug-in Manager
	- Initialize all plug-ins after loading plug-in configuration files
	- Holding the list of plug-ins and the state of plug-ins
	- Providing the interface between handlers and plug-ins
- Transaction Handlers
	- Initialize all plug-ins after loading plug-in configuration files
	- Receiving IPC messages from messaging daemon.
	- Handling synchronous calls
		- Condition variable is used for waiting the return from msg. server)
	- Handling asynchronous calls or events
		- Proxy listener is a component of transaction proxy
		- Proxy listener is waiting for the event
		- Invoking callback function in the msg. handle list
		- i.e. calling incoming callback function of MSG APP handle, when incoming msg
	- Holding list of message handles
		- Message handle is created by MsgOpenMsgHandle(msgHandle)
		- Message handle holds some info such as callback func
- Message API
	- Transport & control API
	- Storage API
	- Filter API
	- Setting API
@}

@defgroup Message_Feature 2.Feature
@ingroup MESSAGE
@{
	<h2 class="pg">Messaging Framework Features</h2>
-# Message Control Features:
	-# Open/Close message handle
-# Message Transport Features:
	-# Submit request to send, forward, reply, retrieve message.
	-# Register callback functions to listen to invoked events from messaging daemon. (message status callback, incoming message callback)
-# Message Storage Features:
	-# Add / Update / Move / Delete / Count / Get message or message list.
	-# Add / Update / Delete / Get folder or folder list.
-# Message Filter Features:
	-# Add / Update / Delete filter or filter list.
-# Message Setting Features:
	-# Set / Get various message setting (i.e. whether to send read/delivery report or not)
@}

@defgroup MESSAGE_USECASES_1 Initialize/Finalization
@ingroup MESSAGE_USECASES
@{
	<h2 class="pg">Messaging Framework Functions</h2>

		<h3 class="pg">Initialization / Finalization to use Messaging Service </h3>
- int msg_open_msg_handle(MSG_HANDLE_T *handle);
@n msg_open_msg_handle() should be called before using the messaging service. This function opens a channel between the application and the messaging framework.
- int msg_close_msg_handle(MSG_HANDLE_T *handle);
@n msg_close_msg_handle() should be called after using the messaging service. This function closes a channel between the application and the messaging framework.
- int msg_reg_sent_status_callback(MSG_HANDLE_T handle, msg_sent_status_cb cb);
@n Application should implement a msg_sent_status_cb function and register it into message handle. If the application sends a message, this callback function will be called to report its sending status. msg_reg_set_status_callback function should be called after creation of message handle.
- int msg_reg_sms_message_callback(MSG_HANDLE_T handle, msg_sms_incoming_cb cb, unsigned short port);
@n Application should implement a msg_sms_incoming_cb function and register it into message handle. It’s used to listen to the SMS incoming event invoked by messaging daemon. The incoming message information structure (MSG_MESSAGE_S) can be delivered as a parameter of the callback function.
- int msg_reg_mms_conf_message_callback(MSG_HANDLE_T handle, msg_mms_conf_msg_incoming_cb cb, char *app_id);
@n Application should implement a msg_mms_conf_msg_incoming_cb function and register it into message handle. It’s used to listen to the MMS incoming event invoked by messaging daemon. The incoming message information structure (MSG_MESSAGE_S) can be delivered as a parameter of the callback function.
@code
#include <stdio.h>
#include <glib.h>
#include <MapiControl.h>
#include <MapiTransport.h>

void sentStatusCB(MSG_HANDLE_T hMsgHandle, MSG_SENT_STATUS_S *pMsgStatus, void *user_param)
{
	// Application can handle message sent status event
}

void incomingSmsMessageCB (MSG_HANDLE_T hMsgHandle, msg_message_t msg, void *user_param)
{
	// Application can handle SMS incoming message.
}

void incomingMmsConfMessageCB(MSG_HANDLE_T hMsgHandle, msg_message_t msg, void *user_param)
{
	// Application can handle MMS incoming message.
}

int main(void)
{
	MSG_HANDLE_T msgHandle = NULL;

	err = msg_open_msg_handle(&msgHandle);

	if (err != MSG_SUCCESS)
	{
		printf("msg_open_msg_handle() Fail [%d]", err);
		return err;
	}

	// Register sent status handler
	err = msg_reg_sent_status_callback(msgHandle, &sentStatusCB, NULL);

	if (err != MSG_SUCCESS)
	{
		printf("msg_reg_sent_status_callback() Fail [%d]", err);
		return err;
	}

	// Register SMS incoming message handler
	err = msg_reg_sms_message_callback(msgHandle, &incomingSmsMessageCB, 0, NULL);

	if (err != MSG_SUCCESS)
	{
		printf("msg_reg_sms_message_callback() Fail [%d]", err);
		return err;
	}

	// Register MMS incoming message handler
	err = msg_reg_mms_conf_message_callback(msgHandle, &incomingMmsConfMessageCB, NULL, NULL);

	if (err != MSG_SUCCESS)
	{
		printf("msg_reg_mms_conf_message_callback() Fail [%d]", err);
		return err;
	}

	// g_main_loop should be used to listen CB event from messaging daemon
	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(mainloop);

	msg_close_msg_handle(&msgHandle);

	return err;
}
@endcode
@}

@defgroup MESSAGE_USECASES_2 Adding a SMS Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Adding a SMS Message</h3>
- msg_message_t msg_new_message();
@n msg_new_message() is a function to create a message object which can store the message information. You should call this function to set the message that you want to add or send.
- int msg_set_message_type(msg_message_t msg, MSG_MESSAGE_TYPE_T msg_type);
@n msg_set_message_type() is a function to set the message type such as SMS or MMS. The first parameter is the message object which is created by msg_new_message(). The second parameter is the message type you want to set. It should be one of enum type ( _MSG_MESSAGE_TYPE_E). If setting is successful, the function returns MSG_SUCCESS. Otherwise it returns an error in enum type (_MSG_ERROR_E).
- int msg_sms_set_message_body(msg_message_t msg, const char* mdata, int size);
@n msg_sms_set_message_body() is a function to set the SMS body data. The first parameter is the message object which is created by msg_new_message(). The second parameter is the SMS body data you want to set. The third parameter is the length of SMS body data. If setting is successful, the function returns MSG_SUCCESS. Otherwise it returns an error in enum type (_MSG_ERROR_E).
- int msg_add_address(msg_message_t msg, const char* phone_num_list, MSG_RECIPIENT_TYPE_T to_type);
@n msg_add_address() is a function to add the phone numbers for the message. The first parameter is the message object which is created by msg_new_message(). The second parameter is the list of phone number you want to set. You can add several phone numbers at once. The phone numbers should be separated by ','. The third parameter is the recipient type of phone number. It should be one of enum type (_MSG_RECIPIENT_TYPE_E). If setting is successful, the function returns MSG_SUCCESS. Otherwise it returns an error in enum type (_MSG_ERROR_E).
- int msg_add_message(MSG_HANDLE_T handle, const msg_message_t msg, const MSG_SENDINGOPT_S *send_opt);
@n msg_add_message() is a function to add a composed message into the database of messaging framework. Before calling this function, the application should set the message object and the sending option structure and then pass them as parameters. If you don't want to change the current sending option, set the variable 'bSetting' to false. If saving a message is successful, the function returns MSG_SUCCESS. Otherwise it returns an error in enum type (_MSG_ERROR_E).
- int msg_release_message(msg_message_t *msg);
@n msg_release_message() is a function to free the memory of a message object which is create by msg_new_message(). If freeing the memory is successful, the function returns MSG_SUCCESS. Otherwise it returns an error in enum type (_MSG_ERROR_E).
@code
#include <string.h>
#include <MapiStorage.h>
#include <MapiMessage.h>

void test_add_sms_message(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_SENDINGOPT_S sendOpt = {0, };
	sendOpt.bSetting = false;

	msg_message_t msgInfo = msg_new_message();

	// Set Message Type
	err = msg_set_message_type(msgInfo, MSG_TYPE_SMS);

	if (err != MSG_SUCCESS) goto free_memory;

	char msgText[1024];

	memset(msgText, 0x00, 1024);
	strncpy(msgText, "Test SMS Message", sizeof(msgText)-1);
	int dataSize = strlen(msgText);

	// Set SMS text
	err = msg_sms_set_message_body(msgInfo, msgText, dataSize);

	if (err != MSG_SUCCESS) goto free_memory;

	char number[MAX_ADDRESS_VAL_LEN];

	memset(number, 0x00, MAX_ADDRESS_VAL_LEN);
	strncpy(number, "+821030011234", sizeof(number)-1);

	// Set Recipient Address
	err = msg_add_address(msgInfo, number, MSG_RECIPIENTS_TYPE_TO);

	if (err != MSG_SUCCESS) goto free_memory;

	err = msg_add_message(hMsgHandle, msgInfo, &sendOpt);

	if (err == MSG_SUCCESS)
		printf("Saving Message is OK!");
	else
		printf("Saving Message is failed!");

free_memory:
	msg_release_message(&msgInfo);
}
@endcode
@}

@defgroup MESSAGE_USECASES_3 Adding a MMS Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Adding a MMS Message</h3>
- int msg_set_subject(msg_message_t msg, const char* subject);
@n msg_set_subject() is a function to set the subject of MMS. The first parameter is the message object which is created by msg_new_message(). The second parameter is the subject you want to set. If setting is successful, the function returns MSG_SUCCESS. Otherwise it returns an error in enum type (_MSG_ERROR_E).

- MMS_MESSAGE_DATA_S* msg_mms_create_message(void);
@n msg_mms_create_message() allocates a MMS_MESSAGE_DATA_S structure and returns it’s address. MMS_MESSAGE_DATA_S is needed to represent mms specific data and transfer it to the messaging daemon in the pData parameter of the MSG_MESSAGE_S structure.

- MMS_SMIL_ROOTLAYOUT* msg_mms_set_rootlayout(MMS_MESSAGE_DATA_S* msg, const int width, const int height, const int bgcolor);
@n msg_mms_set_rootlayout() is a function to set smil root layout. The parameters representing the size and background color of smil root layout should be set.

- MMS_SMIL_REGION* msg_mms_add_region(MMS_MESSAGE_DATA_S *msg_data, const char* szID, const int x, const int y, const int width, const int height, const int bgcolor);
@n msg_mms_add_region() is a function to add a smil region. Smil region is needed to display text message, image, and video data (Each content has its own region). This function is called to allocate a region for each contents.

- MMS_PAGE_S* msg_mms_add_page(MMS_MESSAGE_DATA_S *msg_data, const int duration);
@n msg_mms_add_page() is a function to add a smil page.

- MMS_MEDIA_S* msg_mms_add_media(MMS_PAGE_S *page, const MmsSmilMediaType mediatype, const char* regionid, char* filepath);
@n msg_mms_add_media() is a function to add multimedia content to a mms message. If some content should be played with smil player, this function can be used.

- MMS_ATTACH_S* msg_mms_add_attachment(MMS_MESSAGE_DATA_S *msg_data, char *filepath);
@n msg_mms_add_attachment() is a function to add a content as an attached file. With this function a content might be attached as a multipart mixed type.

- int msg_mms_set_message_body(msg_message_t msg, const MMS_MESSAGE_DATA_S *msg_data);
@n msg_mms_set_message_body() is a function to set the MMS body data. The first parameter is the message object which is created by msg_new_message(). The second parameter is the structure which is allocated by msg_mms_create_message() and is set by the APIs for MMS.

- int msg_mms_destroy_message(MMS_MESSAGE_DATA_S* msg);
@n msg_mms_destroy_message() should be called to deallocate the MMS_MESSAGE_DATA_S structure and internal region, page, media, and attach list.
@code
#include <string.h>
#include <MapiStorage.h>
#include <MapiMessage.h>

void test_add_mms_message(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_SENDINGOPT_S sendOpt = {0, };
	sendOpt.bSetting = false;

	msg_message_t msgInfo = msg_new_message();

	// Set Message Type
	err = msg_set_message_type(msgInfo, MSG_TYPE_MMS);

	if (err != MSG_SUCCESS) goto free_memory;

	MMS_MESSAGE_DATA_S* data;
	MMS_PAGE_S* page;
	MMS_MEDIA_S* media[3];
	MMS_ATTACH_S* attach;
	int nSize = 0;

	// Set MMS subject
	char subject[MAX_SUBJECT_LEN];

	memset(subject, 0x00, MAX_SUBJECT_LEN);
	strncpy(subject, "hello mms", sizeof(subject)-1);

	err = msg_set_subject(msgInfo, subject);

	if (err != MSG_SUCCESS) goto free_memory;

	// Set MMS Body data
	data = msg_mms_create_message();
	msg_mms_set_rootlayout(data, 100, 100, 0xffffff);
	msg_mms_add_region(data, "Image", 0, 50, 100, 50, 0xffffff);
	msg_mms_add_region(data, "Text", 0, 0, 100, 50, 0xffffff);
	page = msg_mms_add_page(data, 5000);
	media[0] = msg_mms_add_media(page, MMS_SMIL_MEDIA_IMG, "Image", "/tmp/image.jpg");
	media[1] = msg_mms_add_media(page, MMS_SMIL_MEDIA_AUDIO, NULL, "/tmp/audio.amr");
	media[2] = msg_mms_add_media(page, MMS_SMIL_MEDIA_TEXT, "Text", "/tmp/message.txt");
	attach = msg_mms_add_attachment(data, "attachment.3gp");

	err = msg_mms_set_message_body(msgInfo, data);

	if (err != MSG_SUCCESS)
	{
		msg_mms_destroy_message(data);
		goto free_memory;
	}

	msg_mms_destroy_message(data);

	// Set Recipient Address
	char number[MAX_ADDRESS_VAL_LEN];

	memset(number, 0x00, MAX_ADDRESS_VAL_LEN);
	strncpy(number, "+821030011234", sizeof(number)-1);

	err = msg_add_address(msgInfo, number, MSG_RECIPIENTS_TYPE_TO);

	if (err != MSG_SUCCESS) goto free_memory;

	err = msg_add_message(hMsgHandle, msgInfo, &sendOpt);

	if (err == MSG_SUCCESS)
		printf("Saving Message is OK!");
	else
		printf("Saving Message is failed!");

free_memory:
	msg_release_message(&msgInfo);
}
@endcode
@}

@defgroup MESSAGE_USECASES_4 Sending a SMS/MMS Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Sending a SMS/MMS Message</h3>

- bool msg_is_sms(msg_message_t msg);
@n msg_is_sms() is a function to check whether the message type of message object is SMS or not. The first parameter is the message object which is created by msg_new_message(). The function returns true if the message object is a SMS message. Otherwise, it returns false.

- bool msg_is_mms(msg_message_t msg);
@n msg_is_mms() is a function to check whether the message type of message object is MMS or not. The first parameter is the message object which is created by msg_new_message(). The function returns true if the message object is a MMS message. Otherwise, it returns false.

- int msg_sms_send_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req);
@n msg_sms_send_message() is a function to send SMS through messaging framework. The first parameter is the handle which is created by msg_open_msg_handle (). The second parameter is the structure that includes the message information to send. You can get the result of sending on sent status callback.

- int msg_mms_send_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req);
@n msg_sms_send_message() is a function to send SMS through messaging framework. The first parameter is the handle which is created by msg_open_msg_handle (). The second parameter is the structure that includes the message information to send. You can get the result of sending on sent status callback.
@code
#include <MapiTransport.h>
#include <MapiMessage.h>

int MsgTestSendMsg(MSG_HANDLE_T hMsgHandle, msg_message_t pMsg)
{
	if (hMsgHandle == NULL)
	{
		printf("Handle is NULL\n");
		return MSG_ERR_NULL_MSGHANDLE;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	MSG_REQUEST_S req = {0};

	if (pMsg == NULL)
	{
		printf("Message is NULL\n");
		return MSG_ERR_NULL_MESSAGE;
	}

	req.msg = pMsg;

	if (msg_is_sms(req.msg))
		err = msg_sms_send_message(hMsgHandle, &req);
	else if (msg_is_mms(req.msg))
		err = msg_mms_send_message(hMsgHandle, &req);

	if (err == MSG_SUCCESS)
		printf("Request to Send Message is successful!!!");
	else
		printf("Request to Send Message is failed!!!");

	return err;
}
@endcode
@}

@defgroup MESSAGE_USECASES_5 Sending Simple SMS Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Simple SMS Sending</h3>
- int msg_sms_send(const char *phone_num_list, const char *sms_text, msg_simple_sent_status_cb cb, void *user_param);
@n msg_sms_send() is a simple function to send an SMS message. Without this API, in order to send a message the application should allocate a channel with messaging daemon for IPC, register sent-status callback function to monitor the sending result, and fulfill many member variables of MSG_MESSAGE_S. This function implicitly makes a connection with messaging daemon and registers the callback function. In addition, member variables of the MSG_MESSAGE_S structure are filled with default values except for the recipient phone_number and sms_text.
@code
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <MapiTransport.h>

GMainLoop *mainloop;

typedef struct {
	char number[256];
	char text[256];
	char userdata[256];
} send_data;

void sent_status_cb(MSG_SENT_STATUS_S *pMsgStatus, void *userData)
{
	if (pMsgStatus->status == MSG_NETWORK_SEND_SUCCESS)
		printf("reqId : %d  MSG SENT SUCCESS !!!\n", pMsgStatus->reqId);
	else
		printf("reqId : %d  MSG SENT FAIL !!! [%d]\n", pMsgStatus->reqId, pMsgStatus->status);
}

// count from here
gboolean send_func(gpointer data)
{
	send_data* sms_input = (send_data*)data;

	printf("Begin to send [%s] to [%s]\n", sms_input->number, sms_input->text);
	MSG_ERROR_T err = msg_sms_send(sms_input->number, sms_input->text, &sent_status_cb, (void*)sms_input->userdata);

	if (err != MSG_SUCCESS)
		printf("Send failed [%d]\n", err);

	return FALSE;
}
// end

int main(int argc, char* argv[])
{
	if (argc != 3 && argc != 4)
	{
		printf("Usage: %s  {phone_num_list} {sms_text} [user_data]\n", argv[0]);
		printf("phone_num_list: phone_num1, phone_num2, ..., phone_numN\n");
		return 0;
	}

	// Set sms input parameters : phone numbers and text
	send_data sms_input = {};
	strncpy(sms_input.number, argv[1], sizeof(sms_input.number)-1);
	strncpy(sms_input.text, argv[2], sizeof(sms_input.text)-1);
	if (argc == 4)
		strncpy(sms_input.userdata, argv[3], sizeof(sms_input.userdata)-1);

	// Add Sending Function to GMainLoop
	g_idle_add(&send_func, (gpointer) &sms_input);

	// start GMainLoop
	mainloop = g_main_loop_new(NULL, FALSE);

	printf("Entering GMain Loop to Receive Notifications in Thread...\n");

	g_main_loop_run(mainloop);

	printf("==== End Test App. Bye...===\n");

	return 0;
}
@endcode
@}

@defgroup MESSAGE_USECASES_6 Retrieving a MMS Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Retrieving a MMS Message</h3>
- int msg_mms_retrieve_message(MSG_HANDLE_T handle, MSG_REQUEST_S* req);
@n msg_mms_retrieve_message() is a function to submit a retrieve MMS request.
@code
void MsgTestRetrieveMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId)
{
	if (hMsgHandle == NULL)
	{
		printf("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	msg_message_t msg = msg_new_message();
	MSG_SENDINGOPT_S sendOpt = {0, };

	err = msg_get_message(hMsgHandle, (MSG_MESSAGE_ID_T)nMsgId, msg, &sendOpt);

	if (err != MSG_SUCCESS)
		printf("Get Message Failed!");

	MSG_REQUEST_S req = {0, msg, sendOpt};

	err = msg_mms_retrieve_message(hMsgHandle, &req);

	if (err != MSG_SUCCESS)
		printf("Retrieve MMS Message Failed!");

	msg_release_message(&msg);
}
@endcode
@}

@defgroup MESSAGE_USECASES_7 Getting a SMS Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Getting a SMS Message</h3>
- int msg_get_message(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id, msg_message_t msg, MSG_SENDINGOPT_S *send_opt);
@n msg_get_message() is a function to get a message. The first parameter is the handle which is created by msg_open_msg_handle (). The second parameter is the message ID you want to get. The third parameter is the message object to receive the message information. The last parameter is the structure to receive the message sending options.

- int msg_get_message_id(msg_message_t msg);
@n msg_get_message_id() is a function to get the message ID. The parameter is the message object. If the function is successful, it returns the message ID. Otherwise it returns an error in enum type (_MSG_ERROR_E).

- int msg_get_folder_id(msg_message_t msg);
@n msg_get_folder_id() is a function to get the ID of the folder  that the message is saved within. The parameter is the message object. If the function is successful, it returns one of the enum type in (_MSG_FOLDER_ID_E) . Otherwise it returns an error in enum type (_MSG_ERROR_E).

- int msg_get_message_type(msg_message_t msg);
@n msg_get_message_type() is a function to get the message type. The parameter is the message object. If the function is successful, it returns one of the enum type in (_MSG_MESSAGE_TYPE_E). Otherwise it returns an error in enum type (_MSG_ERROR_E).

- int msg_get_address_count(msg_message_t msg);
@n msg_get_address_count() is a function to get the number of addresses. The parameter is the message object. If the function is successful, it returns the number of addresses. Otherwise it returns an error in enum type (_MSG_ERROR_E).

- const char* msg_get_ith_address(msg_message_t msg, int ith);
@n msg_get_ith_address() is a function to get the ith address of message. The first parameter is the message object. The second parameter is the index of address you want to get. If the function is successful, it returns the address string. Otherwise it returns NULL.

- time_t* msg_get_time(msg_message_t msg);
@n msg_get_time() is a function to get the time value of message. The parameter is the message object. If the function is successful, it returns the time value. Otherwise it returns NULL.

- int msg_get_network_status(msg_message_t msg);
@n msg_get_network_status() is a function to get the network status of message. The parameter is the message object. If the function is successful, it returns one of the enum type in (_MSG_NETWORK_STATUS_E). Otherwise it returns an error in enum type (_MSG_ERROR_E).

- bool msg_is_read(msg_message_t msg);
@n msg_is_read() is a function to check whether the message was read or not. The parameter is the message object. If the message was read, it returns true. Otherwise it returns false.

- bool msg_is_protected(msg_message_t msg);
@n msg_is_protected() is a function to check whether the message is protected or not. The parameter is the message object. If the the message was protected, it returns true. Otherwise it returns false.

- int msg_get_message_body_size(msg_message_t msg);
@n msg_get_message_body_size() is a function to get the byte size of message. The parameter is the message object. If the function is successful, it returns the byte size of message. Otherwise it returns an error in enum type (_MSG_ERROR_E).

- const char* msg_sms_get_message_body(msg_message_t msg);
@n msg_sms_get_message_body() is a function to get the body data of message. The first parameter is the message object. If the function is successful, it returns the body data. Otherwise it returns NULL.
@code
void MsgTestGetSmsMessage(MSG_HANDLE_T hMsgHandle, int MsgId)
{
	if (hMsgHandle == NULL)
	{
		printf("Handle is NULL\n");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	msg_message_t msg = msg_new_message();
	MSG_SENDINGOPT_S sendOpt = {0, };

	// Get Message
	err = msg_get_message(hMsgHandle, (MSG_MESSAGE_ID_T)MsgId, msg, &sendOpt);

	if (err != MSG_SUCCESS) goto free_memory;

	printf("msgId = %d\n", msg_get_message_id(msg));
	printf("folderId = %d\n", msg_get_folder_id(msg));
	printf("msgType = %d\n", msg_get_message_type(msg));
	printf("phone number = %s\n", msg_get_ith_address(msg, 0));
	printf("displayTime = %s\n", ctime(msg_get_time(msg)));
	printf("networkStatus = %d\n", msg_get_network_status(msg));
	printf("bRead = %d\n", msg_is_read(msg));
	printf("bProtected = %d\n", msg_is_protected(msg));
	printf("dataSize = %d\n", msg_get_message_body_size(msg));
	printf("msgData = %s\n", msg_sms_get_message_body(msg));

free_memory:
	msg_release_message(&msg);
}
@endcode
@}

@defgroup MESSAGE_USECASES_8 Getting a MMS Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Getting a MMS Message</h3>
- int msg_mms_get_message_body(msg_message_t msg, MMS_MESSAGE_DATA_S *body);
@n msg_get_address_count. The parameter is the message object. The second parameter is the structure to receive MMS data as output. If the function is successful, it returns MSG_SUCCESS. Otherwise it returns an error in enum type (_MSG_ERROR_E).
- MMS_SMIL_REGION* msg_mms_get_smil_region(MMS_MESSAGE_DATA_S msgBody, int region_idx);
@n msg_mms_get_smil_region() is a function to get a SMIL region information. The first parameter is the structure of MMS data. The second parameter is the index of SMIL region you want to get. If the function is successful, it returns the structure which contains the SMIL region information. Otherwise it returns NULL.
- MMS_PAGE_S* msg_mms_get_page(MMS_MESSAGE_DATA_S msgBody, int page_idx);
@n msg_mms_get_page() is a function to get a SMIL page information. The first parameter is the structure of MMS data. The second parameter is the index of SMIL page you want to get. If the function is successful, it returns the structure which contains the SMIL page information. Otherwise it returns NULL.
- MMS_MEDIA_S* msg_mms_get_media(MMS_PAGE_S *page, int media_idx);
@n msg_mms_get_media() is a function to get a media information in a SMIL page. The first parameter is the structure of SMIL page you want to get the media from. The second parameter is the index of media you want to get. If the function is successful, it returns the structure which contains the media information. Otherwise it returns NULL.
- MMS_ATTACH_S* msg_mms_get_attachment(MMS_MESSAGE_DATA_S msgBody, int attach_idx);
@n msg_mms_get_attachment() is a function to get the file information of  an attachment. The first parameter is the structure of MMS data. The second parameter is the index of attachment you want to get. If the function is successful, it returns the structure which contains the attachment file information. Otherwise it returns NULL.
- const char* msg_get_subject(msg_message_t msg);
@n msg_get_subject() is a function to get the subject of MMS. The parameter is the message object. If the function is successful, it returns the subject string. Otherwise it returns NULL.
@code
void MsgTestGetMmsMessage(MSG_HANDLE_T hMsgHandle, int MsgId)
{
	if (hMsgHandle == NULL)
	{
		printf("Handle is NULL\n");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;

	msg_message_t msg = msg_new_message();
	MSG_SENDINGOPT_S sendOpt = {0, };

	// Get Message
	err = msg_get_message(hMsgHandle, (MSG_MESSAGE_ID_T)MsgId, msg, &sendOpt);

	if (err != MSG_SUCCESS) goto free_memory;

	if (msg_is_mms(msg) == false)
	{
		printf("It is not MMS Message!");
		goto free_memory;
	}

	MMS_MESSAGE_DATA_S* msgBody = msg_mms_create_message();

	// Get MMS Body Data
	msg_mms_get_message_body(msg, msgBody);

	//Print root-layout info
	printf("width: %d \n
		height: %d \n
		nbgColor:%x \n",
		msgBody->rootlayout.width.value,
		msgBody->rootlayout.height.value,
		msgBody->rootlayout.bgColor);

	// Print Region Info
	for (int i = 0; i < msgBody->regionCnt; ++i)
	{
		MMS_SMIL_REGION* pRegion = msg_mms_get_smil_region(msgBody, i);

		printf("region id: %s\n
			region left : %d\n
			region top : %d\n
			region width : %d\n
			region height : %d\n
			region bgColor : %x\n
			region fit : %d\n",
			pRegion->szID,pRegion->nLeft.value,
			pRegion->nTop.value,pRegion->width.value,
			pRegion->height.value, pRegion->bgColor,
			pRegion->fit);
	}

	// Print Page info
	for (int i = 0; i< msgBody->pageCnt; ++i)
	{
		MMS_PAGE_S* pPage = msg_mms_get_page(msgBody, i);

		printf("page's duration: %d msec\n
			page's media count: %d\n",
			pPage->nDur, pPage->mediaCnt);

		// Print Contents Info
		for(int j = 0; j < pPage->mediaCnt; ++j)
		{
			MMS_MEDIA_S* pMedia = msg_mms_get_media(pPage, j);
			printf("media's filename: %s\n
				media's filepath: %s\n
				media's regionId: %s\n
				Bold: %d Italic: %d\n",
				pMedia->szFileName,
				pMedia->szFilePath,
				pMedia->regionId,
				pMedia->sMedia.sText.bBold,
				pMedia->sMedia.sText.bItalic);

			if(pMedia->drmType != MSG_DRM_TYPE_NONE)
			{
				printf("media's drmtype: %d\n
					media's drmpath: %s\n",
					pMedia->drmType, pMedia->szDrm2FullPath);
			}
		}
	}

	for (int i = 0; i < msgBody->attachCnt; ++i)
	{
		MMS_ATTACH_S* pAttach = msg_mms_get_attachment(msgBody, i);

		printf("Attachment file Name: %s\n
			Attachment file Path: %s\n
			Attached file size: %d\n",
			pAttach->szFileName,
			pAttach->szFilePath,
			pAttach->fileSize);

		if(pAttach->drmType != MSG_DRM_TYPE_NONE)
			printf("media's drmtype: %d\n
				media's drmpath: %s\n",
				pAttach->drmType, pAttach->szDrm2FullPath);
	}

	printf("Subject: %s\n", msg_get_subject(pMsg));

	printf("msgId = %d", msg_get_message_id(msg));
	printf("folderId = %d", msg_get_folder_id(msg));
	printf("phone number = %s", msg_get_ith_address(msg, 0));
	printf("displayTime = %s", ctime(msg_get_time(msg)));
	printf("networkStatus = %d", msg_get_network_status(msg));
	printf("bRead = %d", msg_is_read(msg));
	printf("bProtected = %d", msg_is_protected(msg));

free_memory:
	msg_mms_destroy_message(msgBody);

	msg_release_message(&msg);
}
@endcode
@}

@defgroup MESSAGE_USECASES_9 Delete a Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Delete a Message</h3>
- int msg_delete_message(MSG_HANDLE_T handle, MSG_MESSAGE_ID_T msg_id);
@n msg_delete_message() is a function to delete a message from message box by msg_id.
@code
void MsgTestDeleteMessage(MSG_HANDLE_T hMsgHandle, MSG_MESSAGE_ID_T nMsgId)
{
	MSG_ERROR_T err;

	if (msg_delete_message(hMsgHandle, nMsgId) != MSG_SUCCESS)
		printf("Failed to delete Message");

}
@endcode
@}

@defgroup MESSAGE_USECASES_10 Set/Get Message Setting
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Set / Get Message setting</h3>
msg_set_config() and msg_get_cofig() is used to set or to get the message setting options. MSG_SETTING_S structure includes various setting information and is shared between message application and messaging daemon.

- int msg_set_config(MSG_HANDLE_T handle, const MSG_SETTING_S *setting);
@n msg_set_config() sets a message option.
- int msg_get_config(MSG_HANDLE_T handle, MSG_SETTING_S *setting);
@n msg_get_config() gets a message option.
@code
void MsgTestSetGeneralOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_SETTING_S setting = {0, };

	setting.type = MSG_GENERAL_OPT;

	setting.option.generalOpt.bKeepCopy = true;
	setting.option.generalOpt.alertTone = MSG_ALERT_TONE_ONCE;

	err = msg_set_config(hMsgHandle, &setting);

	if (err == MSG_SUCCESS)
		printf("Setting Config Data is OK!");
	else
		printf("Setting Config Data is failed!");
}

void MsgTestGetGeneralOpt(MSG_HANDLE_T hMsgHandle)
{
	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_SETTING_S setting = {0, };

	err = msg_get_config(hMsgHandle, &setting);

	if (err == MSG_SUCCESS)
	{
		printf("Setting Config Data is OK!\n");
		printf("bKeepCopy : [%d], AlertTone : [%d]\n", setting.option.generalOpt.bKeepCopy, setting.option.generalOpt.alertTone);
	}
	else
		printf("Setting Config Data is failed!\n");
}
@endcode
@}

@defgroup MESSAGE_USECASES_11 Filtering a Message
@ingroup MESSAGE_USECASES
@{
		<h3 class="pg">Filtering a Message</h3>
Messaging framework provides a filtering function for incoming message. New incoming message can be blocked (Move to spam-box or discard) by adding a filtering rule. A incoming message can be blocked by its originating address or by subject matching. If address matching filter rule is applied, only messages with exactly the same phone number can be blocked. Whereas, if a subject matching filter rule is applied, all messages that include the registered subject string might be blocked. An application can add or remove a filtering rule by calling msg_add_filter() or msg_delete_filter().

- int msg_add_filter(MSG_HANDLE_T handle, const MSG_FILTER_S *filter);
@n msg_add_filter() inserts a filtering rule to filter list and returns filter id.
- int msg_delete_filter(MSG_HANDLE_T handle, MSG_FILTER_ID_T filter_id);
@n msg_delete_filter()removes a filtering rule from filter list by filter id
@code
void MsgTestAddFilter(MSG_HANDLE_T hMsgHandle)
{
	if (hMsgHandle == NULL)
	{
		printf("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;
	MSG_FILTER_S filter[2]= {0, };

	// Add filter by address
	filter[0].filterType = MSG_FILTER_BY_ADDRESS;
	strncpy(filter[0].filterValue, "+821234567890", MAX_FILTER_VALUE_LEN);

	err = msg_add_filter(hMsgHandle, &filter[0]);

	if (err == MSG_SUCCESS)
		printf("msg_add_filter success");
	else
		printf("msg_add_filter fail - err [%d]", err);

	// Add filter by subject
	filter[1].filterType = MSG_FILTER_BY_SUBJECT;
	strncpy(filter[1].filterValue, "test filter", MAX_FILTER_VALUE_LEN);

	err = msg_add_filter(hMsgHandle, &filter[1]);

	if (err == MSG_SUCCESS)
		printf("msg_add_filter success");
	else
		printf("msg_add_filter fail - err [%d]", err);

	return;
}

void MsgTestDeleteFilter(MSG_HANDLE_T hMsgHandle, MSG_FILTER_ID_T filterId)
{
	if (hMsgHandle == NULL)
	{
		printf("Handle is NULL");
		return;
	}

	MSG_ERROR_T err = MSG_SUCCESS;
	err = msg_delete_filter(hMsgHandle, filterId);

	if (MSG_SUCCESS == err)
	{
		printf("msg_delete_filter success");
	}
	else
	{
		printf("msg_delete_filter fail - err [%d]", err);
	}

	return;
}
@endcode
@}
*/

/**
@addtogroup MESSAGE
@{
	@defgroup MESSAGE_USECASES Use Cases
@}
*/
