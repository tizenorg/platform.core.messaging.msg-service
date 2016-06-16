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

#ifndef MSG_H_
#define MSG_H_

/*==================================================================================================
											INCLUDE FILES
==================================================================================================*/
#include <stdbool.h>

#include "msg_types.h"

/*==================================================================================================
											DEFINES
==================================================================================================*/

/**
 *	@brief	Defines macro for privilege name http://tizen.org/privilege/message.read
 */
#define MSG_SERVICE_READ_PRIV_NAME		"http://tizen.org/privilege/message.read"

/**
 *	@brief	Defines macro for privilege name http://tizen.org/privilege/message.write
 */
#define MSG_SERVICE_WRITE_PRIV_NAME		"http://tizen.org/privilege/message.write"

#ifdef __cplusplus
extern "C"
{
#endif

/*==================================================================================================
											FUNCTION PROTOTYPES
==================================================================================================*/

/**
 * @ingroup MSG_SERVICE_FRAMEWORK
 * @defgroup MSG_SERVICE_FRAMEWORK_CONTROL_MODULE Control API
 * @brief The Control API provides functions to manage message handle and set or get each structure.
 *
 * @addtogroup MSG_SERVICE_FRAMEWORK_CONTROL_MODULE
 * @{
 *
 * @section MSG_SERVICE_FRAMEWORK_CONTROL_MODULE_HEADER Required Header
 *   \#include <msg.h>
 *
 * @section MSG_SERVICE_FRAMEWORK_CONTROL_MODULE_OVERVIEW Overview
 *
 * The CONTROL API provides the following functionalities:
 * - Handles message handle
 * - Get/Set message and setting values
 *
 * @section MSG_SERVICE_FRAMEWORK_CONTROL_MODULE_FEATURE Related Features
 * This API is related with the following features:\n
 *  - http://tizen.org/feature/network.telephony\n
 *  - http://tizen.org/feature/network.telephony.sms\n
 *
 * It is recommended to design feature related codes in your application for reliability.\n
 *
 * You can check if a device supports the related features for this API by using @ref CAPI_SYSTEM_SYSTEM_INFO_MODULE, thereby controlling the procedure of your application.\n
 *
 * To ensure your application is only running on the device with specific features, please define the features in your manifest file using the manifest editor in the SDK.\n
 *
 * More details on featuring your application can be found from <a href="../org.tizen.mobile.native.appprogramming/html/ide_sdk_tools/feature_element.htm"><b>Feature Element</b>.</a>
 *
 */

/**
 * @brief Opens a channel between an application and messaging framework.
 * @details For application to utilize the services of Messaging Framework, this API should be called to establish
 *          connection between the application and Messaging Framework.
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks The handle parameter returned must be used by application for further API calls to Messaging Service.
 * @remarks The memory for the handle need not be allocated by the application.
 * @remarks An error will be returned in case Messaging Service is not running.
 *
 * @param[in] handle The Message handle to be passed for all messaging services
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS                 Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER   Input parameter is invalid
 * @retval MSG_ERR_MEMORY_ERROR        Memory error
 * @retval MSG_ERR_COMMUNICATION_ERROR Communication error between client and server
 * @retval MSG_ERR_PERMISSION_DENIED   The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED       Not supported
 */

int msg_open_msg_handle(msg_handle_t *handle);


/**
 * @brief Closes the channel between application and messaging framework.
 * @details Once application utilizes services of Messaging Service, this API needs to be invoked
 *          to close the channel between application and Messaging Service.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks The handle parameter returned must be used by application for further API calls to Messaging Service.
 * @remarks The memory for the handle need not be allocated by the application \n
 * @remarks An error will be returned in case Messaging Service is not running.
 *
 * @param[in] handle The Message handle to be passed for all messaging services
 *
 * @return @c 0 on success,
 *       otherwise a negative error value
 *
 * @retval MSG_SUCCESS                 Success in operation
 * @retval MSG_ERR_INVALID_PARAMETER   Input parameter is invalid
 * @retval MSG_ERR_COMMUNICATION_ERROR Communication error between client and server
 * @retval MSG_ERR_PERMISSION_DENIED   The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED       Not supported
 */

int msg_close_msg_handle(msg_handle_t *handle);


/**
 * @brief Create structure pointer to get/set message framework data.
 * @details Get/set message framework data with this structure pointer.
 *
 * @since_tizen 2.3
 *
 * @remarks It should use defined enumerations.
 * @remarks You must release handle after operations.
 *
 * @param[in] field The enumerations to create the structure pointer
 *
 * @return The created structure pointer on success,
 *         otherwise null value
 *
 * @retval #msg_struct_t Successfully created structure pointer
 * @retval NULL          Invalid parameter
 * @retval MSG_ERR_NOT_SUPPORTED       Not supported
 *
 */

msg_struct_t msg_create_struct(int field);


/**
 * @brief Release memory for message data structure.
 * @details You must release memory of message data structure that is allocated by msg_create_struct().
 *
 * @since_tizen 2.3
 *
 * @remarks It should not be used after this API.
 *
 * @param[in] msg_struct_handle The structure pointer to release
 *
 * @return @c 0 on success,
 *         otherwise a negative error value
 *
 * @retval MSG_SUCCESS          Success in operation
 * @retval MSG_ERR_NULL_POINTER      Input parameter is NULL
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_release_struct(msg_struct_t *msg_struct_handle);


/**
 * @brief Release memory for list structure.
 * @details You must release memory of list structure that is allocated by getting list API.
 *
 * @since_tizen 2.3
 *
 * @remarks It should not be used after this API
 *
 * @param[in] msg_struct_list The list structure pointer to release
 *
 * @return @c 0 on success,
 *       otherwise a negative error value
 *
 * @retval MSG_SUCCESS          Success in operation
 * @retval MSG_ERR_NULL_POINTER      Input parameter is NULL
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_release_list_struct(msg_struct_list_s *msg_struct_list);


/**
 * @brief Get a specific integer value from message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle  A pointer of message structure type
 * @param[in]  field              The enumeration to get a value of specific field
 * @param[out] value              The requested integer value
 *
 * @return @c 0 on success,
 *       otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_get_int_value(msg_struct_t msg_struct_handle, int field, int *value);


/**
 * @brief Gets a specific string value from message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle A pointer of message structure type
 * @param[in]  field             The enumeration to get a value of specific field
 * @param[out] value             The requested string value
 * @param[in]  size              The allocated buffer size of application side
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_get_str_value(msg_struct_t msg_struct_handle, int field, char *value, int size);


/**
 * @brief Gets a specific boolean value from message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle  A pointer of message structure type
 * @param[in]  field              The enumeration to get a value of specific field
 * @param[out] value              The requested boolean value
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_get_bool_value(msg_struct_t msg_struct_handle, int field, bool *value);


/**
 * @brief Gets an inner structure handle from message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle  A pointer of message structure type
 * @param[in]  field              The enumeration to get a value of specific field
 * @param[out] value              The requested structure handle
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_get_struct_handle(msg_struct_t msg_struct_handle, int field, msg_struct_t *value);


/**
 * @brief Gets a list handle from message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle A pointer of message structure type
 * @param[in]  field             The enumeration to get a value of specific field
 * @param[out] value             The requested list handle
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */
/*================================================================================================*/
int msg_get_list_handle(msg_struct_t msg_struct_handle, int field, void **value);


/**
 * @brief Sets a specific integer value to message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle A pointer of message structure type
 * @param[in]  field             The enumeration to get a value of specific field
 * @param[in] value             The requested integer value
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_set_int_value(msg_struct_t msg_struct_handle, int field, int value);


/**
 * @brief Sets a specific string to message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle A pointer of message structure type
 * @param[in]  field             The enumeration to get a value of specific field
 * @param[in] value             The requested string
 * @param[in]  size              The requested size of string to set
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_MEMORY_ERROR      Memory error
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_set_str_value(msg_struct_t msg_struct_handle, int field, const char *value, int size);


/**
 * @brief Sets a specific boolean value to message structure data.
 *
 * @since_tizen 2.3
 *
 * @details Set a specific boolean value to message structure data.
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle  A pointer of message structure type
 * @param[in]  field              The enumeration to get a value of specific field
 * @param[in] value              The requested boolean value
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_set_bool_value(msg_struct_t msg_struct_handle, int field, bool value);


/**
 * @brief Sets a inner structure handle to message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle A pointer of message structure type
 * @param[in]  field             The enumeration to get a value of specific field
 * @param[in] value             The requested structure handle to set
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_set_struct_handle(msg_struct_t msg_struct_handle, int field, msg_struct_t value);


/**
 * @brief Set a list handle to message structure data.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid enumerations.
 *
 * @param[in]  msg_struct_handle A pointer of message structure type
 * @param[in]  field             The enumeration to get a value of specific field
 * @param[in] value             The requested list handle to set
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter (msg_struct_handle/value) is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter (field) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_set_list_handle(msg_struct_t msg_struct_handle, int field, void *value);


/**
 * @brief Gets a MMS structure handle from message structure.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid structure handle.
 *
 * @param[in]  msg_struct_handle  A pointer of message structure type
 * @param[out] mms_struct_handle  The MMS structure handle to get
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_get_mms_struct(msg_struct_t msg_struct_handle, msg_struct_t mms_struct_handle);


/**
 * @brief Sets a MMS structure handle to message structure.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid structure handle.
 *
 * @param[in]  msg_struct_handle  A pointer of message structure type
 * @param[in] mms_struct_handle  The MMS structure handle to set
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_set_mms_struct(msg_struct_t msg_struct_handle, msg_struct_t mms_struct_handle);


/**
 * @brief Adds an item to list handle of message structure type.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid list_handle.
 *
 * @param[in] msg_struct_handle A pointer of message structure type
 * @param[in] field             The field to set item
 * @param[in] item              The #msg_struct_t structure to be added to list handle
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is not valid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_list_add_item(msg_struct_t msg_struct_handle, int field, msg_struct_t *item);


/**
 * @brief Gets n-th data from list handle.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid list_handle.
 *
 * @param[in] list_handle A pointer of message structure type
 * @param[in] index       The index of list structure
 *
 * @return  #msg_struct_t value on success,
 *          otherwise @c NULL value
 *
 * @retval #msg_struct_t Successfully done
 * @retval NULL          Input parameter (list_handle) is not valid
 * @retval MSG_ERR_NOT_SUPPORTED       Not supported
 *
 */

msg_struct_t msg_list_nth_data(msg_list_handle_t list_handle, int index);


/**
 * @brief Gets the length(count) of list handle.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid list_handle.
 *          If there is an error, it will be returned as below values.
 *
 * @param[in] list_handle A pointer of message structure type
 *
 * @return The length of message list (int)
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_list_length(msg_list_handle_t list_handle);


/**
 * @brief Releases entire data of list handle in message structure.
 *
 * @since_tizen 2.3
 *
 * @remarks Use valid msg_struct_handle and enumeration.
 *
 * @param[in] msg_struct_handle A pointer of message structure type
 * @param[in] field             The enumeration to release (list handle)
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter is NULL
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_list_clear(msg_struct_t msg_struct_handle, int field);


/**
 * @brief Releases entire data of a list handle and the list.
 *
 * @since_tizen 2.3.1
 *
 * @remarks #msg_list_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] list_handle A pointer of message structure type
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_NULL_POINTER      Input parameter is NULL
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_list_free(msg_list_handle_t list_handle);


/**
 * @brief Adds a filter to block messages by address or word.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_struct_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle  The message handle
 * @param[in] filter  A pointer to message filter structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_FILTER_ERROR      Filter operation error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_add_filter(msg_handle_t handle, const msg_struct_t filter);


/**
 * @brief  Updates filter to block messages by address or word.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 * @remarks #msg_struct_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle The message handle
 * @param[in] filter A pointer to message filter structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_FILTER_ERROR      Filter operation error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_update_filter(msg_handle_t handle, const msg_struct_t filter);


/**
 * @brief Deletes the filter to block messages by address or word.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @param[in] handle    The message handle
 * @param[in] filter_id The filter index to delete from filter DB
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_FILTER_ERROR      Filter operation error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_delete_filter(msg_handle_t handle, msg_filter_id_t filter_id);


/**
 * @brief Gets all filter list.
 * @details This API is used to get a filter list that is already added filter set.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in]  handle      The message handle
 * @param[out] filter_list A pointer to list of message filter structure
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_FILTER_ERROR      Filter operation error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_filter_list(msg_handle_t handle, msg_struct_list_s *filter_list);


/**
 * @brief Sets the flag of message blocking.
 * @details This API is used to set a flag of block operation.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle    The message handle
 * @param[in] set_flag  Set @c true to enable block operation,
 *                      otherwise set @c false to disable block operation
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_FILTER_ERROR      Filter operation error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_filter_operation(msg_handle_t handle, bool set_flag);


/**
 * @brief Gets the flag value of message blocking.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in]  handle    The message handle
 * @param[out] set_flag  @c true if message blocking is enabled,
 *                       otherwise @c false if message blocking is not enabled
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_FILTER_ERROR      Filter operation error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_filter_operation(msg_handle_t handle, bool *set_flag);


/**
 * @brief Sets the flag of message blocking for specific filter.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle    The message handle
 * @param[in] filter_id The specific index to set flag
 * @param[in] active    Set @c true to enable the block operation,
 *                      otherwise @c false to not enable the block operation
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_FILTER_ERROR      Filter operation error
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_filter_active(msg_handle_t handle, msg_filter_id_t filter_id, bool active);


/**
 * @brief Gets SMSC list and informations.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of SMSC data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_smsc_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets SMSC list and informations.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of SMSC data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_smsc_opt(msg_handle_t handle, msg_struct_t msg_struct);



/**
 * @brief Gets CB channel list and informations.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of CB data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_cb_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets CB channel list and informations.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of CB data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_cb_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Gets SMS sending options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of sending option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_sms_send_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets SMS sending options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of sending option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_sms_send_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Gets MMS sending options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of sending option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_mms_send_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets MMS sending options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of sending option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_mms_send_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Gets MMS receiving options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of receiving option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_mms_recv_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets MMS receiving options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of receiving option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_mms_recv_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Gets push message options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of push message option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_push_msg_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets push message options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of push message option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_push_msg_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Gets voice message options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of voice message option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_voice_msg_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets voice message options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of voice message option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_voice_msg_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Gets message general options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of message general option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_general_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets message general options.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of message general option data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_general_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Gets message size.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.read
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of message size data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_get_msgsize_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Sets message size.
 *
 * @since_tizen 2.3
 * @privlevel public
 * @privilege %http://tizen.org/privilege/message.write
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in] handle     The message handle
 * @param[in] msg_struct A structure pointer of message size data
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is invalid
 * @retval MSG_ERR_PERMISSION_DENIED The application does not have the privilege to call this method
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 */

int msg_set_msgsize_opt(msg_handle_t handle, msg_struct_t msg_struct);


/**
 * @brief Calculates input text length by encode type.
 *
 * @since_tizen 2.3
 *
 * @remarks #msg_handle_t MUST be valid, otherwise the function will fail.
 *
 * @param[in]  msg_text            The string to calculate length
 * @param[in]  msg_encode_type_to  The current encode type
 * @param[out] text_size           The calculated text size
 * @param[out] segment_size        The message segment size
 * @param[out] msg_encode_type_in  The encode type it should be changed to
 *
 * @return  @c 0 on success,
 *        otherwise a negative error value
 *
 * @retval MSG_SUCCESS               Successfully done
 * @retval MSG_ERR_INVALID_PARAMETER Input parameter is not valid.
 * @retval MSG_ERR_NOT_SUPPORTED     Not supported
 *
 */

int msg_util_calculate_text_length(const char* msg_text, msg_encode_type_t msg_encode_type_to, unsigned int *text_size, unsigned int *segment_size, msg_encode_type_t *msg_encode_type_in);


/**
 * @}
*/


#ifdef __cplusplus
}
#endif

#endif /* MSG_H_ */
