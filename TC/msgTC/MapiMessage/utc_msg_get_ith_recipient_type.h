 /*
  * Copyright 2012  Samsung Electronics Co., Ltd
  *
  * Licensed under the Flora License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  *    http://www.tizenopensource.org/license
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */

#include <tet_api.h>

#include "MapiMessage.h"

static void startup(), cleanup();
void (*tet_startup) () = startup;
void (*tet_cleanup) () = cleanup;

 static void utc_msg_get_ith_recipient_type_001(void);
 static void utc_msg_get_ith_recipient_type_002(void);



 struct tet_testlist tet_testlist[] =	{
			{ utc_msg_get_ith_recipient_type_001,1},
			{ utc_msg_get_ith_recipient_type_002,2},
			{NULL,0}
 };
