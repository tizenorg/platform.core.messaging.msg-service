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

#ifndef __MSG_JSON_PARSER_H__
#define __MSG_JSON_PARSER_H__

#include <stdio.h>
#include "MsgDebug.h"

typedef enum {
        MSG_JSON_GEN_ARRAY,
        MSG_JSON_GEN_OBJECT
} msg_json_gen_type;

typedef struct msg_json_gen_object_t {
	msg_json_gen_type type;
	void *value;
} msg_json_gen_object;

msg_json_gen_object* msg_json_gen_new_obj(msg_json_gen_type type);
void msg_json_gen_free_obj(msg_json_gen_object *obj);
void msg_json_gen_set_child(msg_json_gen_object *parent, const char *key, msg_json_gen_object *child);
void msg_json_gen_set_value(msg_json_gen_object *parent, const char *key, long long int value);
void msg_json_gen_set_value(msg_json_gen_object *parent, const char *key, const char *value);
char* msg_json_gen_make_json_msg(msg_json_gen_object *root_obj, unsigned long *len);

#define MSG_JSON_OBJ_SET_STR(json_obj, key, val)\
	if(json_obj && key && val){\
		json_object_set_string_member(json_obj, key, val);\
	} else {\
		MSG_DEBUG("Error : json_obj = %p, key = %s, val = %s", json_obj, key, val);\
	}

#define MSG_JSON_OBJ_SET_INT(json_obj, key, val)\
	if(json_obj && key){\
		json_object_set_int_member(json_obj, key, val);\
	} else {\
		MSG_DEBUG("Error : json_obj = %p, key = %s, val = %d", json_obj, key, val);\
	}

#define MSG_JSON_OBJ_SET_OBJ(json_obj, key, val)\
	if(json_obj && key){\
		json_object_set_object_member(json_obj, key, val);\
	} else {\
		MSG_DEBUG("Error : json_obj = %p, key = %s, val = %p", json_obj, key, val);\
	}

#define MSG_JSON_OBJ_SET_ARRAY(json_obj, key, json_array)\
	if(json_obj && key && json_array){\
		json_object_set_array_member(json_obj, key, json_array);\
	} else {\
		MSG_DEBUG("Error : json_obj = %p, key = %s, json_array = %p", json_obj, key, json_array);\
	}

#define MSG_JSON_ARRAY_ADD_OBJECT(json_array, json_obj)\
	if(json_array && json_obj){\
		json_array_add_object_element(json_array, json_obj);\
	} else {\
		MSG_DEBUG("Error : json_array = %p, json_obj = %p", json_array, json_obj);\
	}

#define MSG_PRINT_PARSER_OBJECT(index, obj) {\
	switch(obj.type) {\
	case MSG_JSON_PARSER_NULL:\
		MSG_DEBUG("parse obj : idx = %d, key =[%s], type = [%d], value = [%p], value num = [%f]", index, obj.key, obj.type, obj.value, obj.number_value);\
		break;\
	case MSG_JSON_PARSER_INT:\
		MSG_DEBUG("parse obj : idx = [%d], key = [%s], type = [%s], value = [%d]", index, obj.key, "MSG_JSON_PARSER_INT", (int)obj.number_value);\
		break;\
	case MSG_JSON_PARSER_UINT:\
		MSG_DEBUG("parse obj : idx = [%d], key = [%s], type = [%s], value = [%u]", index, obj.key, "MSG_JSON_PARSER_UINT", (unsigned int)obj.number_value);\
		break;\
	case MSG_JSON_PARSER_REAL:\
		MSG_DEBUG("parse obj : idx = [%d], key = [%s], type = [%s], value = [%f]", index, obj.key, "MSG_JSON_PARSER_REAL", obj.number_value);\
		break;\
	case MSG_JSON_PARSER_STRING:\
		MSG_DEBUG("parse obj : idx = [%d], key = [%s], type = [%s], value = [%s]", index, obj.key, "MSG_JSON_PARSER_STRING", (char *)obj.value);\
		break;\
	case MSG_JSON_PARSER_BOOLEAN:\
		MSG_DEBUG("parse obj : idx = [%d], key = [%s], type = [%s], value = [%s]", index, obj.key, "MSG_JSON_PARSER_BOOLEAN", obj.number_value == 0 ? "TRUE" : "FALSE");\
		break;\
	case MSG_JSON_PARSER_ARRAY:\
		MSG_DEBUG("parse obj : idx = [%d], key = [%s], type = [%s]", index, obj.key, "MSG_JSON_PARSER_ARRAY");\
		break;\
	case MSG_JSON_PARSER_OBJECT:\
		MSG_DEBUG("parse obj : idx = [%d], key = [%s], type = [%s]", index, obj.key, "MSG_JSON_PARSER_OBJECT");\
		break;\
	default:\
		MSG_DEBUG("parse obj : idx = %d, key =[%s], type = [%d], value = [%p], value num = [%f]", index, obj.key, obj.type, obj.value, obj.number_value);\
		break;\
	}\
}
/**
*@enum msg_json_parser_value_type
* This enumerates json-node types.
*/
typedef enum {
        MSG_JSON_PARSER_NULL = 0,      /** 0 <null value */
        MSG_JSON_PARSER_INT,           /** 1 < signed integer value */
        MSG_JSON_PARSER_UINT,          /** 2 < unsigned integer value */
        MSG_JSON_PARSER_REAL,          /** 3 < double value */
        MSG_JSON_PARSER_STRING,        /** 4 < UTF-8 string value */
        MSG_JSON_PARSER_BOOLEAN,       /** 5 < bool value */
        MSG_JSON_PARSER_ARRAY,         /** 6 < array value (ordered list) */
        MSG_JSON_PARSER_OBJECT,        /** 7 < object value */
} msg_json_parser_value_type;

typedef void *msg_json_parser_handle;

typedef struct msg_json_parser_object_t {
        msg_json_parser_value_type type;
        char *key;
        void *value;    /** Value of the node. VALID
                                        only if type is
                                        MSG_JSON_PARSER_NULL,
                                        MSG_JSON_PARSER_STRING,
                                        MSG_JSON_PARSER_ARRAY,
                                        MSG_JSON_PARSER_OBJECT */
        double float_value; /** Value of the node. VALID
                                        only if type is
                                        MSG_JSON_PARSER_REAL */
        long long int number_value; /** Value of the node. VALID
                                        only if type is
                                        MSG_JSON_PARSER_UINT,
                                        MSG_JSON_PARSER_INT,
                                        MSG_JSON_PARSER_BOOLEAN */
} msg_json_parser_object;

msg_json_parser_handle msg_json_parser_handle_create(void);
void msg_json_parser_handle_destory(msg_json_parser_handle *handle);
int msg_json_parser_get_value(msg_json_parser_object *json_obj);
int msg_json_parser_object_get_value(msg_json_parser_object *json_obj);
int msg_json_parser_get_next_child(const msg_json_parser_object *parent, msg_json_parser_object *child, int index);
void msg_json_parser_parse_buffer(msg_json_parser_handle handle, const char* value, int value_len, msg_json_parser_object *json_obj);
int msg_json_parser_get_child_by_name(const msg_json_parser_object *parent, msg_json_parser_object *child, const char *name);
int msg_json_parser_get_child_count(msg_json_parser_object *object);

#endif /*__MSG_JSON_PARSER_H__*/
