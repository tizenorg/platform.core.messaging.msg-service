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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>
#include "MsgJsonParser.h"

static char root_key[5] = "root";

msg_json_gen_object* msg_json_gen_new_obj(msg_json_gen_type type)
{
	void *json_value = NULL;
	switch(type) {
	case MSG_JSON_GEN_OBJECT:
		json_value = (void *)json_object_new();
		break;
	case MSG_JSON_GEN_ARRAY:
		json_value = (void *)json_array_new();
		break;
	default:
		break;
	}
	if (json_value != NULL) {
		msg_json_gen_object *new_obj = NULL;
		new_obj = (msg_json_gen_object *)g_try_malloc0(sizeof(msg_json_gen_object));

		if (new_obj == NULL) {
			g_object_unref(json_value);
			return NULL;
		}

		new_obj->type = type;
		new_obj->value = json_value;
		return new_obj;
	}
	return NULL;
}

void msg_json_gen_free_obj(msg_json_gen_object *obj)
{
	if (obj == NULL) {
		return;
	}
	if (obj->value != NULL) {
		g_object_unref(obj->value);
		obj->value = NULL;
	}
	g_free(obj);
	obj = NULL;
}

void msg_json_gen_set_child(msg_json_gen_object *parent, const char *key, msg_json_gen_object *child) {
	if (parent == NULL || child == NULL)
	{
		return;
	}
	switch(parent->type) {
	case MSG_JSON_GEN_OBJECT:
		if (key == NULL) {
			return;
		}
		switch(child->type) {
		case MSG_JSON_GEN_OBJECT:
			json_object_set_object_member((JsonObject *)parent->value, key, (JsonObject *)child->value);
			break;
		case MSG_JSON_GEN_ARRAY:
			json_object_set_array_member((JsonObject *)parent->value, key, (JsonArray *)child->value);
			break;
		}
		break;
	case MSG_JSON_GEN_ARRAY:
		switch(child->type) {
		case MSG_JSON_GEN_OBJECT:
			json_array_add_object_element((JsonArray *)parent->value, (JsonObject *)child->value);
			break;
		case MSG_JSON_GEN_ARRAY:
			json_array_add_array_element((JsonArray *)parent->value, (JsonArray *)child->value);
			break;
		}
		break;
	}
}

void msg_json_gen_set_value(msg_json_gen_object *parent, const char *key, long long int value) {
	if (parent == NULL)
	{
		return;
	}
	switch(parent->type) {
	case MSG_JSON_GEN_OBJECT:
		if (key == NULL) {
			return;
		}
		json_object_set_int_member((JsonObject *)parent->value, key, value);
		break;
	case MSG_JSON_GEN_ARRAY:
		json_array_add_int_element((JsonArray *)parent->value, value);
		break;
	}
}

void msg_json_gen_set_value(msg_json_gen_object *parent, const char *key, const char *value) {
	if (parent == NULL)
	{
		return;
	}
	switch(parent->type) {
	case MSG_JSON_GEN_OBJECT:
		if (key == NULL) {
			return;
		}
		json_object_set_string_member((JsonObject *)parent->value, key, value);
		break;
	case MSG_JSON_GEN_ARRAY:
		json_array_add_string_element((JsonArray *)parent->value, value);
		break;
	}
}

char* msg_json_gen_make_json_msg(msg_json_gen_object *root_obj, unsigned long *len) {
	if (root_obj == NULL)
	{
		return NULL;
	}
	JsonGenerator *generator = json_generator_new();
	JsonNode *root = json_node_new(JSON_NODE_OBJECT);

	json_node_take_object(root, (JsonObject *)root_obj->value);
	json_generator_set_root(generator, root);

	char *json_msg = json_generator_to_data(generator, (gsize *)len);
	g_object_unref(generator);
	json_node_free(root);

	return json_msg;
}

int msg_json_parser_get_value(msg_json_parser_object *json_obj)
{
	GValue value = { 0 };

	/*Input params validation */
	if (json_obj == NULL) {
		MSG_DEBUG("Invalid Input Parameters");
		return -1;
	}

	json_obj->float_value = 0;
	json_obj->number_value = 0;

	json_node_get_value((JsonNode *) json_obj->value, &value);

	switch (json_node_get_value_type((JsonNode *) json_obj->value)) {
	case G_TYPE_STRING:
		{
			json_obj->type = MSG_JSON_PARSER_STRING;
			json_obj->value = (void *)g_value_get_string(&value);
		}
		break;
	case G_TYPE_INT:
		{
			json_obj->type = MSG_JSON_PARSER_INT;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_int(&value);
		}
		break;
	case G_TYPE_UINT:
		{
			json_obj->type = MSG_JSON_PARSER_UINT;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_uint(&value);
		}
		break;
	case G_TYPE_BOOLEAN:
		{
			json_obj->type = MSG_JSON_PARSER_BOOLEAN;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_boolean(&value);
		}
		break;
	case G_TYPE_INT64:
		{
			json_obj->type = MSG_JSON_PARSER_INT;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_int64(&value);
		}
		break;
	case G_TYPE_UINT64:
		{
			json_obj->type = MSG_JSON_PARSER_UINT;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_uint64(&value);
		}
		break;
	case G_TYPE_DOUBLE:
		{
			json_obj->type = MSG_JSON_PARSER_REAL;
			json_obj->value = NULL;
			json_obj->float_value = (double)g_value_get_double(&value);
		}
		break;
	default:
		{
			MSG_DEBUG(" Entering node default case");
		}
		break;
	}

	return 1;
}

int msg_json_parser_object_get_value(msg_json_parser_object *json_obj)
{
	GValue value = { 0 };
	JsonNode *node = NULL;

	/*Input params validation */
	if (json_obj == NULL) {
		MSG_DEBUG("Invalid Input Parameters");
		return -1;
	}
	node = json_object_get_member((JsonObject *)json_obj->value, json_obj->key);

	json_obj->float_value = 0;
	json_obj->number_value = 0;

	json_node_get_value((JsonNode *) node, &value);

	switch (json_node_get_value_type((JsonNode *) node)) {
	case G_TYPE_STRING:
		{
			json_obj->type = MSG_JSON_PARSER_STRING;
			json_obj->value = (void *)g_value_get_string(&value);
		}
		break;
	case G_TYPE_INT:
		{
			json_obj->type = MSG_JSON_PARSER_INT;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_int(&value);
		}
		break;
	case G_TYPE_UINT:
		{
			json_obj->type = MSG_JSON_PARSER_UINT;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_uint(&value);
		}
		break;
	case G_TYPE_BOOLEAN:
		{
			json_obj->type = MSG_JSON_PARSER_BOOLEAN;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_boolean(&value);
		}
		break;
	case G_TYPE_INT64:
		{
			json_obj->type = MSG_JSON_PARSER_INT;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_int64(&value);
		}
		break;
	case G_TYPE_UINT64:
		{
			json_obj->type = MSG_JSON_PARSER_UINT;
			json_obj->value = NULL;
			json_obj->number_value = g_value_get_uint64(&value);
		}
		break;
	case G_TYPE_DOUBLE:
		{
			json_obj->type = MSG_JSON_PARSER_REAL;
			json_obj->value = NULL;
			json_obj->float_value = (double)g_value_get_double(&value);
		}
		break;
	default:
		{
			MSG_DEBUG(" Entering node default case");
		}
		break;
	}

	return 1;
}

msg_json_parser_handle msg_json_parser_handle_create(void)
{
	JsonParser *jsonParser = NULL;

	jsonParser = json_parser_new();

	return(msg_json_parser_handle)jsonParser;
}

void msg_json_parser_handle_destory(msg_json_parser_handle *handle)
{
	if (handle == NULL)
		return;

	g_object_unref(*handle);
	*handle = NULL;
}

void msg_json_parser_parse_buffer(msg_json_parser_handle handle, const char* value, int value_len, msg_json_parser_object *json_obj)
{
	gboolean gRet = TRUE;
	JsonNode *root = NULL;
	GError *error = NULL;
	JsonParser *jsonParser = (JsonParser *)handle;
	JsonNodeType parentType = JSON_NODE_NULL;

	/*Input params validation*/
	if (value == NULL || value_len == 0) {
		MSG_DEBUG("Invalid Input Parameters");
		return ;
	}

	if (jsonParser != NULL) {

		/** Loads a JSON stream from a buffer and parses it */
		gRet = json_parser_load_from_data(jsonParser, value, value_len, &error);
		if (gRet != TRUE) {
			g_error_free(error);
		} else {
			/** Fetch the root node */
			root = json_parser_get_root(jsonParser);

			if (root != NULL) {

				json_obj->key = root_key;
				json_obj->value = root;
				parentType = json_node_get_node_type(root);

				if (parentType == JSON_NODE_VALUE) {
					gRet = msg_json_parser_get_value(json_obj);
				} else if (parentType == JSON_NODE_OBJECT) {
					json_obj->type = MSG_JSON_PARSER_OBJECT;
				} else if (parentType == JSON_NODE_ARRAY) {
					json_obj->type = MSG_JSON_PARSER_ARRAY;
				} else {
					json_obj->type = MSG_JSON_PARSER_NULL;
				}
			} else {
				json_obj->key = NULL;
				json_obj->value = NULL;
			}
		}
	} else {

	}

	return ;
}

int msg_json_parser_get_next_child(const msg_json_parser_object *parent, msg_json_parser_object *child, int index)
{
	int lReturn = 1;
	JsonNodeType jNodeParentType = JSON_NODE_NULL;
	JsonNodeType jNodeChildType = JSON_NODE_NULL;
	JsonArray *tempArray = NULL;
	JsonObject *tempObj = NULL;
	GList *members = NULL;

	/*Input params validation */
	if (parent == NULL || parent->value == NULL || child == NULL || index < 0) {
		MSG_DEBUG("Invalid Input Parameters");
		return 0;
	}

	/** Get the JSON Parent Node Type */
	jNodeParentType = json_node_get_node_type((JsonNode *) parent->value);

	switch (jNodeParentType) {
	case JSON_NODE_OBJECT:
		{
			/** Get the JSON object from JSON Parent Node */
			tempObj = json_node_get_object((JsonNode *) parent->value);

			/** Get the list of keys from the object node */
			members = json_object_get_members(tempObj);

			/** Get the key by index from the list */
			child->key = (char *)g_list_nth_data(members, index);

			g_list_free(members);
			if (child->key == NULL) {
				return 0;
			}

			/** Get the JSONNode by key from the list */
			child->value = json_object_get_member(tempObj, child->key);

			/** Identify the node type of the JSOSNNode */
			jNodeChildType = json_node_get_node_type((JsonNode *) child->value);

			switch (jNodeChildType) {
			case JSON_NODE_OBJECT:
				{
					child->type = MSG_JSON_PARSER_OBJECT;
				}
				break;
			case JSON_NODE_ARRAY:
				{
					child->type = MSG_JSON_PARSER_ARRAY;
				}
				break;
			case JSON_NODE_VALUE:
				{
					lReturn = msg_json_parser_get_value(child);
				}
				break;
			case JSON_NODE_NULL:
				{
				}
				break;
			default:
				{
					lReturn = 0;
				}
				break;
			}

		}
		break;
	case JSON_NODE_ARRAY:
		{
			/** Get the JSON array from the JSON node */
			tempArray = json_node_get_array((JsonNode *) parent->value);

			child->key = NULL;

			if ((guint) index >= json_array_get_length(tempArray)) {
				return 0;
			}

			/** Get the JSONNode from the list of values */
			child->value = (void *)json_array_get_element(tempArray, index);
			if (child->value == NULL) {
				return 0;
			}
			/* Get the child type */
			jNodeChildType = json_node_get_node_type((JsonNode *) child->value);

			switch (jNodeChildType) {
			case JSON_NODE_OBJECT:
				{
					child->type = MSG_JSON_PARSER_OBJECT;
				}
				break;
			case JSON_NODE_ARRAY:
				{
					child->type = MSG_JSON_PARSER_ARRAY;
				}
				break;
			case JSON_NODE_VALUE:
				{
					lReturn = msg_json_parser_get_value(child);
				}
				break;
			case JSON_NODE_NULL:
				{
				}
				break;
			default:
				{
					lReturn = 0;
					break;
				}
			}

		}
		break;
	case JSON_NODE_VALUE:
	default:
		{
			child->key = NULL;
			child->value = NULL;
			lReturn = 0;
		}
		break;
	}

	return lReturn;
}


/**
 * @fn msg_json_parser_get_child_by_name
 * This function is used for getting the child node by it's name for
	input node.
 * @param [IN] \n
 * parent: msg_json_parser_object structure pointer whose child node
	is to be retrieved \n
 * name: const char pointer containing key of the required child \n
 * @param [OUT] \n
 * child: msg_json_parser_object structure pointer. Should be allocated
	and freed after it's use by caller. It will be filled with
	corresponding data \n
 * @return	Return type is int. Should be typecasted to
	msg_json_parser_parse_status to check the return status. If
	MSG_JSON_PARSER_PARSE_SUCCESS, then only child contains
	valid data, else some error occured. \n
 * @remark This API is synchronous.
 */
int msg_json_parser_get_child_by_name(const msg_json_parser_object *parent,
					msg_json_parser_object *child,
					const char *name)
{
	int lReturn = -1;
	JsonObject *tempObj = NULL;
	JsonNodeType jNodeParentType = JSON_NODE_NULL;
	JsonNodeType jNodeChildType = JSON_NODE_NULL;

	/*Input params validation */
	if (parent == NULL || parent->value == NULL || child == NULL
	    || name == NULL) {
		MSG_DEBUG("Invalid Input Parameters");
		return -1;
	}

	/** Get the JSON Parent Node Type */
	jNodeParentType = json_node_get_node_type((JsonNode *) parent->value);

	switch (jNodeParentType) {
	case JSON_NODE_OBJECT:
		{
			/** Get the JSON object from JSON Parent Node */
			tempObj =
			    json_node_get_object((JsonNode *) parent->value);

			/** Get the list of keys from the object node */
			GList *members = json_object_get_members(tempObj);
			if (members == NULL) {
				return -1;
			}

			/** Get the key by index from the list */
			char *member = NULL;
			for (unsigned int i = 0; i < g_list_length(members); i++) {

				member = (char *)g_list_nth_data(members, i);

				if (g_strcmp0((char *)name, member) == 0) {
					child->key	= member;
				}
			}

			g_list_free(members);

			if (child->key == NULL) {
				return -1;
			}

			/** Get the JSONNode by key from the list */
			child->value = (JsonNode *) json_object_get_member(tempObj,	child->key);

			/** Identify the node type of the JSOSNNode */
			jNodeChildType = json_node_get_node_type((JsonNode *) child->value);

			switch (jNodeChildType) {
			case JSON_NODE_OBJECT:
				{
					child->type = MSG_JSON_PARSER_OBJECT;
				}
				break;
			case JSON_NODE_ARRAY:
				{
					child->type = MSG_JSON_PARSER_ARRAY;
				}
				break;
			case JSON_NODE_VALUE:
				{
					lReturn = msg_json_parser_get_value(child);
				}
				break;
			case JSON_NODE_NULL:
				{

				}
				break;
			default:
				{
					lReturn = -1;
				}
				break;
			}

		}
		break;
	case JSON_NODE_ARRAY:
	case JSON_NODE_VALUE:
	default:
		{
			child->key = NULL;
			child->value = NULL;
			lReturn = -1;
		}
		break;
	}

	return lReturn;
}


/**
 * @fn msg_json_parser_get_child_count
 * This function returns the count for a specified JSON object.
 * @param [IN] \n
 * object: msg_json_parser_object structure pointer whose child count
	is to be retrieved \n
 * @return	Return type is int. This contains the value of the child node
	for the specified JSON object \n
 * @remark This API is synchronous.
 */
int msg_json_parser_get_child_count(msg_json_parser_object *object)
{
	JsonNodeType jNodeParentType = JSON_NODE_NULL;
	JsonObject *tempObj = NULL;
	JsonArray *tempArray = NULL;
	int count = 0;

	/*Input params validation */
	if (object == NULL || object->value == NULL) {
		MSG_DEBUG("Invalid Input Parameters");
		return count;
	}

	/** Get the JSON Parent Node Type */
	jNodeParentType = json_node_get_node_type((JsonNode *) object->value);

	switch (jNodeParentType) {
	case JSON_NODE_OBJECT:
		{
			MSG_DEBUG("  JSON_NODE_OBJECT ");
			tempObj = NULL;
			/** Get the JSON object from JSON Parent Node */
			tempObj = json_node_get_object((JsonNode *) object->value);
			/** Get the number of members from the object node */
			count = json_object_get_size(tempObj);
		}
		break;
	case JSON_NODE_ARRAY:
		{
			MSG_DEBUG("  JSON_NODE_ARRAY ");
			tempArray = NULL;
			/** Get the JSON array from the JSON node */
			tempArray =
			    json_node_get_array((JsonNode *) object->value);
			/** Get the number of members from the array node */
			count = json_array_get_length(tempArray);
		}
		break;
	case JSON_NODE_VALUE:
	default:
		{

		}
		break;
	}

	MSG_DEBUG("COUNT :: %d ", count);
	return count;
}

