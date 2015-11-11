/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd. All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#include "MsgJsonParser.h"
#include "MsgSerialize.h"
#include "MsgMmsTypes.h"
#include "MsgDebug.h"
#include "MsgMmsMessage.h"


#ifdef MSG_MMS_USE_JSON_DATA
#include <glib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>

typedef enum {
	TYPE_STR,
	TYPE_INT,
	TYPE_ARRAY,
	TYPE_OBJECT,
} mms_json_data_type_e;

typedef struct _mms_json_item {
	const char *key;
	int e_val;
	int type;
} mms_json_item_s;

typedef enum {
	MMS_UNKNOWN = -1,
	MMS_HEADER = 0,
	MMS_MULTIPART_LIST,
	MMS_SMIL_MULTIPART,

	MULTIPART_TYPE ,
	MULTIPART_CONTENT_TYPE,
	MULTIPART_NAME,
	MULTIPART_FILEPATH,
	MULTIPART_FILEDATA,
	MULTIPART_CONTENT_ID,
	MULTIPART_CONTENT_LOCATION,

	HEADER_CONTENT_LOCATION,
	HEADER_CONTENT_TYPE,
	HEADER_CONTENT_TYPE_INT,
	HEADER_DATE,
	HEADER_DELIVERY_REPORT,
	HEADER_DELIVERY_TIME,
	HEADER_EXPIRY_TIME,
	HEADER_MESSAGE_CLASS,
	HEADER_MID,
	HEADER_MESSAGE_TYPE,
	HEADER_VERSION,
	HEADER_PRIORITY,
	HEADER_READ_REPORT,
	HEADER_HIDE_ADDRESS,
	HEADER_TRID,
	HEADER_CONTENT_CLASS,

	MMS_BACKUP_TYPE,
} mms_json_enum_e;

static mms_json_item_s mms_json_table[] = {
	{"header",		MMS_HEADER,	TYPE_INT},
	{"multipart_list",	MMS_MULTIPART_LIST, TYPE_ARRAY},
	{"smil",		MMS_SMIL_MULTIPART,	TYPE_OBJECT},
//multipart
	{"mp_type",	 MULTIPART_TYPE,	TYPE_INT},
	{"mp_ct",	 MULTIPART_CONTENT_TYPE,	TYPE_STR},
	{"mp_name",	 MULTIPART_NAME,	TYPE_STR},
	{"mp_path",	 MULTIPART_FILEPATH,	TYPE_STR},
	{"mp_data",	 MULTIPART_FILEDATA,	TYPE_STR},

	{"mp_cid",	 MULTIPART_CONTENT_ID,	TYPE_STR},
	{"mp_cl",	 MULTIPART_CONTENT_LOCATION,	TYPE_STR},
//header
	{"h_cl",	HEADER_CONTENT_LOCATION,	TYPE_STR},
	{"h_ct",	HEADER_CONTENT_TYPE,	TYPE_STR},
	{"h_ct_int",	HEADER_CONTENT_TYPE_INT,	TYPE_INT},
	{"h_date",	HEADER_DATE, TYPE_INT},
	{"h_d_rpt",	HEADER_DELIVERY_REPORT, TYPE_INT},
	{"h_d_time",	HEADER_DELIVERY_TIME, TYPE_INT},
	{"h_exp",	HEADER_EXPIRY_TIME, TYPE_INT},
	{"h_mclass",	HEADER_MESSAGE_CLASS,	TYPE_INT},
	{"h_mid",	HEADER_MID, TYPE_STR},
	{"h_mtype",	HEADER_MESSAGE_TYPE,	TYPE_INT},
	{"h_v",		HEADER_VERSION,	TYPE_INT},
	{"h_prioriy",	HEADER_PRIORITY,	TYPE_INT},
	{"h_r_rpt",	HEADER_READ_REPORT, TYPE_INT},
	{"h_hide_addr",	HEADER_HIDE_ADDRESS, TYPE_INT},
	{"h_tid",	HEADER_TRID,	TYPE_STR},
	{"h_cclass",	HEADER_CONTENT_CLASS, TYPE_INT},
	{"backup_type",	MMS_BACKUP_TYPE, TYPE_INT},

};

mms_json_enum_e get_mms_key_type(const char * key)
{
	int i;

	int table_count = sizeof(mms_json_table)/sizeof(mms_json_item_s);
	if (key) {
		for (i = 0; i < table_count; i++) {
			if (g_strcmp0(mms_json_table[i].key, key) == 0)
				return (mms_json_enum_e)mms_json_table[i].e_val;
		}
	}

	return MMS_UNKNOWN;
}

const char *get_mms_key(mms_json_enum_e e)
{
	int i;

	int table_count = sizeof(mms_json_table)/sizeof(mms_json_item_s);

	for (i = 0; i < table_count; i++) {
		if (mms_json_table[i].e_val == e)
			return mms_json_table[i].key;
	}

	return NULL;
}

int MsgSerializeHeader(const MMS_HEADER_DATA_S *pheader, JsonObject **headerObject)
{
	JsonObject *header_object = json_object_new();

	MSG_JSON_OBJ_SET_STR(header_object, get_mms_key(HEADER_CONTENT_LOCATION), pheader->contentLocation);

	MSG_JSON_OBJ_SET_STR(header_object, get_mms_key(HEADER_CONTENT_TYPE), pheader->szContentType);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_CONTENT_TYPE_INT), pheader->contentType);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_DATE), pheader->date);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_DELIVERY_REPORT), pheader->bDeliveryReport);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_DELIVERY_TIME), pheader->delivery.time);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_EXPIRY_TIME), pheader->expiry.time);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_MESSAGE_CLASS), pheader->messageClass);

	MSG_JSON_OBJ_SET_STR(header_object, get_mms_key(HEADER_MID), pheader->messageID);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_MESSAGE_TYPE), pheader->messageType);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_VERSION), pheader->mmsVersion);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_PRIORITY), pheader->mmsPriority);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_READ_REPORT), pheader->bReadReport);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_HIDE_ADDRESS), pheader->bHideAddress);

	MSG_JSON_OBJ_SET_STR(header_object, get_mms_key(HEADER_TRID), pheader->trID);

	MSG_JSON_OBJ_SET_INT(header_object, get_mms_key(HEADER_CONTENT_CLASS), pheader->contentClass);

	*headerObject = header_object;

	return 0;
}

int MsgParseHeader(msg_json_parser_object *parse_obj, MMS_HEADER_DATA_S *pheader)
{
	MSG_BEGIN();

	int index_child = 0;
	mms_json_enum_e type;
	msg_json_parser_object child = {};

	if (parse_obj == NULL)
		return -1;

	while(msg_json_parser_get_next_child(parse_obj, &child, index_child))
	{
		MSG_PRINT_PARSER_OBJECT(index_child, child);
		type = get_mms_key_type(child.key);

		switch (type) {
		case HEADER_CONTENT_LOCATION:
			snprintf(pheader->contentLocation, sizeof(pheader->contentLocation), "%s", (char *)child.value);
			break;
		case HEADER_CONTENT_TYPE:
			snprintf(pheader->szContentType, sizeof(pheader->szContentType), "%s", (char *)child.value);
			break;
		case HEADER_CONTENT_TYPE_INT:
			pheader->contentType = (int)child.number_value;
			break;
		case HEADER_DATE:
			pheader->date = (unsigned long int)child.number_value;
			break;
		case HEADER_DELIVERY_REPORT:
			pheader->bDeliveryReport = (bool)child.number_value;
			break;
		case HEADER_DELIVERY_TIME:
			if ((unsigned int)child.number_value > 0) {
				pheader->delivery.type = MMS_TIMETYPE_RELATIVE;
				pheader->delivery.time = (unsigned int)child.number_value;
			}
			break;
		case HEADER_EXPIRY_TIME:
			if ((unsigned int)child.number_value > 0) {
			pheader->expiry.type = MMS_TIMETYPE_RELATIVE;
			pheader->expiry.time = (unsigned int)child.number_value;
			}
			break;
		case HEADER_MESSAGE_CLASS:
			pheader->messageClass = (int)child.number_value;
			break;
		case HEADER_MID:
			snprintf(pheader->messageID, sizeof(pheader->messageID), "%s", (char *)child.value);
			break;
		case HEADER_MESSAGE_TYPE:
			pheader->messageType = (int)child.number_value;
			break;
		case HEADER_VERSION:
			pheader->mmsVersion = (int)child.number_value;
			break;
		case HEADER_PRIORITY:
			pheader->mmsPriority = (int)child.number_value;
			break;
		case HEADER_READ_REPORT:
			pheader->bReadReport = (bool)child.number_value;
			break;
		case HEADER_HIDE_ADDRESS:
			pheader->bHideAddress = (bool)child.number_value;
			break;
		case HEADER_TRID:
			snprintf(pheader->trID, sizeof(pheader->trID), "%s", (char *)child.value);
			break;
		case HEADER_CONTENT_CLASS:
			pheader->contentClass = (int)child.number_value;
			break;
		default:
			MSG_DEBUG("Not Support key = [%s], type = [%d]", child.key, type);
			break;
		}

		index_child++;
	}

	MSG_END();
	return 0;
}

int MsgSerializeMultipart(const MMS_MULTIPART_DATA_S *pMultipart, JsonObject **multipartObject)
{
	JsonObject *object = json_object_new();

	MSG_JSON_OBJ_SET_INT(object, get_mms_key(MULTIPART_TYPE), pMultipart->type);

	MSG_JSON_OBJ_SET_STR(object, get_mms_key(MULTIPART_CONTENT_TYPE), pMultipart->szContentType);
	MSG_JSON_OBJ_SET_STR(object, get_mms_key(MULTIPART_NAME), pMultipart->szFileName);
	MSG_JSON_OBJ_SET_STR(object, get_mms_key(MULTIPART_FILEPATH), pMultipart->szFilePath);

	if (pMultipart->pMultipartData) {
		char* base64data = g_base64_encode((const guchar*)pMultipart->pMultipartData, pMultipart->nMultipartDataLen);
		MSG_JSON_OBJ_SET_STR(object, get_mms_key(MULTIPART_FILEDATA), base64data);
		g_free(base64data);
	}

	MSG_JSON_OBJ_SET_STR(object, get_mms_key(MULTIPART_CONTENT_ID), pMultipart->szContentID);
	MSG_JSON_OBJ_SET_STR(object, get_mms_key(MULTIPART_CONTENT_LOCATION), pMultipart->szContentLocation);

	*multipartObject = object;

	return 0;
}

int MsgParseMultipartData(msg_json_parser_object *parse_obj, MMS_MULTIPART_DATA_S *pMultipart)
{
	MSG_BEGIN();

	int index_child = 0;
	mms_json_enum_e type;
	msg_json_parser_object child = {};

	if (parse_obj == NULL)
		return -1;

	while(msg_json_parser_get_next_child(parse_obj, &child, index_child))
	{
		MSG_PRINT_PARSER_OBJECT(index_child, child);

		type = get_mms_key_type(child.key);

		switch (type) {
		case MULTIPART_TYPE:
			pMultipart->type = (MimeType)child.number_value;
			break;
		case MULTIPART_CONTENT_TYPE:
			snprintf(pMultipart->szContentType, sizeof(pMultipart->szContentType), "%s", (char *)child.value);
			break;
		case MULTIPART_NAME:
			snprintf(pMultipart->szFileName, sizeof(pMultipart->szFileName), "%s", (char *)child.value);
			break;
		case MULTIPART_FILEPATH:
			snprintf(pMultipart->szFilePath, sizeof(pMultipart->szFilePath), "%s", (char *)child.value);
			break;
		case MULTIPART_FILEDATA:
			pMultipart->pMultipartData = (char*)g_base64_decode((char *)child.value, &pMultipart->nMultipartDataLen);
			break;
		case MULTIPART_CONTENT_ID:
			snprintf(pMultipart->szContentID, sizeof(pMultipart->szContentID), "%s", (char *)child.value);
			break;
		case MULTIPART_CONTENT_LOCATION:
			snprintf(pMultipart->szContentLocation, sizeof(pMultipart->szContentLocation), "%s", (char *)child.value);
			break;
		default:
			MSG_DEBUG("Not Support key = [%s], type = [%d]", child.key, type);
			break;
		}

		index_child++;
	}

	MSG_END();
	return 0;
}

int MsgParseMultipartListData(msg_json_parser_object *parse_obj, MMS_DATA_S *pMsgData)
{
	MSG_BEGIN();

	int index_child = 0;

	msg_json_parser_object child = {};

	if (parse_obj == NULL)
		return -1;

	while(msg_json_parser_get_next_child(parse_obj, &child, index_child))
	{
		if (child.value != NULL && child.type != MSG_JSON_PARSER_NULL) {

			MSG_PRINT_PARSER_OBJECT(index_child, child);

			MMS_MULTIPART_DATA_S *pMultipart= MsgMmsCreateMultipart();

			if (pMultipart) {
				if (MsgParseMultipartData(&child, pMultipart) == 0) {
					pMsgData->multipartlist = g_list_append(pMsgData->multipartlist, pMultipart);
				} else {
					free(pMultipart);
					pMultipart = NULL;
				}
			}
		} else {
			MSG_DEBUG("Get child : idx  = %d, key = %s, type = %d, value = %p", index_child, child.key, child.type, child.value);
		}

		index_child++;
	}

	MSG_END();
	return 0;
}

int MsgParseMmsData(msg_json_parser_object *parse_obj, MMS_DATA_S *pMsgData)
{
	MSG_BEGIN();

	int index_child = 0;
	mms_json_enum_e type;
	msg_json_parser_object child = {};

	if (parse_obj == NULL)
		return -1;

	while(msg_json_parser_get_next_child(parse_obj, &child, index_child))
	{
		if (child.key != NULL) {

			MSG_PRINT_PARSER_OBJECT(index_child, child);

			type = get_mms_key_type(child.key);

			switch (type) {
			case MMS_MULTIPART_LIST:
				MsgParseMultipartListData(&child, pMsgData);
				break;
			case MMS_HEADER:
				if (pMsgData->header == NULL) {
					pMsgData->header = MsgMmsCreateHeader();
				}
				if (pMsgData->header)
					MsgParseHeader(&child, pMsgData->header);
				break;
			case MMS_SMIL_MULTIPART:
			{
				MMS_MULTIPART_DATA_S *pMultipart = MsgMmsCreateMultipart();
				if (pMultipart) {
					if (MsgParseMultipartData(&child, pMultipart) == 0) {
						pMsgData->smil = pMultipart;
					} else {
						free(pMultipart);
						pMultipart = NULL;
					}
				}
				break;
			}
			case MMS_BACKUP_TYPE:
				pMsgData->backup_type = (int)child.number_value;
				break;
			default:
				MSG_DEBUG("Not Support key = [%s], type = [%d]", child.key, type);
				break;
			}

		} else {
			MSG_PRINT_PARSER_OBJECT(index_child, child);
		}

		index_child++;
	}

	MSG_END();
	return 0;
}


int MsgSerializeMmsJsonData(const MMS_DATA_S *pMsgData, char **pValue)
{
	MSG_BEGIN();

	if (pMsgData == NULL)
		return MSG_ERR_NULL_POINTER;

	JsonGenerator *generator;
	JsonNode *root;

	JsonObject *object_main, *object_header;
	JsonArray *array_multipart;

	gsize len = 0;

	generator = json_generator_new();

	root = json_node_new(JSON_NODE_OBJECT);

	object_main = json_object_new();

	MSG_JSON_OBJ_SET_INT(object_main, get_mms_key(MMS_BACKUP_TYPE), pMsgData->backup_type);

	//smil multipart
	if (pMsgData->smil) {
		JsonObject *smil_object = NULL;

		MsgSerializeMultipart(pMsgData->smil, &smil_object);
		MSG_JSON_OBJ_SET_OBJ(object_main, get_mms_key(MMS_SMIL_MULTIPART), smil_object);
	}

	if (pMsgData->multipartlist) {
		//multipart
		array_multipart = json_array_new();

		int list_count = g_list_length(pMsgData->multipartlist);

		MSG_DEBUG("Page Count is [%d]", list_count);

		for (int i = 0; i < list_count; i++) {

			MMS_MULTIPART_DATA_S *multipart = (MMS_MULTIPART_DATA_S *)g_list_nth_data(pMsgData->multipartlist, i);

			if (multipart) {
				JsonObject *multipart_object = NULL;

				MsgSerializeMultipart(multipart, &multipart_object);

				MSG_JSON_ARRAY_ADD_OBJECT(array_multipart, multipart_object);

			} else {
				MSG_DEBUG("Not Exist Multipart Data in [%d]th", i);
			}
		}

		MSG_JSON_OBJ_SET_ARRAY(object_main, get_mms_key(MMS_MULTIPART_LIST), array_multipart);
	}

	//header
	if (pMsgData->header) {
		MsgSerializeHeader(pMsgData->header, &object_header);

		MSG_JSON_OBJ_SET_OBJ(object_main, get_mms_key(MMS_HEADER), object_header);
	}

	json_node_take_object(root, object_main);

	json_generator_set_root(generator, root);

	*pValue = json_generator_to_data(generator, &len);
	MSG_BEGIN();
	MSG_DEBUG("Serialized Data : %s", *pValue);

	json_node_free(root);

	g_object_unref(generator);

	MSG_END();

	return 0;
}


int MsgDeserializeMmsJsonData(char* value, int value_len, MMS_DATA_S **ppMmsData)
{
	MSG_BEGIN();

	int ret = 0;

	msg_json_parser_handle parser_handle = NULL;
	msg_json_parser_object root = {};
	MMS_DATA_S *pMmsData = NULL;

	pMmsData = MsgMmsCreate();

	parser_handle = msg_json_parser_handle_create();

	msg_json_parser_parse_buffer(parser_handle, value, value_len, &root);

	MSG_DEBUG("Deserialized : %s", value);

	MSG_DEBUG("root : key = %s, type = %d, value = %p", root.key, root.type, root.value);

	ret = MsgParseMmsData(&root, pMmsData);
	if (ret != 0) {
		MSG_DEBUG("Fail to MsgParseMessageData, ret = %d", ret);
		MsgMmsRelease(&pMmsData);
	} else {
		*ppMmsData = pMmsData;
	}

	msg_json_parser_handle_destory(&parser_handle);

	MSG_END();

	return ret;
}
#endif


int MsgSerializeMms(const MMS_DATA_S *pMsgData, char **pValue)
{
	MSG_BEGIN();

	if (pMsgData == NULL)
		return MSG_ERR_NULL_POINTER;

	int bufsize = 0;

	int isExistHeader = 0, isExistSmil = 0, isExistMultipart = 0;
	int multipart_cnt = 0;
	char *buf = NULL;
	int i;

	bufsize += sizeof(int);	// back-up type

	int to_cnt = 0;
	int cc_cnt = 0;
	int bcc_cnt = 0;

	bufsize += sizeof(int); // check header data

	if (pMsgData->header) {
		isExistHeader = 1;

		bufsize += sizeof(MMS_HEADER_DATA_S); // header

		to_cnt = g_list_length(pMsgData->header->to);
		cc_cnt = g_list_length(pMsgData->header->cc);
		bcc_cnt = g_list_length(pMsgData->header->bcc);

		bufsize += (3 * sizeof(int));

		MSG_DEBUG("Address \"to\" count = [%d]", to_cnt);

		for (i = 0; i < to_cnt; i++) {
			MMS_ADDRESS_DATA_S *addr_data = (MMS_ADDRESS_DATA_S *)g_list_nth_data(pMsgData->header->to, i);
			if (addr_data && addr_data->address_val) {
				bufsize += (sizeof(int) + sizeof(int) + strlen(addr_data->address_val)); // type, length, address
			}
		}
		for (i = 0; i < cc_cnt; i++) {
			MMS_ADDRESS_DATA_S *addr_data = (MMS_ADDRESS_DATA_S *)g_list_nth_data(pMsgData->header->cc, i);
			if (addr_data && addr_data->address_val) {
				bufsize += (sizeof(int) + sizeof(int) + strlen(addr_data->address_val)); // type, length, address
			}
		}
		for (i = 0; i < bcc_cnt; i++) {
			MMS_ADDRESS_DATA_S *addr_data = (MMS_ADDRESS_DATA_S *)g_list_nth_data(pMsgData->header->bcc, i);
			if (addr_data && addr_data->address_val) {
				bufsize += (sizeof(int) + sizeof(int) + strlen(addr_data->address_val)); // type, length, address
			}
		}
	}

	bufsize += sizeof(int); // check smil data

	if (pMsgData->smil) {
		isExistSmil = 1;
		bufsize += (sizeof(MMS_MULTIPART_DATA_S) + (sizeof(char)*pMsgData->smil->nMultipartDataLen)); // smil data
	}

	bufsize += sizeof(int); // check multipart list data

	if (pMsgData->multipartlist) {
		isExistMultipart = 1;
		multipart_cnt = g_list_length(pMsgData->multipartlist);

		bufsize += sizeof(int); // multipart count

		for (i = 0; i < multipart_cnt; i++) {
			MMS_MULTIPART_DATA_S *multipart_data = (MMS_MULTIPART_DATA_S *)g_list_nth_data(pMsgData->multipartlist, i);
			bufsize += sizeof(MMS_MULTIPART_DATA_S) + (sizeof(char)*multipart_data->nMultipartDataLen);
		}
	}

	MSG_DEBUG("Serialize bufsize = %d", bufsize);

	buf = (char *)calloc(1, bufsize);
	if (buf == NULL)
		return -1;

	int serial_index = 0;
	int offset = 0;

	// 1. Backup type
	memcpy(buf, &pMsgData->backup_type, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] backup type = %d", serial_index++, offset, pMsgData->backup_type);
	offset += sizeof(int);

	// 2. Header Data
	memcpy(buf + offset, &isExistHeader, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->header) {
		memcpy(buf + offset, pMsgData->header, sizeof(MMS_HEADER_DATA_S));
		offset += sizeof(MMS_HEADER_DATA_S);

		// address
		memcpy(buf + offset, &to_cnt, sizeof(int));
		MSG_DEBUG("[#%2d][%5d] TO Count = %d", serial_index++, offset, to_cnt);
		offset += sizeof(int);

		for (i = 0; i < to_cnt; i++) {
			MMS_ADDRESS_DATA_S *to_addr = (MMS_ADDRESS_DATA_S *)g_list_nth_data(pMsgData->header->to, i);
			if (to_addr && to_addr->address_val) {
				memcpy(buf + offset, &to_addr->address_type, sizeof(int));
				MSG_DEBUG("[#%2d][%5d] address type = %d", serial_index++, offset, to_addr->address_type);
				offset += sizeof(int);
				memcpy(buf + offset, to_addr->address_val, sizeof(char)*MAX_ADDRESS_VAL_LEN);
				MSG_SEC_DEBUG("[#%2d][%5d] address val = %s", serial_index++, offset, to_addr->address_val);
				offset += sizeof(char)*MAX_ADDRESS_VAL_LEN;
			}
		}

		// address
		memcpy(buf + offset, &cc_cnt, sizeof(int));
		MSG_DEBUG("[#%2d][%5d] CC Count = %d", serial_index++, offset, cc_cnt);
		offset += sizeof(int);

		for (i = 0; i < cc_cnt; i++) {
			MMS_ADDRESS_DATA_S *cc_addr = (MMS_ADDRESS_DATA_S *)g_list_nth_data(pMsgData->header->cc, i);
			if (cc_addr && cc_addr->address_val) {
				memcpy(buf + offset, &cc_addr->address_type, sizeof(int));
				MSG_DEBUG("[#%2d][%5d] address type = %d", serial_index++, offset, cc_addr->address_type);
				offset += sizeof(int);
				memcpy(buf + offset, cc_addr->address_val, sizeof(char)*MAX_ADDRESS_VAL_LEN);
				MSG_SEC_DEBUG("[#%2d][%5d] address val = %s", serial_index++, offset, cc_addr->address_val);
				offset += sizeof(char)*MAX_ADDRESS_VAL_LEN;
			}
		}

		// address
		memcpy(buf + offset, &bcc_cnt, sizeof(int));
		MSG_DEBUG("[#%2d][%5d] BCC Count = %d", serial_index++, offset, bcc_cnt);
		offset += sizeof(int);

		for (i = 0; i < bcc_cnt; i++) {
			MMS_ADDRESS_DATA_S *bcc_addr = (MMS_ADDRESS_DATA_S *)g_list_nth_data(pMsgData->header->bcc, i);
			if (bcc_addr && bcc_addr->address_val) {
				memcpy(buf + offset, &bcc_addr->address_type, sizeof(int));
				MSG_DEBUG("[#%2d][%5d] address type = %d", serial_index++, offset, bcc_addr->address_type);
				offset += sizeof(int);
				memcpy(buf + offset, bcc_addr->address_val, sizeof(char)*MAX_ADDRESS_VAL_LEN);
				MSG_SEC_DEBUG("[#%2d][%5d] address val = %s", serial_index++, offset, bcc_addr->address_val);
				offset += sizeof(char)*MAX_ADDRESS_VAL_LEN;
			}
		}
	}

	// 3. Smil Data
	memcpy(buf + offset, &isExistSmil, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->smil) {
		memcpy(buf + offset, pMsgData->smil, sizeof(MMS_MULTIPART_DATA_S));
		offset += sizeof(MMS_MULTIPART_DATA_S);

		MSG_DEBUG("SMIL file path = [%s]", pMsgData->smil->szFilePath);
		MSG_DEBUG("SMIL nMultipartDataLen = [%d]", pMsgData->smil->nMultipartDataLen);

		if (pMsgData->smil->pMultipartData) {
			memcpy(buf + offset, pMsgData->smil->pMultipartData, sizeof(char)*pMsgData->smil->nMultipartDataLen);
			MSG_SEC_DEBUG("[#%2d][%5d] smil data = %s", serial_index++, offset, pMsgData->smil->pMultipartData);
			offset += sizeof(char)*pMsgData->smil->nMultipartDataLen;
		}
	}

	// 4. Multipart list data
	memcpy(buf + offset, &isExistMultipart, sizeof(int));
	offset += sizeof(int);

	if (pMsgData->multipartlist) {
		MSG_DEBUG("Multipart list count = [ %d]", multipart_cnt);
		memcpy(buf + offset, &multipart_cnt, sizeof(int));
		offset += sizeof(int);

		for (i = 0; i < multipart_cnt; i++) {
			MMS_MULTIPART_DATA_S *multipart_data = (MMS_MULTIPART_DATA_S *)g_list_nth_data(pMsgData->multipartlist, i);
			MSG_DEBUG("multipart_data = [%p]", multipart_data);
			if (multipart_data) {
				memcpy(buf + offset, multipart_data, sizeof(MMS_MULTIPART_DATA_S));
				offset += sizeof(MMS_MULTIPART_DATA_S);

				MSG_SEC_DEBUG("Multipart file path = [%s]", multipart_data->szFilePath);

				if (multipart_data->pMultipartData) {
					memcpy(buf + offset, multipart_data->pMultipartData, sizeof(char)*multipart_data->nMultipartDataLen);
					MSG_DEBUG("[#%2d][%5d] multipart data ptr = %p", serial_index++, offset, multipart_data->pMultipartData);
					offset += sizeof(char)*multipart_data->nMultipartDataLen;
				}
			}
		}
	}

	*pValue = buf;

	MSG_DEBUG("Expect Buffer Size: %d, Final offset : %d", bufsize, offset);

	MSG_END();

	return bufsize;
}


int MsgDeserializeMmsData(char* value, int value_len, MMS_DATA_S **ppMmsData)
{
	MSG_BEGIN();

	if (value == NULL) {
		MSG_DEBUG("Serialized data is NULL");
		return -1;
	}

	MMS_DATA_S *pMmsData = NULL;

	int isExistHeader = 0, isExistSmil = 0, isExistMultipart = 0;
	int addr_cnt = 0;
	int multipart_cnt = 0;

	int deserial_index = 0;
	int offset = 0;
	int i = 0;

	pMmsData = MsgMmsCreate();
	*ppMmsData = pMmsData;
	if (pMmsData == NULL)
		return -1;


	// 1. Backup type
	memcpy(&(pMmsData->backup_type), value, sizeof(int));
	MSG_DEBUG("[#%2d][%5d] backup type = %d", deserial_index++, offset, pMmsData->backup_type);
	offset += sizeof(int);

	// 2. Header Data
	memcpy(&isExistHeader, value + offset, sizeof(int));
	offset += sizeof(int);

	if (isExistHeader) {
		pMmsData->header = (MMS_HEADER_DATA_S *)calloc(1, sizeof(MMS_HEADER_DATA_S));
		if (pMmsData->header == NULL)
			return -1;

		memcpy(pMmsData->header, value + offset, sizeof(MMS_HEADER_DATA_S));
		offset += sizeof(MMS_HEADER_DATA_S);

		memcpy(&addr_cnt, value + offset, sizeof(int));
		offset += sizeof(int);

		pMmsData->header->to = NULL;

		for (i = 0; i < addr_cnt; i++) {
			MMS_ADDRESS_DATA_S* to_addr = (MMS_ADDRESS_DATA_S*)calloc(1, sizeof(MMS_ADDRESS_DATA_S));
			if (to_addr == NULL)
				return -1;

			memcpy(&(to_addr->address_type), value + offset, sizeof(int));
			MSG_DEBUG("[#%2d][%5d] address type = %d", deserial_index++, offset, to_addr->address_type);
			offset += sizeof(int);

			memcpy(to_addr->address_val, value + offset, sizeof(char)*MAX_ADDRESS_VAL_LEN);
			MSG_SEC_DEBUG("[#%2d][%5d] address val = %s", deserial_index++, offset, to_addr->address_val);
			offset += sizeof(char)*MAX_ADDRESS_VAL_LEN;

			pMmsData->header->to = g_list_append(pMmsData->header->to, (void *)to_addr);
		}

		memcpy(&addr_cnt, value + offset, sizeof(int));
		offset += sizeof(int);

		pMmsData->header->cc = NULL;

		for (i = 0; i < addr_cnt; i++) {
			MMS_ADDRESS_DATA_S* cc_addr = (MMS_ADDRESS_DATA_S*)calloc(1, sizeof(MMS_ADDRESS_DATA_S));
			if (cc_addr == NULL)
				return -1;

			memcpy(&(cc_addr->address_type), value + offset, sizeof(int));
			MSG_DEBUG("[#%2d][%5d] address type = %d", deserial_index++, offset, cc_addr->address_type);
			offset += sizeof(int);

			memcpy(cc_addr->address_val, value + offset, sizeof(char)*MAX_ADDRESS_VAL_LEN);
			MSG_SEC_DEBUG("[#%2d][%5d] address val = %s", deserial_index++, offset, cc_addr->address_val);
			offset += sizeof(char)*MAX_ADDRESS_VAL_LEN;

			pMmsData->header->cc = g_list_append(pMmsData->header->cc, (void *)cc_addr);
		}

		memcpy(&addr_cnt, value + offset, sizeof(int));
		offset += sizeof(int);

		pMmsData->header->bcc = NULL;

		for (i = 0; i < addr_cnt; i++) {
			MMS_ADDRESS_DATA_S* bcc_addr = (MMS_ADDRESS_DATA_S*)calloc(1, sizeof(MMS_ADDRESS_DATA_S));
			if (bcc_addr == NULL)
				return -1;

			memcpy(&(bcc_addr->address_type), value + offset, sizeof(int));
			MSG_DEBUG("[#%2d][%5d] address type = %d", deserial_index++, offset, bcc_addr->address_type);
			offset += sizeof(int);

			memcpy(bcc_addr->address_val, value + offset, sizeof(char)*MAX_ADDRESS_VAL_LEN);
			MSG_SEC_DEBUG("[#%2d][%5d] address val = %s", deserial_index++, offset, bcc_addr->address_val);
			offset += sizeof(char)*MAX_ADDRESS_VAL_LEN;

			pMmsData->header->bcc = g_list_append(pMmsData->header->bcc, (void *)bcc_addr);
		}
	}

	// 3. Smil Data
	memcpy(&isExistSmil, value + offset, sizeof(int));
	offset += sizeof(int);

	if (isExistSmil) {
		pMmsData->smil = (MMS_MULTIPART_DATA_S *)calloc(1, sizeof(MMS_MULTIPART_DATA_S));
		if (pMmsData->smil == NULL)
			return -1;

		memcpy(pMmsData->smil, value + offset, sizeof(MMS_MULTIPART_DATA_S));
		offset += sizeof(MMS_MULTIPART_DATA_S);

		MSG_SEC_DEBUG("SMIL file path = [%s]", pMmsData->smil->szFilePath);
		MSG_DEBUG("SMIL nMultipartDataLen = [%d]", pMmsData->smil->nMultipartDataLen);

		if (pMmsData->smil->nMultipartDataLen > 0) {
			pMmsData->smil->pMultipartData = (char *)calloc(1, sizeof(char)*pMmsData->smil->nMultipartDataLen);
			if (pMmsData->smil->pMultipartData == NULL)
				return -1;

			memcpy(pMmsData->smil->pMultipartData, value + offset, sizeof(char)*pMmsData->smil->nMultipartDataLen);
			MSG_DEBUG("[#%2d][%5d] smil data ptr = %p", deserial_index++, offset, pMmsData->smil->pMultipartData);
			offset += sizeof(char)*pMmsData->smil->nMultipartDataLen;
		} else {
			pMmsData->smil->pMultipartData = NULL;
		}
	}

	// 4. Multipart list data
	memcpy(&isExistMultipart, value + offset, sizeof(int));
	offset += sizeof(int);

	if (isExistMultipart) {
		memcpy(&multipart_cnt, value + offset, sizeof(int));
		offset += sizeof(int);

		MSG_DEBUG("Multipart list count = [ %d]", multipart_cnt);

		pMmsData->multipartlist = NULL;

		for (i = 0; i < multipart_cnt; i++) {
			MMS_MULTIPART_DATA_S *multipart_data = (MMS_MULTIPART_DATA_S *)calloc(1, sizeof(MMS_MULTIPART_DATA_S));
			if (multipart_data == NULL)
				return -1;

			memcpy(multipart_data, value + offset, sizeof(MMS_MULTIPART_DATA_S));
			offset += sizeof(MMS_MULTIPART_DATA_S);

			MSG_SEC_DEBUG("Multipart file path = [%s]", multipart_data->szFilePath);

			if (multipart_data->nMultipartDataLen > 0) {
				multipart_data->pMultipartData = (char *)calloc(1, sizeof(char)*multipart_data->nMultipartDataLen);
				if (multipart_data->pMultipartData == NULL) {
					free(multipart_data);
					return -1;
				}

				memcpy(multipart_data->pMultipartData, value + offset, sizeof(char)*multipart_data->nMultipartDataLen);
				MSG_DEBUG("[#%2d][%5d] multipart_data ptr = %p", deserial_index++, offset, multipart_data->pMultipartData);
				offset += sizeof(char)*multipart_data->nMultipartDataLen;
			} else {
				multipart_data->pMultipartData = NULL;
			}
			pMmsData->multipartlist = g_list_append(pMmsData->multipartlist, (void *)multipart_data);
		}
	}

	MSG_DEBUG("Final offset : %d", offset);

	MSG_END();
	return 0;
}
