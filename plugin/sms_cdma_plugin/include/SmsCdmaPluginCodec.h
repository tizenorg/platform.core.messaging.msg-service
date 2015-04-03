/*
* Copyright 2012-2013  Samsung Electronics Co., Ltd
*
* Licensed under the Flora License, Version 1.1 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://floralicense.org/license/
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef SMS_CDMA_PLUGIN_CODEC_H
#define SMS_CDMA_PLUGIN_CODEC_H


/*==================================================================================================
                                         INCLUDE FILES
==================================================================================================*/
#include "SmsCdmaPluginTypes.h"


/*==================================================================================================
                                     CLASS DEFINITIONS
==================================================================================================*/
class SmsPluginMsgCodec
{
public:

	static SmsPluginMsgCodec* instance();

	static bool checkInvalidPDU(const unsigned char *p_pkg_str, const int p_pkg_len);

	static int encodeMsg(const sms_trans_msg_s *pMsg, unsigned char *p_pkg_str);
	static int decodeMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_trans_msg_s *p_msg);

private:
	SmsPluginMsgCodec();
	~SmsPluginMsgCodec();

	static SmsPluginMsgCodec* pInstance;

	static int encodeP2PMsg(const sms_trans_p2p_msg_s *p_msg, unsigned char *p_pkg_str);
	static int encodeAckMsg(const sms_trans_ack_msg_s *p_msg, unsigned char *p_pkg_str);
	static int encodeCBMsg(const sms_trans_broadcast_msg_s *p_msg, unsigned char *p_pkg_str);

	static int encodeTelesvcMsg(const sms_telesvc_msg_s *p_msg, unsigned char *p_pkg_str);

	static int encodeTelesvcCancelMsg(const sms_telesvc_cancel_s *p_msg, unsigned char *p_pkg_str);
	static int encodeTelesvcSubmitMsg(const sms_telesvc_submit_s *p_msg, unsigned char *p_pkg_str);
	static int encodeTelesvcUserAckMsg(const sms_telesvc_user_ack_s *p_msg, unsigned char *p_pkg_str);
	static int encodeTelesvcReadAckMsg(const sms_telesvc_read_ack_s *p_msg, unsigned char *p_pkg_str);
	static int encodeTelesvcDeliverReportMsg(const sms_telesvc_report_s *p_msg, unsigned char *p_pkg_str);

	static int decodeP2PMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_trans_p2p_msg_s *p_p2p);
	static int decodeCBMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_trans_broadcast_msg_s *p_cb);
	static int decodeAckMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_trans_ack_msg_s *p_ack);

	static void decodeP2PTelesvcMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_msg_s *p_telesvc);
	static void decodeP2PDeliveryAckMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_deliver_ack_s *p_del_ack);
	static void decodeP2PSubmitReportMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_report_s *p_sub_report);
	static void decodeP2PUserAckMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_user_ack_s *p_user_ack);
	static void decodeP2PReadAckMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_read_ack_s *p_read_ack);
	static void decodeP2PDeliverMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_deliver_s *p_del);
	static void decodeP2PSubmitMsg(const unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_submit_s *p_sub);
	static void decodeCBBearerData(const unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_msg_s *p_telesvc, bool isCMAS);

	static int encodeUserData(const unsigned char* src, unsigned char *dest, int src_size);
	static void decodeUserData(unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_userdata_s *p_user);
	static void decodeCMASData(unsigned char *p_pkg_str, int p_pkg_len, sms_telesvc_cmasdata_s *p_cmas);

	static int decodeTeleId(const unsigned char *p_pkg_str, int p_pkg_len, sms_trans_telesvc_id_t *tele_id);
	static int decodeSvcCtg(const unsigned char *p_pkg_str, int p_pkg_len, sms_trans_svc_ctg_t *svc_ctg);
	static int decodeAddress(const unsigned char *p_pkg_str, int p_pkg_len, sms_trans_addr_s *addr);
	static int decodeSubAddress(const unsigned char *p_pkg_str, int p_pkg_len, sms_trans_sub_addr_s *sub_addr);

	static int decodeMsgId(const unsigned char *p_pkg_str, int pkg_len, sms_trans_msg_id_s *p_msg_id);
	static void decodeCallBackNum(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_addr_s *p_callback);
	static int decodeAbsTime(const unsigned char *p_pkg_str, sms_time_abs_s *p_time_abs);
	static sms_message_type_t findMsgType(const unsigned char *p_pkg_str, int pkg_len);
};

#endif //SMS_CDMA_PLUGIN_CODEC_H
