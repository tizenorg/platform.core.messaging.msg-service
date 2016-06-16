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

#ifndef MSGLBS_H_
#define MSGLBS_H_

/*==================================================================================================
											INCLUDE FILES
==================================================================================================*/

/*==================================================================================================
											DEFINES
==================================================================================================*/
#define MSG_LBS_PORT 7275


/*==================================================================================================
											FUNCTION PROTOTYPES
==================================================================================================*/
void MsgLbsSms(const char *msgBody, int msgSize);
void MsgLbsWapPush(const char *pushHeader, const char *pushBody, int pushBodySize);


#endif /* MSGLBS_H_ */
