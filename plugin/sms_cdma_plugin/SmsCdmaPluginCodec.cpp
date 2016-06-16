/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved
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

#include <stdio.h>
#include <string.h>

#include "MsgDebug.h"
#include "MsgTextConvert.h"

#include "SmsCdmaPluginParamCodec.h"
#include "SmsCdmaPluginCodec.h"


SmsPluginMsgCodec* SmsPluginMsgCodec::pInstance = NULL;


SmsPluginMsgCodec::SmsPluginMsgCodec()
{
}


SmsPluginMsgCodec::~SmsPluginMsgCodec()
{
}


SmsPluginMsgCodec* SmsPluginMsgCodec::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginMsgCodec();

	return pInstance;
}


void _shiftNBit(unsigned char *src, unsigned int n_bytes, unsigned int n_shift_bit)
{
	char tmp;

    for (unsigned int index = 1; index < n_bytes; index++) {
            tmp = src[index] >> (8 - n_shift_bit);
            src[index-1] |= tmp;
            src[index] = src[index] << n_shift_bit;
    }
}


void _shiftNBit_for_decode(unsigned char *src, unsigned int n_bytes, unsigned int n_shift_bit)
{
     for (unsigned int index = 0; index < n_bytes; index++) {
         src[index]<<=n_shift_bit;
         src[index] |= (src[index+1] >> (8 - n_shift_bit));
         }
}


void _copy_short_to_char(unsigned char *dest, const unsigned short *src)
{
	dest[0] = (0xff00 & *src) >> 8;
	dest[1] = 0x00ff & *src;
}


void _copy_char_to_short(unsigned short *dest, const unsigned char *src)
{
     *dest = src[0]*256 + src[1];
}


unsigned char _convert_to_BCD(unsigned char val)
{
	unsigned char ret = 0x00;
	ret = ((val/10) << 4) | (val%10);
	return ret;
}


int UnpackGSM7bitData(unsigned char *src, unsigned char *dest, unsigned int dataLen)
{
	unsigned int srcIdx = 0, dstIdx = 0, shift = 0;

	MSG_DEBUG("dataLen = %d", dataLen);

	for (; dstIdx < dataLen; dstIdx++) {
		if (shift == 0) {
			dest[dstIdx] = src[srcIdx] & 0x7F;

			shift = 7;
			srcIdx++;
			dstIdx++;

			if (dstIdx >= dataLen)
				break;
		}

		if (shift > 0) {
			dest[dstIdx] = (src[srcIdx-1] >> shift) + (src[srcIdx] << (8 - shift));

			dest[dstIdx] &= 0x7F;

			shift--;

			if (shift > 0)
				srcIdx++;
		}
	}

	return dstIdx;
}


int PackGSM7bitData(const unsigned char *pUserData, unsigned char *pPackData, int dataLen)
{
	int srcIdx = 0, dstIdx = 0, shift = 0;

	if (shift > 0)
		dstIdx = 1;

	while (srcIdx < dataLen) {
		if (shift == 0) {
			pPackData[dstIdx] = pUserData[srcIdx];

			shift = 7;
			srcIdx++;
			dstIdx++;

			if (srcIdx >= dataLen)
				break;
		}

		if (shift > 1) {
			pPackData[dstIdx-1] |= pUserData[srcIdx] << shift;
			pPackData[dstIdx] = pUserData[srcIdx] >> (8-shift);
			shift--;

			srcIdx++;
			dstIdx++;
		} else if (shift == 1) {
			pPackData[dstIdx-1] |= pUserData[srcIdx] << shift;

			srcIdx++;

			shift--;
		}
	}

	return dstIdx;
}


bool SmsPluginMsgCodec::checkInvalidPDU(const unsigned char *p_pkg_str, const int p_pkg_len)
{
	MSG_BEGIN();
	int offset = 0;

	if (!(p_pkg_str[offset] == 0x00 || p_pkg_str[offset] == 0x01 || p_pkg_str[offset] == 0x02)) {
		MSG_WARN("Invalid PDU : Message Type [%2x]", p_pkg_str[offset]);
		return false;
	}

	offset++;

	while (offset < p_pkg_len) {
		switch (p_pkg_str[offset]) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x08:
			offset += (p_pkg_str[offset+1]+2);
			break;
		default:
			MSG_WARN("Invalid PDU : Parameter ID [%2x], offset [%d]", p_pkg_str[offset], offset);
			return false;
		}
	}

	if (offset != p_pkg_len)
		return false;

	MSG_END();
	return true;
}


int SmsPluginMsgCodec::encodeMsg(const sms_trans_msg_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int encode_size = 0;

	switch (p_msg->type) {
	case SMS_TRANS_P2P_MSG:
		encode_size = encodeP2PMsg(&(p_msg->data.p2p_msg), p_pkg_str);
		break;
	case SMS_TRANS_BROADCAST_MSG:
		encode_size = encodeCBMsg(&(p_msg->data.cb_msg), p_pkg_str);
		break;
	case SMS_TRANS_ACK_MSG:
		encode_size = encodeAckMsg(&(p_msg->data.ack_msg), p_pkg_str);
		break;
	default:
		break;
	}


	MSG_END();

	return encode_size;
}


int SmsPluginMsgCodec::encodeP2PMsg(const sms_trans_p2p_msg_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int offset = 0, encode_size = 0;
	int addr_len = 0;
	int index = 0, len_index = 0;

	p_pkg_str[offset++] = SMS_TRANS_P2P_MSG;

	/* 1. teleservice id */
	p_pkg_str[offset++] = SMS_TRANS_PARAM_TELESVC_IDENTIFIER;
	p_pkg_str[offset++] = 2;
	/* fixed */
	/* memcpy(p_pkg_str+offset, &(p_msg->telesvc_id), sizeof(sms_trans_telesvc_id_t)); */
	_copy_short_to_char(p_pkg_str+offset, &(p_msg->telesvc_id));
	offset += sizeof(sms_trans_telesvc_id_t);

	/* 2. Service category */
	if (p_msg->svc_ctg < SMS_TRANS_SVC_CTG_UNDEFINED) {
		p_pkg_str[offset++] = SMS_TRANS_PARAM_SERVICE_CATEGORY;
		p_pkg_str[offset++] = 0x02;
		/* fixed */
		_copy_short_to_char(&p_pkg_str[offset], &(p_msg->svc_ctg));
	}

	/* 3. Address */
	p_pkg_str[offset++] = SMS_TRANS_PARAM_DEST_ADDRESS;

	/* Will be set to param length */
	len_index = offset++;

	p_pkg_str[offset] = p_msg->address.digit_mode ? 0x80 : 0x00;
	p_pkg_str[offset] |= (p_msg->address.number_mode ? 0x40 : 0x00);

	if (p_msg->address.digit_mode == false) {
		index = offset++;
		p_pkg_str[offset++] = p_msg->address.addr_len;

		addr_len = SmsPluginParamCodec::instance()->convertDigitToDTMF(p_msg->address.szData, p_msg->address.addr_len, 0, p_pkg_str+offset);

		for (int j=0; j < addr_len; j++) {
			MSG_DEBUG("ADDRESS 4BIT DTMF [%d] = [%02x]", j, p_pkg_str[offset+j]);
		}

		offset += addr_len;

		_shiftNBit(&p_pkg_str[index], offset-index+1, 6);
	} else if (p_msg->address.digit_mode == true) {
		p_pkg_str[offset] |= p_msg->address.number_type << 3;

		if (p_msg->address.number_mode == false) {
			p_pkg_str[offset++] |= p_msg->address.number_plan >> 1;
			p_pkg_str[offset++] |= p_msg->address.number_plan << 7;
			index = offset-1;
			p_pkg_str[offset++] = p_msg->address.addr_len;

			memcpy(p_pkg_str+offset, p_msg->address.szData, p_msg->address.addr_len);

			offset += p_msg->address.addr_len;

			_shiftNBit(&p_pkg_str[index], offset-index+1, 7);
		} else if (p_msg->address.number_mode == true) {
			index = offset++;
			p_pkg_str[offset++] = p_msg->address.addr_len;
			memcpy(p_pkg_str+offset, p_msg->address.szData, p_msg->address.addr_len);

			offset += p_msg->address.addr_len;

			_shiftNBit(&p_pkg_str[index], offset-index+1, 3);
		}
	}

	p_pkg_str[len_index] = offset - len_index - 1 ;
	MSG_DEBUG("Address subparam length field = [%d]", p_pkg_str[len_index]);

	/* 4. Sub address (optional) */
	if (p_msg->sub_address.addr_len > 0) {
		p_pkg_str[offset++] = SMS_TRANS_PARAM_ORG_SUB_ADDRESS;
		p_pkg_str[offset] = p_msg->sub_address.addr_len + 2;
		index = offset++;
		p_pkg_str[offset] |= p_msg->sub_address.type << 5;
		p_pkg_str[offset++] |= (p_msg->sub_address.odd ? 0x10 : 0x00);
		p_pkg_str[offset++] = p_msg->sub_address.addr_len;
		memcpy(p_pkg_str+offset, p_msg->sub_address.szData, p_msg->sub_address.addr_len);

		offset += p_msg->sub_address.addr_len;

		_shiftNBit(&p_pkg_str[index], offset-index+1, 4);
	}

	/* 5. Bearer reply option (optional) */
	if (p_msg->reply_seq > 0) {
		p_pkg_str[offset++] = SMS_TRANS_PARAM_BEARER_REPLY_OPTION;
		p_pkg_str[offset++] = 1;
		p_pkg_str[offset++] = (unsigned char)(p_msg->reply_seq << 2);
		MSG_DEBUG("Reply sequnce number = [%d]", p_msg->reply_seq);
	}

	/* 6. Bearer data */
    p_pkg_str[offset++] = SMS_TRANS_PARAM_BEARER_DATA;
    /* PARAMETER_LEN field should be filled at the last part. */
    index = offset++;

	unsigned char *encode_data = &p_pkg_str[offset];

	encode_size = encodeTelesvcMsg(&(p_msg->telesvc_msg), encode_data);
	/* PARAMETER_LEN */
	p_pkg_str[index] = encode_size;

	offset += encode_size;

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::encodeCBMsg(const sms_trans_broadcast_msg_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int offset = 0, encode_size = 0;
	int len_index = 0;

	/* 1. Service Category(Mandatory) */
	p_pkg_str[offset++] = SMS_TRANS_PARAM_SERVICE_CATEGORY;
	p_pkg_str[offset++] = 0x02;
	_copy_short_to_char(&p_pkg_str[offset], &p_msg->svc_ctg);


	/* 2. Bearer Data(Optional) */
	/* TODO: give condition */
    p_pkg_str[offset++] = SMS_TRANS_PARAM_BEARER_DATA;
    /* PARAMETER_LEN field should be filled at the last part. */
	len_index = offset++;

	unsigned char *encode_data = &p_pkg_str[offset];

	encode_size = encodeTelesvcMsg(&(p_msg->telesvc_msg), encode_data);
	/* PARAMETER_LEN */
	p_pkg_str[len_index] = encode_size;

	offset += encode_size;

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::encodeAckMsg(const sms_trans_ack_msg_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int offset = 0;
	int addr_len = 0, len_index = 0;
	int index = 0;

	/* 1. Address */
	p_pkg_str[offset++] = SMS_TRANS_PARAM_DEST_ADDRESS;

	/* Will be set to param length */
	len_index = offset++;

	p_pkg_str[offset] = p_msg->address.digit_mode ? 0x80 : 0x00;
	p_pkg_str[offset] |= (p_msg->address.number_mode ? 0x40 : 0x00);

	index = offset++;

	if (p_msg->address.digit_mode == false) {
		p_pkg_str[offset++] = p_msg->address.addr_len;

		addr_len = SmsPluginParamCodec::instance()->convertDigitToDTMF(p_msg->address.szData, p_msg->address.addr_len, 0, p_pkg_str+offset);

		for (int j=0; j < addr_len; j++) {
			MSG_DEBUG("ADDRESS 4BIT DTMF [%d] = [%02x]", j, p_pkg_str[offset+j]);
		}

		offset += addr_len;

		_shiftNBit(&p_pkg_str[index], offset-index+1, 6);
	} else if (p_msg->address.digit_mode == true) {
		p_pkg_str[offset] |= p_msg->address.number_type << 3;

		if (p_msg->address.number_mode == false) {
			p_pkg_str[offset++] |= p_msg->address.number_plan >> 1;
			p_pkg_str[offset++] |= p_msg->address.number_plan << 7;
			index++;
			p_pkg_str[offset++] = p_msg->address.addr_len;

			memcpy(p_pkg_str+offset, p_msg->address.szData, p_msg->address.addr_len);

			offset += p_msg->address.addr_len;

			_shiftNBit(&p_pkg_str[index], offset-index+1, 7);
		} else if (p_msg->address.number_mode == true) {
			p_pkg_str[++offset] = p_msg->address.addr_len;
			offset++;
			memcpy(p_pkg_str+offset, p_msg->address.szData, p_msg->address.addr_len);

			offset += p_msg->address.addr_len;

			_shiftNBit(&p_pkg_str[index], offset-index+1, 3);
		}
	}

	p_pkg_str[len_index] = offset - len_index - 1 ;
	MSG_DEBUG("Address subparam length field = [%d]", p_pkg_str[len_index]);

	/* 2. Sub address */
	if (p_msg->sub_address.addr_len > 0) {
		p_pkg_str[offset++] = SMS_TRANS_PARAM_ORG_SUB_ADDRESS;
		p_pkg_str[offset] = p_msg->sub_address.addr_len + 2;
		index = offset++;
		p_pkg_str[offset] |= p_msg->sub_address.type << 5;
		p_pkg_str[offset++] |= (p_msg->sub_address.odd ? 0x10 : 0x00);
		p_pkg_str[offset++] = p_msg->sub_address.addr_len;
		memcpy(p_pkg_str+offset, p_msg->sub_address.szData, p_msg->sub_address.addr_len);

		offset += p_msg->sub_address.addr_len;

		_shiftNBit(&p_pkg_str[index], offset-index+1, 4);
	}

	/* 3. Cause code */
	p_pkg_str[offset++] = SMS_TRANS_PARAM_CAUSE_CODES;
	index = offset++;
	p_pkg_str[offset] |= p_msg->cause_code.reply_seq << 2;
	p_pkg_str[offset] |= p_msg->cause_code.error_class;
	if (p_msg->cause_code.error_class != 0x0) {
		p_pkg_str[++offset] = p_msg->cause_code.cause_code;
	}
	p_pkg_str[index] = offset - index;

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::encodeTelesvcMsg(const sms_telesvc_msg_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();
	int encode_size = 0;

	MSG_DEBUG("Teleservice msg type = [%d]", p_msg->type);

	switch (p_msg->type) {
	/* case SMS_TYPE_DELIVER:
		encode_size = encodeTelesvcDeliverMsg(&(p_msg->data.delever), p_pkg_str);
		break;  */
	case SMS_TYPE_SUBMIT:
		encode_size = encodeTelesvcSubmitMsg(&(p_msg->data.submit), p_pkg_str);
		break;
	case SMS_TYPE_CANCEL:
		encode_size = encodeTelesvcCancelMsg(&(p_msg->data.cancel), p_pkg_str);
		break;
	case SMS_TYPE_USER_ACK:
		encode_size = encodeTelesvcUserAckMsg(&(p_msg->data.user_ack), p_pkg_str);
		break;
	case SMS_TYPE_READ_ACK:
		encode_size = encodeTelesvcReadAckMsg(&(p_msg->data.read_ack), p_pkg_str);
		break;
	case SMS_TYPE_DELIVER_REPORT:
		encode_size = encodeTelesvcDeliverReportMsg(&(p_msg->data.report), p_pkg_str);
		break;
	default:
		MSG_DEBUG("No matching type for [%d]", p_msg->type);
		break;
	}

	MSG_END();
	return encode_size;
}


/*
int SmsPluginMsgCodec::encodeTelesvcDeliverMsg(const sms_telesvc_deliver_s *p_msg, char *p_pkg_str)
{

}
*/


int SmsPluginMsgCodec::encodeTelesvcReadAckMsg(const sms_telesvc_read_ack_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int offset = 0;

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::encodeTelesvcUserAckMsg(const sms_telesvc_user_ack_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int offset = 0;

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::encodeTelesvcDeliverReportMsg(const sms_telesvc_report_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int offset = 0, len_index = 0, encode_size = 0;
	bool delReservedBit = false;

	/* 1. Message Identifier (Mandatory) */
	p_pkg_str[offset++] = SMS_BEARER_MESSAGE_IDENTIFIER;
	p_pkg_str[offset++] = 3;
	_copy_short_to_char(p_pkg_str+offset+1, &(p_msg->msg_id.msg_id));
	_shiftNBit(&p_pkg_str[offset], 3, 4);
	p_pkg_str[offset] |= SMS_TYPE_SUBMIT << 4;
	offset += 2;
	p_pkg_str[offset++] |= (p_msg->msg_id.header_ind ? 0x08 : 0x00);


	/* 2. TP-Failure Cause (Conditional) */
	if (p_msg->tp_fail_cause >= 0x8000) {
		p_pkg_str[offset++] = SMS_BEARER_TP_FAILURE_CAUSE;
		p_pkg_str[offset++] = 1;
		p_pkg_str[offset++] = p_msg->tp_fail_cause;
	}

	/* 3. User Data (Optional) */
	if (p_msg->user_data.data_len > 0) {
		p_pkg_str[offset++] = SMS_BEARER_USER_DATA;
		len_index = offset;
		offset++;
		if (p_msg->user_data.encode_type == 0x01 || p_msg->user_data.encode_type == 0x0a)
			p_pkg_str[offset++] = p_msg->user_data.msg_type;

		p_pkg_str[offset++] = p_msg->user_data.encode_type << 3;
		p_pkg_str[offset++] = p_msg->user_data.data_len;

		if (p_msg->user_data.encode_type == SMS_ENCODE_7BIT_ASCII || p_msg->user_data.encode_type == SMS_ENCODE_GSM7BIT) {
			encode_size = encodeUserData(p_msg->user_data.user_data, &p_pkg_str[offset], p_msg->user_data.data_len);
			offset += encode_size;
			if (p_msg->user_data.data_len % 8 > 4)
				delReservedBit = true;
		} else {
			memcpy(p_pkg_str+offset, p_msg->user_data.user_data, p_msg->user_data.data_len);
			offset += p_msg->user_data.data_len;
		}
		_shiftNBit(&p_pkg_str[len_index+1], offset-len_index-1, 3);

		if (delReservedBit == true)
			offset--;

		p_pkg_str[len_index] = offset - len_index - 1;
	}

	/* 4. Language Indicator (Optional) */
	/* TODO : give condition */
	/*
	p_pkg_str[offset++] = SMS_BEARER_ALERT_ON_MSG_DELIVERY;
	p_pkg_str[offset++] = 1;
	p_pkg_str[offset++] = p_msg->language;
	*/

	/* 5. Multiple Encoding User Data (Optional) */
	/* Omitted */

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::encodeTelesvcCancelMsg(const sms_telesvc_cancel_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int offset = 0;

	/* 1. Message Identifier */
	p_pkg_str[offset++] = SMS_BEARER_MESSAGE_IDENTIFIER;
	p_pkg_str[offset++] = 3;
	_copy_short_to_char(p_pkg_str+offset+1, &(p_msg->msg_id.msg_id));
	_shiftNBit(&p_pkg_str[offset], 3, 4);
	p_pkg_str[offset] |= SMS_TYPE_SUBMIT << 4;
	offset += 2;
	p_pkg_str[offset++] |= (p_msg->msg_id.header_ind ? 0x08 : 0x00);

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::encodeTelesvcSubmitMsg(const sms_telesvc_submit_s *p_msg, unsigned char *p_pkg_str)
{
	MSG_BEGIN();

	int offset = 0, len_index = 0;
	int encode_size = 0;
	bool delReservedBit = false;

	/* 1. Message Identifier */
	p_pkg_str[offset++] = SMS_BEARER_MESSAGE_IDENTIFIER;
	p_pkg_str[offset++] = 3;
	_copy_short_to_char(p_pkg_str+offset+1, &(p_msg->msg_id.msg_id));
	_shiftNBit(&p_pkg_str[offset], 3, 4);
	p_pkg_str[offset] |= SMS_TYPE_SUBMIT << 4;
	offset += 2;
	p_pkg_str[offset++] |= (p_msg->msg_id.header_ind ? 0x08 : 0x00);

	/* 2. User Data */
	if (p_msg->user_data.data_len > 0) {
		p_pkg_str[offset++] = SMS_BEARER_USER_DATA;
		len_index = offset;
		offset++;
		if (p_msg->user_data.encode_type == 0x01 || p_msg->user_data.encode_type == 0x0a)
			p_pkg_str[offset++] = p_msg->user_data.msg_type;

		p_pkg_str[offset++] = p_msg->user_data.encode_type << 3;
		p_pkg_str[offset++] = p_msg->user_data.data_len;

		if (p_msg->user_data.encode_type == SMS_ENCODE_7BIT_ASCII) {
			encode_size = encodeUserData(p_msg->user_data.user_data, &p_pkg_str[offset], p_msg->user_data.data_len);
			offset += encode_size;
			if (p_msg->user_data.data_len % 8 > 4)
				delReservedBit = true;
		} else if (p_msg->user_data.encode_type == SMS_ENCODE_GSM7BIT) {
			encode_size = PackGSM7bitData(p_msg->user_data.user_data, &p_pkg_str[offset], p_msg->user_data.data_len);
			offset += encode_size;
			if (p_msg->user_data.data_len % 8 > 4)
				delReservedBit = true;
		} else if (p_msg->user_data.encode_type == SMS_ENCODE_UNICODE) {
			MsgTextConvert *textCvt = MsgTextConvert::instance();
			encode_size = textCvt->convertUTF8ToUCS2(&p_pkg_str[offset], SMS_MAX_USER_DATA_LEN, p_msg->user_data.user_data, p_msg->user_data.data_len);
			p_pkg_str[offset-1] = encode_size / 2;
			offset += encode_size;
		} else {
			memcpy(p_pkg_str+offset, p_msg->user_data.user_data, p_msg->user_data.data_len);
			offset += p_msg->user_data.data_len;
		}
		_shiftNBit(&p_pkg_str[len_index+1], offset-len_index-1, 3);

		if (delReservedBit == true)
			offset--;

		p_pkg_str[len_index] = offset - len_index - 1;
	}
	/* Sprint and Verizon issue */
	/* 3. Validity Period */
	/*
	if (p_msg->val_period.format == SMS_TIME_RELATIVE) {
		p_pkg_str[offset++] = SMS_BEARER_VALIDITY_PERIOD_RELATIVE;
		p_pkg_str[offset++] = 1;
		p_pkg_str[offset++] = p_msg->val_period.time.rel_time.rel_time;

	} else if (p_msg->val_period.format == SMS_TIME_ABSOLUTE){
		p_pkg_str[offset++] = SMS_BEARER_VALIDITY_PERIOD_ABSOLUTE;
		p_pkg_str[offset++] = 6;
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->val_period.time.abs_time.year);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->val_period.time.abs_time.month);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->val_period.time.abs_time.day);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->val_period.time.abs_time.hours);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->val_period.time.abs_time.minutes);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->val_period.time.abs_time.seconds);
	}
	*/

	/* 4. Deferred Delivery Time */
	if (p_msg->defer_val_period.format == SMS_TIME_RELATIVE) {
		p_pkg_str[offset++] = SMS_BEARER_DEFERRED_DELIVERY_TIME_RELATIVE;
		p_pkg_str[offset++] = 1;
		p_pkg_str[offset++] = p_msg->defer_val_period.time.rel_time.rel_time;

	} else if (p_msg->defer_val_period.format == SMS_TIME_ABSOLUTE) {
		p_pkg_str[offset++] = SMS_BEARER_DEFERRED_DELIVERY_TIME_ABSOLUTE;
		p_pkg_str[offset++] = 6;
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->defer_val_period.time.abs_time.year);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->defer_val_period.time.abs_time.month);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->defer_val_period.time.abs_time.day);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->defer_val_period.time.abs_time.hours);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->defer_val_period.time.abs_time.minutes);
		p_pkg_str[offset++] = _convert_to_BCD(p_msg->defer_val_period.time.abs_time.seconds);
	}

	/* 5. Priority Indicator */
	if (p_msg->priority >= SMS_PRIORITY_NORMAL && p_msg->priority <=	SMS_PRIORITY_EMERGENCY) {
		p_pkg_str[offset++] = SMS_BEARER_PRIORITY_INDICATOR;
		p_pkg_str[offset++] = 1;
		p_pkg_str[offset++] = p_msg->priority << 6;
	}

	/* Sprint and Verizon issue */
	/* 6. Privacy Indicator */
	/*
	if (p_msg->privacy >= SMS_PRIVACY_NOT_RESTRICTED && p_msg->privacy <= SMS_PRIVACY_SECRET) {
		p_pkg_str[offset++] = SMS_BEARER_PRIVACY_INDICATOR;
		p_pkg_str[offset++] = 1;
		p_pkg_str[offset++] = p_msg->privacy << 6;
	}
	*/

	/* 7. Reply Option */
	if (p_msg->reply_opt.user_ack_req | p_msg->reply_opt.deliver_ack_req | p_msg->reply_opt.read_ack_req | p_msg->reply_opt.report_req) {
		p_pkg_str[offset++] = SMS_BEARER_REPLY_OPTION;
		p_pkg_str[offset++] = 1;
		p_pkg_str[offset] |= p_msg->reply_opt.user_ack_req << 7;
		p_pkg_str[offset] |= p_msg->reply_opt.deliver_ack_req << 6;
		p_pkg_str[offset] |= p_msg->reply_opt.read_ack_req << 5;
		p_pkg_str[offset++] |= p_msg->reply_opt.report_req << 4;
	}

	/* 8. Alert on Message Delivery */
	/* TODO : give condition */
	/*
	p_pkg_str[offset++] = SMS_BEARER_ALERT_ON_MSG_DELIVERY;
	p_pkg_str[offset++] = 1;
	p_pkg_str[offset++] = p_msg->alert_priority << 6;
	*/

	/* 9. Language Indicator */
	/* TODO : give condition */
	/*
	p_pkg_str[offset++] = SMS_BEARER_ALERT_ON_MSG_DELIVERY;
	p_pkg_str[offset++] = 1;
	p_pkg_str[offset++] = p_msg->language;
	*/

	/* 10. Call-back Number */
	if (p_msg->callback_number.addr_len > 0) {
		p_pkg_str[offset++] = SMS_BEARER_CALLBACK_NUMBER;

		int len_index = offset++;

		p_pkg_str[offset] |= p_msg->callback_number.digit_mode << 7;

		if (p_msg->callback_number.digit_mode == false) {
			p_pkg_str[offset++] |= (p_msg->callback_number.addr_len & 0xfe) >> 1;
			p_pkg_str[offset] |= (p_msg->callback_number.addr_len & 0x01) << 7;
			int addr_len = SmsPluginParamCodec::instance()->convertDigitToDTMF(p_msg->callback_number.szData, p_msg->callback_number.addr_len, 1, p_pkg_str+offset);
			offset += addr_len;
		} else if (p_msg->callback_number.digit_mode == true) {
			p_pkg_str[offset] |= p_msg->callback_number.number_type << 6;
			p_pkg_str[offset++] |= p_msg->callback_number.number_plan;
			p_pkg_str[offset++] = p_msg->callback_number.addr_len;
			memcpy(p_pkg_str+offset, p_msg->callback_number.szData, p_msg->callback_number.addr_len);
			offset += p_msg->callback_number.addr_len;
		}

		p_pkg_str[len_index] = offset- len_index - 1;
	}

	/* 11. Multiple Encoding User Data */
	/* Omitted */

	/* 12. Message Deposit Index */
	/* Omitted */

	/* 13. Service Category Program Results */
	/* Omitted */

	MSG_END();
	return offset;
}


int SmsPluginMsgCodec::decodeMsg(const unsigned char *p_pkg_str, int pkg_len, sms_trans_msg_s *p_msg)
{
	MSG_BEGIN();

	int decodelen = 0;

	char mti = p_pkg_str[0] & 0xff;

	switch (mti) {
	case SMS_TRANS_P2P_MSG:
		p_msg->type = SMS_TRANS_P2P_MSG;
		decodelen = decodeP2PMsg(p_pkg_str+1, pkg_len-1, &(p_msg->data.p2p_msg));
		break;
	case SMS_TRANS_BROADCAST_MSG:
		p_msg->type = SMS_TRANS_BROADCAST_MSG;
		decodelen = decodeCBMsg(p_pkg_str+1, pkg_len-1, &(p_msg->data.cb_msg));
		break;
	case SMS_TRANS_ACK_MSG:
		p_msg->type = SMS_TRANS_ACK_MSG;
		decodelen = decodeAckMsg(p_pkg_str+1, pkg_len-1, &(p_msg->data.ack_msg));
		break;
	default:
		p_msg->type = SMS_TRANS_TYPE_RESERVED;
		break;
	}

	MSG_END();

	return decodelen+1;
}


int SmsPluginMsgCodec::decodeP2PMsg(const unsigned char *p_pkg_str, int pkg_len, sms_trans_p2p_msg_s *p_p2p)
{
	MSG_BEGIN();

	int offset = 0, tmp_len = 0;

	while (offset < pkg_len) {
		switch (p_pkg_str[offset]) {
		case 0x00:
			/* Teleservice Identifier */
			offset += decodeTeleId(p_pkg_str+offset, pkg_len, &(p_p2p->telesvc_id));
			break;
			case 0x01:
			/* Service Category */
			offset += decodeSvcCtg(p_pkg_str+offset, pkg_len, &(p_p2p->svc_ctg));
			break;
		case 0x02:
		case 0x04:
			/* Address */
			offset += decodeAddress(p_pkg_str+offset, pkg_len, &(p_p2p->address));
			break;
		case 0x03:
		case 0x05:
			/* Subaddress */
			offset += decodeSubAddress(p_pkg_str+offset, pkg_len, &(p_p2p->sub_address));
			break;
		case 0x06:
			/* Bearer Reply Option*/
			offset += 2;
			p_p2p->reply_seq = (sms_trans_reply_seq_t)(p_pkg_str[offset] >> 2);
			offset++;
			break;
		case 0x08:
			/* Bearer Data */
			tmp_len = p_pkg_str[++offset];
			decodeP2PTelesvcMsg(p_pkg_str+offset+1, tmp_len, &p_p2p->telesvc_msg);
			offset += (tmp_len+1);
			break;
		default:
			break;
		}
	}

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::decodeCBMsg(const unsigned char *p_pkg_str, int pkg_len, sms_trans_broadcast_msg_s *p_cb)
{
	MSG_BEGIN();

	int offset = 0, tmp_len = 0;

	while (offset < pkg_len) {
		if (p_pkg_str[offset] == 0x01) {
			/* Service Category */
			offset += decodeTeleId(p_pkg_str+offset, pkg_len, &(p_cb->svc_ctg));
		} else if (p_pkg_str[offset] == 0x08) {
			/* Bearer Data */
			tmp_len = p_pkg_str[++offset];
			if (p_cb->svc_ctg >= SMS_TRANS_SVC_CTG_CMAS_PRESIDENTIAL && p_cb->svc_ctg <= SMS_TRANS_SVC_CTG_CMAS_TEST) {
				decodeCBBearerData(p_pkg_str+offset+1, tmp_len, &(p_cb->telesvc_msg), true);
			} else {
				decodeCBBearerData(p_pkg_str+offset+1, tmp_len, &(p_cb->telesvc_msg), false);
			}

			offset += (tmp_len+1);
		}
	}

	MSG_END();

	return offset;
}


int SmsPluginMsgCodec::decodeAckMsg(const unsigned char *p_pkg_str, int pkg_len, sms_trans_ack_msg_s *p_ack)
{
	MSG_BEGIN();

	int offset = 0;

	while (offset < pkg_len) {
		if (p_pkg_str[offset] == 0x04) {
			/* Destination Address */
			offset += decodeAddress(p_pkg_str+offset, pkg_len, &(p_ack->address));
		} else if (p_pkg_str[offset] == 0x05) {
			/* Destination Subaddress */
			offset += decodeSubAddress(p_pkg_str+offset, pkg_len, &(p_ack->sub_address));
		} else if (p_pkg_str[offset] == 0x07) {
			/* Cause Codes */
			offset += 2;
			p_ack->cause_code.reply_seq = (sms_trans_reply_seq_t)p_pkg_str[offset] >> 2;
			switch (p_pkg_str[offset++] & 0x03) {
			case 0x00:
				p_ack->cause_code.error_class = SMS_TRANS_ERR_CLASS_NONE;
				break;
			case 0x01:
				break;
			case 0x10:
				p_ack->cause_code.error_class = SMS_TRANS_ERR_CLASS_TEMPORARY;
				break;
			case 0x11:
				p_ack->cause_code.error_class = SMS_TRANS_ERR_CLASS_PERMANENT;
				break;
			}

			if (p_ack->cause_code.error_class != SMS_TRANS_ERR_CLASS_NONE) {
				p_ack->cause_code.cause_code = (sms_trans_cause_code_t)p_pkg_str[offset++];
			}
		}
	}

	MSG_END();

	return offset;
}


void SmsPluginMsgCodec::decodeP2PTelesvcMsg(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_msg_s *p_telesvc)
{
	MSG_BEGIN();

	p_telesvc->type = findMsgType(p_pkg_str, pkg_len);

	MSG_DEBUG("Msg Type = [%d]", p_telesvc->type);

	switch (p_telesvc->type) {
	case SMS_TYPE_DELIVER:
		decodeP2PDeliverMsg(p_pkg_str, pkg_len, &p_telesvc->data.deliver);
		break;
	case SMS_TYPE_SUBMIT:
		decodeP2PSubmitMsg(p_pkg_str, pkg_len, &p_telesvc->data.submit);
		break;
	case SMS_TYPE_DELIVERY_ACK:
		decodeP2PDeliveryAckMsg(p_pkg_str, pkg_len, &p_telesvc->data.delivery_ack);
		break;
	case SMS_TYPE_USER_ACK:
		decodeP2PUserAckMsg(p_pkg_str, pkg_len, &p_telesvc->data.user_ack);
		break;
	case SMS_TYPE_READ_ACK:
		decodeP2PReadAckMsg(p_pkg_str, pkg_len, &p_telesvc->data.read_ack);
		break;
	case SMS_TYPE_SUBMIT_REPORT:
		/* decodeP2PSubmitReportMsg(p_pkg_str, pkg_len, &p_telesvc->data.report); */
		break;
	default:
		break;
	}

	MSG_END();
}


void SmsPluginMsgCodec::decodeP2PDeliverMsg(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_deliver_s *p_del)
{
	MSG_BEGIN();

	int offset = 0, tmp_len, tmp_off;
	unsigned short tmp_param_s;
	unsigned char tmp_str[pkg_len+1];

	while (offset < pkg_len) {
		MSG_DEBUG("current offset = [%d] [%x]", offset, p_pkg_str[offset]);

		switch (p_pkg_str[offset]) {
		case 0x00:
			/* Message Identifier */
			offset += decodeMsgId(p_pkg_str+offset, 5, &(p_del->msg_id));
			break;
		case 0x01:
			/* User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);
			decodeUserData(tmp_str, tmp_len, &(p_del->user_data));
			offset += tmp_len;
			break;
		case 0x03:
			/* Message Center Time Stamp */
			offset += 2;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_del->time_stamp));
			break;
		case 0x04:
			/* Validity Period - Absolute */
			offset += 2;
			p_del->val_period.format = SMS_TIME_ABSOLUTE;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_del->val_period.time.abs_time));
			break;
		case 0x05:
			/* Validity Period - Relative */
			offset += 2;
			p_del->val_period.format = SMS_TIME_RELATIVE;
			p_del->val_period.time.rel_time.rel_time = (sms_relative_time_t)p_pkg_str[offset++];
			break;
		case 0x08:
			/* Priority Indicator */
			offset += 2;
			p_del->priority = (sms_priority_indicator_t)((p_pkg_str[offset++] & 0xc0) >> 6);
			break;
		case 0x09:
			/* Privacy Indicator */
			offset += 2;
			p_del->privacy = (sms_privacy_indicator_t)((p_pkg_str[offset++] & 0xc0) >> 6);
			break;
		case 0x0a:
			/* Reply Option */
			offset += 2;

			if (p_pkg_str[offset] & 0x80)
				p_del->reply_opt.user_ack_req = true;
			else
				p_del->reply_opt.user_ack_req = false;

			if (p_pkg_str[offset] & 0x40)
				p_del->reply_opt.deliver_ack_req = true;
			else
				p_del->reply_opt.deliver_ack_req = false;

			if (p_pkg_str[offset] & 0x20)
				p_del->reply_opt.read_ack_req = true;
			else
				p_del->reply_opt.read_ack_req = false;

			if (p_pkg_str[offset] & 0x10)
				p_del->reply_opt.report_req = true;
			else
				p_del->reply_opt.report_req = false;

			offset++;
			break;
		case 0x0b:
			/* Number of Message */
			offset += 2;
			p_del->num_msg = (((p_pkg_str[offset] & 0xf0) >> 4) * 10) + (p_pkg_str[offset] & 0x0f);
			offset++;
			break;
		case 0x0c:
			/* Alert on Message Delivery */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];
			if (tmp_len > 0)
				p_del->alert_priority = (sms_alert_priority_t)((p_pkg_str[offset] & 0xc0) >> 6);
			offset += tmp_len;
			break;
		case 0x0d:
			/* Language Indicator */
			offset += 2;
			p_del->language = (sms_language_type_t)p_pkg_str[offset++];
			break;
		case 0x0e:
			/* Call-Back Number */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			decodeCallBackNum(&p_pkg_str[offset], tmp_len, &(p_del->callback_number));

			offset += tmp_len;
			break;
		case 0x0f:
			/* Message Display Mode */
			offset += 2;

			p_del->display_mode = (sms_display_mode_t)((p_pkg_str[offset++] & 0xc0) >> 6);
			break;
		case 0x10:
			/* Multiple Encoding User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			/* TODO */

			offset += tmp_len;
			break;
		case 0x11:
			/* Message Deposit Index */
			offset += 2;

			memset(&tmp_param_s, 0x00, sizeof(unsigned short));
			_copy_char_to_short(&tmp_param_s, p_pkg_str+offset);
			p_del->deposit_id = tmp_param_s;

			offset += 2;
			break;
		case 0x16:
			/* Enhanced VMN */
			offset++;
			tmp_len = p_pkg_str[offset++];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);
			tmp_off = 0;

			p_del->enhanced_vmn.priority = (sms_priority_indicator_t)(p_pkg_str[offset] >> 6);
			p_del->enhanced_vmn.password_req = p_pkg_str[offset] & 0x20 ? true : false;
			p_del->enhanced_vmn.setup_req = p_pkg_str[offset] & 0x10 ? true : false;
			p_del->enhanced_vmn.pw_change_req = p_pkg_str[offset] & 0x08 ? true : false;
			_shiftNBit_for_decode(tmp_str, tmp_len, 5);

			if (p_del->enhanced_vmn.setup_req || p_del->enhanced_vmn.pw_change_req) {
				p_del->enhanced_vmn.min_pw_len = tmp_str[tmp_off] >> 4;
				p_del->enhanced_vmn.max_pw_len = tmp_str[tmp_off++] & 0x0f;
			}

			p_del->enhanced_vmn.vm_num_unheard_msg = tmp_str[tmp_off++];
			p_del->enhanced_vmn.vm_mailbox_alm_full = tmp_str[tmp_off] & 0x80 ? true : false;
			p_del->enhanced_vmn.vm_mailbox_full = tmp_str[tmp_off] & 0x40 ? true : false;
			p_del->enhanced_vmn.reply_allowed = tmp_str[tmp_off] & 0x20 ? true : false;
			p_del->enhanced_vmn.fax_included = tmp_str[tmp_off] & 0x10 ? true : false;
			p_del->enhanced_vmn.vm_len = ((tmp_str[tmp_off] >> 0x0f) << 8) | tmp_str[tmp_off+1];
			tmp_off += 2;
			p_del->enhanced_vmn.vm_ret_day = tmp_str[tmp_off] >> 1;
			_shiftNBit_for_decode(tmp_str, tmp_len, 7);
			p_del->enhanced_vmn.vm_msg_id = (tmp_str[tmp_off] << 8) | tmp_str[tmp_off+1];
			tmp_off += 2;
			p_del->enhanced_vmn.vm_mailbox_id = (tmp_str[tmp_off] << 8) | tmp_str[tmp_off+1];
			tmp_off += 2;

			p_del->enhanced_vmn.an_digit_mode = (sms_digit_mode_t)(tmp_str[tmp_off] & 0x80 ? true : false);
			p_del->enhanced_vmn.an_number_type = (sms_number_type_t)((tmp_str[tmp_off] & 0x70) >> 4);
			if (p_del->enhanced_vmn.an_digit_mode) {
				p_del->enhanced_vmn.an_number_plan = (sms_number_plan_t)(tmp_str[tmp_off++] & 0x0f);
				p_del->enhanced_vmn.an_num_field = tmp_str[tmp_off++];
				for (int i = 0; i < p_del->enhanced_vmn.an_num_field; i++) {
					switch (tmp_str[tmp_off] & 0xf0) {
					case 0x10:
					case 0x20:
					case 0x30:
					case 0x40:
					case 0x50:
					case 0x60:
					case 0x70:
					case 0x80:
					case 0x90:
						p_del->enhanced_vmn.an_char[i] = ((tmp_str[tmp_off] & 0xf0) >> 4) + '0';
						break;
					case 0xa0:
						p_del->enhanced_vmn.an_char[i] = '0';
						break;
					case 0xb0:
						p_del->enhanced_vmn.an_char[i] = '*';
						break;
					case 0xc0:
						p_del->enhanced_vmn.an_char[i] = '#';
						break;
					default:
						break;
					}

					_shiftNBit_for_decode(tmp_str, tmp_len, 4);
				}
			} else {
				_shiftNBit_for_decode(tmp_str, tmp_len, 4);
				p_del->enhanced_vmn.an_num_field = tmp_str[tmp_off++];
				memset(p_del->enhanced_vmn.an_char, 0x00, sizeof(p_del->enhanced_vmn.an_char));
				memcpy(p_del->enhanced_vmn.an_char, tmp_str+tmp_off, p_del->enhanced_vmn.an_num_field);
				tmp_off += p_del->enhanced_vmn.an_num_field;
			}

			p_del->enhanced_vmn.cli_digit_mode = (sms_digit_mode_t)(tmp_str[offset] & 0x80 ? true : false);
			p_del->enhanced_vmn.cli_number_type = (sms_number_type_t)(tmp_str[offset] & 0x70 >> 4);
			if (p_del->enhanced_vmn.cli_digit_mode) {
				p_del->enhanced_vmn.cli_number_plan = (sms_number_plan_t)(tmp_str[tmp_off++] & 0x0f);
				p_del->enhanced_vmn.cli_num_field = tmp_str[tmp_off++];
				for (int i = 0; i < p_del->enhanced_vmn.cli_num_field; i++) {
					switch (tmp_str[tmp_off] & 0xf0) {
					case 0x10:
					case 0x20:
					case 0x30:
					case 0x40:
					case 0x50:
					case 0x60:
					case 0x70:
					case 0x80:
					case 0x90:
						p_del->enhanced_vmn.cli_char[i] = ((tmp_str[tmp_off] & 0xf0) >> 4) + '0';
						break;
					case 0xa0:
						p_del->enhanced_vmn.cli_char[i] = '0';
						break;
					case 0xb0:
						p_del->enhanced_vmn.cli_char[i] = '*';
						break;
					case 0xc0:
						p_del->enhanced_vmn.cli_char[i] = '#';
						break;
					default:
						break;
					}

					_shiftNBit_for_decode(tmp_str, tmp_len, 4);
				}
			} else {
				_shiftNBit_for_decode(tmp_str, tmp_len, 4);
				p_del->enhanced_vmn.cli_num_field = tmp_str[tmp_off++];
				memset(p_del->enhanced_vmn.cli_char, 0x00, sizeof(p_del->enhanced_vmn.cli_char));
				memcpy(p_del->enhanced_vmn.cli_char, tmp_str+tmp_off, p_del->enhanced_vmn.cli_num_field);
				tmp_off += p_del->enhanced_vmn.cli_num_field;
			}

			offset += tmp_len;
			break;
		case 0x17:
			/* Enhanced VMN Ack */
			offset++;
			tmp_len = p_pkg_str[offset++];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);
			p_del->enhanced_vmn_ack.vm_mailbox_id = tmp_str[offset] << 8 | tmp_str[offset+1];
			offset += 2;
			p_del->enhanced_vmn_ack.vm_num_unheard_msg = tmp_str[offset++];
			p_del->enhanced_vmn_ack.num_delete_ack = tmp_str[offset] >> 5;
			p_del->enhanced_vmn_ack.num_play_ack = (tmp_str[offset] & 0x1c) >> 2;
			_shiftNBit_for_decode(tmp_str, tmp_len, 6);
			for (int i = 0; i < p_del->enhanced_vmn_ack.num_delete_ack; i++) {
				p_del->enhanced_vmn_ack.da_vm_msg_id[i] = tmp_str[offset] << 8 | tmp_str[offset+1];
				offset += 2;
			}
			for (int i = 0; i < p_del->enhanced_vmn_ack.num_play_ack; i++) {
				p_del->enhanced_vmn_ack.pa_vm_msg_id[i] = tmp_str[offset] << 8 | tmp_str[offset+1];
				offset += 2;
			}
			break;
		default:
			/* skip unrecognized sub parameters */
			offset++;
			tmp_len = p_pkg_str[offset++];
			offset += tmp_len;
			break;
		}
	}

	MSG_END();
}


void SmsPluginMsgCodec::decodeP2PSubmitMsg(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_submit_s *p_sub)
{
	int offset = 0, tmp_len;
	unsigned short tmp_param_s;
	unsigned char tmp_str[pkg_len+1];

	while (offset < pkg_len) {
		if (p_pkg_str[offset] == 0x00) {
			/* Message Identifier */
			offset += decodeMsgId(p_pkg_str+offset, 5, &(p_sub->msg_id));
		} else if (p_pkg_str[offset] == 0x01) {
			/* User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);

			decodeUserData(tmp_str, tmp_len, &(p_sub->user_data));

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x04) {
			/* Validity Period - Absolute */
			offset += 2;
			p_sub->val_period.format = SMS_TIME_ABSOLUTE;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_sub->val_period.time.abs_time));
		} else if (p_pkg_str[offset] == 0x05) {
			/* Validity Period - Relative */
			offset += 2;
			p_sub->val_period.format = SMS_TIME_RELATIVE;
			p_sub->val_period.time.rel_time.rel_time = (sms_relative_time_t)p_pkg_str[offset++];
		} else if (p_pkg_str[offset] == 0x06) {
			/* Deferred Delivery Time - Absolute */
			offset += 2;
			p_sub->defer_val_period.format = SMS_TIME_ABSOLUTE;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_sub->defer_val_period.time.abs_time));
		} else if (p_pkg_str[offset] == 0x07) {
			/* Deferred Delivery Time - Relative */
			offset += 2;
			p_sub->defer_val_period.format = SMS_TIME_RELATIVE;
			p_sub->defer_val_period.time.rel_time.rel_time = (sms_relative_time_t)p_pkg_str[offset++];
		} else if (p_pkg_str[offset] == 0x08) {
			/* Priority indicator */
			offset += 2;
			p_sub->priority = (sms_priority_indicator_t)((p_pkg_str[offset++] & 0xc0) >> 6);
		} else if (p_pkg_str[offset] == 0x09) {
			/* Privacy indicator */
			offset += 2;
			p_sub->privacy = (sms_privacy_indicator_t)((p_pkg_str[offset++] & 0xc0) >> 6);
		} else if (p_pkg_str[offset] == 0x0a) {
			/* Reply Option */
			offset += 2;

			if (p_pkg_str[offset] & 0x80)
				p_sub->reply_opt.user_ack_req = true;
			else
				p_sub->reply_opt.user_ack_req = false;

			if (p_pkg_str[offset] & 0x40)
				p_sub->reply_opt.deliver_ack_req = true;
			else
				p_sub->reply_opt.deliver_ack_req = false;

			if (p_pkg_str[offset] & 0x20)
				p_sub->reply_opt.read_ack_req = true;
			else
				p_sub->reply_opt.read_ack_req = false;

			if (p_pkg_str[offset] & 0x10)
				p_sub->reply_opt.report_req = true;
			else
				p_sub->reply_opt.report_req = false;

			offset++;
		} else if (p_pkg_str[offset] == 0x0c) {
			/* Alert on Message Delivery */
			offset += 2;

			p_sub->alert_priority = (sms_alert_priority_t)((p_pkg_str[offset++] & 0xc0) >> 6);
		} else if (p_pkg_str[offset] == 0x0d) {
			/* Language Indicator */
			offset += 2;

			p_sub->language = (sms_language_type_t)p_pkg_str[offset++];
		} else if (p_pkg_str[offset] == 0x0e) {
			/* Call-Back Number */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			decodeCallBackNum(&p_pkg_str[offset], tmp_len, &(p_sub->callback_number));

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x10) {
			/* Multiple Encoding User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			/* TODO */

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x11) {
			/* Message Deposit Index */
			offset += 2;

			memset(&tmp_param_s, 0x00, sizeof(unsigned short));
			_copy_char_to_short(&tmp_param_s, p_pkg_str+offset);
			p_sub->deposit_id = tmp_param_s;

			offset += 2;
		}
	}
}


void SmsPluginMsgCodec::decodeP2PUserAckMsg(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_user_ack_s *p_user_ack)
{
	MSG_BEGIN();

	int offset = 0, tmp_len;
	unsigned short tmp_param_s;
	unsigned char tmp_str[pkg_len+1];

	while (offset < pkg_len) {
		if (p_pkg_str[offset] == 0x00) {
			/* Message Identifier */
			offset += decodeMsgId(p_pkg_str+offset, 5, &(p_user_ack->msg_id));
		} else if (p_pkg_str[offset] == 0x01) {
			/* User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);

			decodeUserData(tmp_str, tmp_len, &(p_user_ack->user_data));

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x02) {
			/* User Response Code */
			offset += 2;
			p_user_ack->resp_code = p_pkg_str[offset++];
		} else if (p_pkg_str[offset] == 0x03) {
			/* Message Center Time Stamp */
			offset += 2;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_user_ack->time_stamp));
		} else if (p_pkg_str[offset] == 0x10) {
			/* Multiple Encoding User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			/* TODO */

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x11) {
			/* Message Deposit Index */
			offset += 2;

			memset(&tmp_param_s, 0x00, sizeof(unsigned short));
			_copy_char_to_short(&tmp_param_s, p_pkg_str+offset);
			p_user_ack->deposit_id = tmp_param_s;

			offset += 2;
		}
	}

	MSG_END();
}


void SmsPluginMsgCodec::decodeP2PReadAckMsg(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_read_ack_s *p_read_ack)
{
	MSG_BEGIN();

	int offset = 0, tmp_len;
	unsigned short tmp_param_s;
	unsigned char tmp_str[pkg_len+1];

	while (offset < pkg_len) {
		if (p_pkg_str[offset] == 0x00) {
			/* Message Identifier */
			offset += decodeMsgId(p_pkg_str+offset, 5, &(p_read_ack->msg_id));
		} else if (p_pkg_str[offset] == 0x01) {
			/* User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);

			decodeUserData(tmp_str, tmp_len, &(p_read_ack->user_data));

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x03) {
			/* Message Center Time Stamp */
			offset += 2;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_read_ack->time_stamp));
		} else if (p_pkg_str[offset] == 0x10) {
			/* Multiple Encoding User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			/* TODO */

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x11) {
			/* Message Deposit Index */
			offset += 2;

			memset(&tmp_param_s, 0x00, sizeof(unsigned short));
			_copy_char_to_short(&tmp_param_s, p_pkg_str+offset);
			p_read_ack->deposit_id = tmp_param_s;

			offset += 2;
		}
	}

	MSG_END();
}


void SmsPluginMsgCodec::decodeP2PSubmitReportMsg(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_report_s *p_sub_report)
{
	MSG_BEGIN();

	int offset = 0, tmp_len;
	unsigned char tmp_str[pkg_len+1];

	while (offset < pkg_len) {
		switch (p_pkg_str[offset]) {
		case 0x00:
			/* Message Identifier */
			offset += decodeMsgId(p_pkg_str+offset, 5, &(p_sub_report->msg_id));
			break;
		case 0x01:
			/* User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);
			decodeUserData(tmp_str, tmp_len, &(p_sub_report->user_data));
			offset += tmp_len;
			break;
		case 0x0d:
			/* Language Indicator */
			offset += 2;
			p_sub_report->language = (sms_language_type_t)p_pkg_str[offset++];
			break;
		case 0x10:
			/* Multiple Encoding User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			/* TODO */

			offset += tmp_len;
			break;
		case 0x15:
			/* TP-Failure Cause */
			offset += 2;
			p_sub_report->tp_fail_cause = p_pkg_str[offset++];
			break;
		default:
			break;
		}
	}

	MSG_END();
}


void SmsPluginMsgCodec::decodeP2PDeliveryAckMsg(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_deliver_ack_s *p_del_ack)
{
	MSG_BEGIN();

	int offset = 0, tmp_len;
	unsigned char tmp_str[pkg_len+1];

	while (offset < pkg_len) {
		switch (p_pkg_str[offset]) {
		case 0x00:
			/* Message Identifier */
			offset += decodeMsgId(p_pkg_str+offset, 5, &(p_del_ack->msg_id));
			break;
		case 0x01:
			/* User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);

			decodeUserData(tmp_str, tmp_len, &(p_del_ack->user_data));

			offset += tmp_len;
			break;
		case 0x03:
			/* Message Center Time Stamp */
			offset += 2;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_del_ack->time_stamp));
			break;
		case 0x10:
			/* Multiple Encoding User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			/* TODO */

			offset += tmp_len;
			break;
		case 0x14:
			/* Message Status */
			offset += 2;
			p_del_ack->msg_status = (sms_status_code_t)p_pkg_str[offset++];
			break;
		default:
			break;
		}
	}

	MSG_END();
}


void SmsPluginMsgCodec::decodeCBBearerData(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_msg_s *p_telesvc, bool isCMAS)
{
	MSG_BEGIN();

	int offset = 0, tmp_len;
	unsigned char tmp_str[pkg_len+1];

	while (offset < pkg_len) {
		if (p_pkg_str[offset] == 0x00) {
			/* Message Identifier */
			p_telesvc->type = SMS_TYPE_DELIVER;
			offset += decodeMsgId(p_pkg_str+offset, 5, &(p_telesvc->data.deliver.msg_id));
		} else if (p_pkg_str[offset] == 0x01) {
			/* User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];
			memset(tmp_str, 0x00, sizeof(tmp_str));
			memcpy(tmp_str, p_pkg_str+offset, tmp_len);

			if (isCMAS)
				decodeCMASData(tmp_str, tmp_len, &(p_telesvc->data.deliver.cmas_data));
			else
				decodeUserData(tmp_str, tmp_len, &(p_telesvc->data.deliver.user_data));

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x03) {
			/* Message Center Time Stamp */
			offset += 2;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_telesvc->data.deliver.time_stamp));
		} else if (p_pkg_str[offset] == 0x04) {
			/* Validity Period - Absolute */
			offset += 2;
			p_telesvc->data.deliver.val_period.format = SMS_TIME_ABSOLUTE;
			offset += decodeAbsTime(p_pkg_str+offset, &(p_telesvc->data.deliver.val_period.time.abs_time));
		} else if (p_pkg_str[offset] == 0x05) {
			/* Validity Period - Relative */
			offset += 2;
			p_telesvc->data.deliver.val_period.format = SMS_TIME_RELATIVE;
			p_telesvc->data.deliver.val_period.time.rel_time.rel_time = (sms_relative_time_t)p_pkg_str[offset++];
		} else if (p_pkg_str[offset] == 0x08) {
			/* Priority indicator */
			offset += 2;
			p_telesvc->data.deliver.priority = (sms_priority_indicator_t)((p_pkg_str[offset++] & 0xc0) >> 6);
		} else if (p_pkg_str[offset] == 0x0c) {
			/* Alert on Message Delivery */
			offset += 2;

			p_telesvc->data.deliver.alert_priority = (sms_alert_priority_t)((p_pkg_str[offset++] & 0xc0) >> 6);
		} else if (p_pkg_str[offset] == 0x0d) {
			/* Language Indicator */
			offset += 2;

			p_telesvc->data.deliver.language = (sms_language_type_t)p_pkg_str[offset++];
		} else if (p_pkg_str[offset] == 0x0e) {
			/* Call-Back Number */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			decodeCallBackNum(&p_pkg_str[offset], tmp_len, &(p_telesvc->data.deliver.callback_number));

			offset += tmp_len;
		} else if (p_pkg_str[offset] == 0x0f) {
			/* Message Display Mode */
			offset += 2;

			p_telesvc->data.deliver.display_mode = (sms_display_mode_t)((p_pkg_str[offset++] & 0xc0) >> 6);
		} else if (p_pkg_str[offset] == 0x10) {
			/* Multiple Encoding User Data */
			offset += 2;
			tmp_len = p_pkg_str[offset-1];

			/* TODO */

			offset += tmp_len;
		}
	}

	MSG_END();
}


int SmsPluginMsgCodec::decodeTeleId(const unsigned char *p_pkg_str, int pkg_len, sms_trans_telesvc_id_t *tele_id)
{
	int offset = 0;
	unsigned short tmp_param_s;

	offset += 2;

    _copy_char_to_short(&tmp_param_s, &p_pkg_str[offset]);

	switch (tmp_param_s) {
	case SMS_TRANS_TELESVC_CMT_91:
		*tele_id = SMS_TRANS_TELESVC_CMT_91;
		break;
	case SMS_TRANS_TELESVC_CPT_95:
		*tele_id = SMS_TRANS_TELESVC_CPT_95;
		break;
	case SMS_TRANS_TELESVC_CMT_95:
		*tele_id = SMS_TRANS_TELESVC_CMT_95;
		break;
	case SMS_TRANS_TELESVC_VMN_95:
		*tele_id = SMS_TRANS_TELESVC_VMN_95;
		break;
	case SMS_TRANS_TELESVC_WAP:
		*tele_id = SMS_TRANS_TELESVC_WAP;
		break;
	case SMS_TRANS_TELESVC_WEMT:
		*tele_id = SMS_TRANS_TELESVC_WEMT;
		break;
	case SMS_TRANS_TELESVC_SCPT:
		*tele_id = SMS_TRANS_TELESVC_SCPT;
		break;
	case SMS_TRANS_TELESVC_CATPT:
		*tele_id = SMS_TRANS_TELESVC_CATPT;
		break;
	default:
		*tele_id = SMS_TRANS_TELESVC_RESERVED;
		break;
	}

	return offset+2;
}


int SmsPluginMsgCodec::decodeSvcCtg(const unsigned char *p_pkg_str, int pkg_len, sms_trans_svc_ctg_t *svc_ctg)
{
	int offset = 0;
	unsigned short tmp_param_s;

	offset += 2;

	_copy_char_to_short(&tmp_param_s, &p_pkg_str[offset]);
	if ((tmp_param_s >= SMS_TRANS_SVC_CTG_UNKNOWN && tmp_param_s <= SMS_TRANS_SVC_CTG_KDDI_CORP_MAX1)
		|| (tmp_param_s >= SMS_TRANS_SVC_CTG_KDDI_CORP_MIN2 && tmp_param_s <= SMS_TRANS_SVC_CTG_KDDI_CORP_MAX2)
		|| (tmp_param_s >= SMS_TRANS_SVC_CTG_KDDI_CORP_MIN3 && tmp_param_s <= SMS_TRANS_SVC_CTG_KDDI_CORP_MAX3)) {
		*svc_ctg = (sms_trans_svc_ctg_t)tmp_param_s;
	} else {
		*svc_ctg = SMS_TRANS_SVC_CTG_RESERVED;
	}

	return offset+2;
}


int SmsPluginMsgCodec::decodeAddress(const unsigned char *p_pkg_str, int pkg_len, sms_trans_addr_s *addr)
{
	int offset = 0, tmp_len = 0;
	unsigned char tmp_str[pkg_len+1];

	tmp_len = p_pkg_str[++offset];

	memset(tmp_str, 0x00, sizeof(tmp_str));
	memcpy(tmp_str, p_pkg_str+offset+1, tmp_len);

	offset += (tmp_len+1);

	if (tmp_str[0] & 0x80)
		addr->digit_mode = true;
	else
		addr->digit_mode = false;

	if (tmp_str[0] & 0x40)
		addr->number_mode = true;
	else
		addr->number_mode = false;

	_shiftNBit_for_decode(tmp_str, tmp_len, 2);

	if (addr->digit_mode == false) {
		addr->addr_len = tmp_str[0];

		memset(addr->szData, 0x00, sizeof(addr->szData));

		for (unsigned int i = 0; i < addr->addr_len; i++) {
			switch (tmp_str[1] & 0xf0) {
			case 0x10:
			case 0x20:
			case 0x30:
			case 0x40:
			case 0x50:
			case 0x60:
			case 0x70:
			case 0x80:
			case 0x90:
				addr->szData[i] = ((tmp_str[1] & 0xf0) >> 4) + '0';
				break;
			case 0x00:
				/* sprint issue */
			case 0xa0:
				addr->szData[i] = '0';
				break;
			case 0xb0:
				addr->szData[i] = '*';
				break;
			case 0xc0:
				addr->szData[i] = '#';
				break;
			default:
				break;
			}

			_shiftNBit_for_decode(tmp_str, tmp_len, 4);
		}
	} else if (addr->digit_mode == true) {
		if (addr->number_mode == false) {
			/* digit mode = 1, number mode = 0 */
			switch (tmp_str[0] & 0xe0) {
			case 0x00:
				addr->number_type = SMS_NUMBER_TYPE_UNKNOWN;
				break;
			case 0x20:
				addr->number_type = SMS_NUMBER_TYPE_INTERNATIONAL;
				break;
			case 0x40:
				addr->number_type = SMS_NUMBER_TYPE_NATIONAL;
				break;
			case 0x60:
				addr->number_type = SMS_NUMBER_TYPE_NETWORK_SPECIFIC;
				break;
			case 0x80:
				addr->number_type = SMS_NUMBER_TYPE_SUBSCRIBER;
				break;
			case 0xa0:
				addr->number_type = SMS_NUMBER_TYPE_RESERVED_5;
				break;
			case 0xc0:
				addr->number_type = SMS_NUMBER_TYPE_ABBREVIATED;
				break;
			case 0xe0:
				addr->number_type = SMS_NUMBER_TYPE_RESERVED_7;
				break;
			}

			_shiftNBit_for_decode(tmp_str, tmp_len, 3);

			switch (tmp_str[0] & 0xf0) {
			case 0x00:
				addr->number_plan = SMS_NPI_UNKNOWN;
				break;
			case 0x10:
				addr->number_plan = SMS_NPI_ISDN;
				break;
			case 0x30:
				addr->number_plan = SMS_NPI_DATA;
				break;
			case 0x40:
				addr->number_plan = SMS_NPI_TELEX;
				break;
			case 0x90:
				addr->number_plan = SMS_NPI_PRIVATE;
				break;
			default:
				addr->number_plan = SMS_NPI_RESERVED;
				break;
			}

			_shiftNBit_for_decode(tmp_str, tmp_len, 4);
		} else if (addr->number_mode == true) {
			/* digit mode = 1, number mode = 1 */
			switch (tmp_str[0] & 0xe0) {
			case 0x00:
				addr->number_type = SMS_TRANS_DNET_UNKNOWN;
				break;
			case 0x20:
				addr->number_type = SMS_TRANS_DNET_INTERNET_PROTOCOL;
				break;
			case 0x40:
				addr->number_type = SMS_TRANS_DNET_INTERNET_MAIL_ADDR;
				break;
			default:
				addr->number_type = SMS_TRANS_DNET_RESERVED;
				break;
			}

			_shiftNBit_for_decode(tmp_str, tmp_len, 3);
		}

		addr->addr_len = tmp_str[0];

		memset(addr->szData, 0x00, sizeof(addr->szData));
		memcpy(addr->szData, &tmp_str[1], addr->addr_len);
	}

	return offset;
}


int SmsPluginMsgCodec::decodeSubAddress(const unsigned char *p_pkg_str, int pkg_len, sms_trans_sub_addr_s *sub_addr)
{
	int offset = 0, tmp_len = 0;
	unsigned char tmp_str[pkg_len+1];

	tmp_len = p_pkg_str[++offset];
	memset(tmp_str, 0x00, sizeof(tmp_str));
	memcpy(tmp_str, p_pkg_str+offset+1, tmp_len);

	offset += (tmp_len+1);

	switch (tmp_str[0] & 0xe0) {
	case 0x00:
		sub_addr->type = SMS_TRANS_SUB_ADDR_NSAP;
		break;
	case 0x20:
		sub_addr->type = SMS_TRANS_SUB_ADDR_USER;
		break;
	default:
		sub_addr->type = SMS_TRANS_SUB_ADDR_RESERVED;
		break;
	}

	if (tmp_str[0] & 0x10)
		sub_addr->odd = true;
	else
		sub_addr->odd = false;

	_shiftNBit_for_decode(tmp_str, tmp_len, 4);
	memset(sub_addr->szData, 0x00, sizeof(sub_addr->szData));
	memcpy(sub_addr->szData, tmp_str+1, tmp_str[0]);

	return offset;
}


int SmsPluginMsgCodec::decodeMsgId(const unsigned char *p_pkg_str, int pkg_len, sms_trans_msg_id_s *p_msg_id)
{
	int offset = 0;
	unsigned short tmp_param_s;
	unsigned char tmp_str[pkg_len+1];

	memset(tmp_str, 0x00, sizeof(tmp_str));
	memcpy(tmp_str, &p_pkg_str[offset+2], 3);

	_shiftNBit_for_decode(tmp_str, 3, 4);

	memset(&tmp_param_s, 0x00, sizeof(unsigned short));
	_copy_char_to_short(&tmp_param_s, tmp_str);

	p_msg_id->msg_id = tmp_param_s;
	if (tmp_str[2] & 0x80)
		p_msg_id->header_ind = true;
	else
		p_msg_id->header_ind = false;

	offset += 5;

	return offset;
}


void SmsPluginMsgCodec::decodeCallBackNum(const unsigned char *p_pkg_str, int pkg_len, sms_telesvc_addr_s *p_callback)
{
	int offset = 0;
	unsigned char tmp_str[pkg_len+1];

	if (p_pkg_str[offset] & 0x80) {
		p_callback->digit_mode = true;

		switch (p_pkg_str[offset] & 0x70) {
		case 0x00:
			p_callback->number_type = SMS_NUMBER_TYPE_UNKNOWN;
			break;
		case 0x10:
			p_callback->number_type = SMS_NUMBER_TYPE_INTERNATIONAL;
			break;
		case 0x20:
			p_callback->number_type = SMS_NUMBER_TYPE_NATIONAL;
			break;
		case 0x30:
			p_callback->number_type = SMS_NUMBER_TYPE_NETWORK_SPECIFIC;
			break;
		case 0x40:
			p_callback->number_type = SMS_NUMBER_TYPE_SUBSCRIBER;
			break;
		case 0x50:
			p_callback->number_type = SMS_NUMBER_TYPE_RESERVED_5;
			break;
		case 0x60:
			p_callback->number_type = SMS_NUMBER_TYPE_ABBREVIATED;
			break;
		case 0x70:
			p_callback->number_type = SMS_NUMBER_TYPE_RESERVED_7;
			break;
		default:
			break;
		}

		switch (p_pkg_str[offset++] & 0x0f) {
		case 0x00:
			p_callback->number_plan = SMS_NPI_UNKNOWN;
			break;
		case 0x01:
			p_callback->number_plan = SMS_NPI_ISDN;
			break;
		case 0x03:
			p_callback->number_plan = SMS_NPI_DATA;
			break;
		case 0x04:
			p_callback->number_plan = SMS_NPI_TELEX;
			break;
		case 0x09:
			p_callback->number_plan = SMS_NPI_PRIVATE;
			break;
		case 0x0f:
		default:
			p_callback->number_plan = SMS_NPI_RESERVED;
			break;
		}

		p_callback->addr_len = p_pkg_str[offset++];
		memset(p_callback->szData, 0x00, sizeof(p_callback->szData));

		if (p_callback->number_type == SMS_NUMBER_TYPE_INTERNATIONAL) {
			memcpy(&(p_callback->szData[1]), p_pkg_str+offset, p_callback->addr_len);
			if (p_callback->szData[1] != '\0') {
				p_callback->szData[0] = '+';
			}
		} else {
			memcpy(p_callback->szData, p_pkg_str+offset, p_callback->addr_len);
		}
	} else {
		p_callback->digit_mode = false;

		memset(tmp_str, 0x00, sizeof(tmp_str));
		memcpy(tmp_str, p_pkg_str+offset, pkg_len);

		_shiftNBit_for_decode(tmp_str, pkg_len, 1);

		p_callback->addr_len = tmp_str[0];

		memset(p_callback->szData, 0x00, sizeof(p_callback->szData));

		for (unsigned int i = 0; i < p_callback->addr_len; i++) {
			switch (tmp_str[1] & 0xf0) {
			case 0x10:
			case 0x20:
			case 0x30:
			case 0x40:
			case 0x50:
			case 0x60:
			case 0x70:
			case 0x80:
			case 0x90:
				p_callback->szData[i] = ((tmp_str[1] & 0xf0) >> 4) + '0';
				break;
			case 0xa0:
				p_callback->szData[i] = '0';
				break;
			case 0xb0:
				p_callback->szData[i] = '*';
				break;
			case 0xc0:
				p_callback->szData[i] = '#';
				break;
			default :
				break;
			}

			_shiftNBit_for_decode(tmp_str, pkg_len, 4);
		}
	}
}


int SmsPluginMsgCodec::decodeAbsTime(const unsigned char *p_pkg_str, sms_time_abs_s *p_time_abs)
{
	int offset = 0;

	p_time_abs->year = (((p_pkg_str[offset] & 0xf0) >> 4) * 10) + (p_pkg_str[offset] & 0x0f);
	offset++;
	p_time_abs->month = (((p_pkg_str[offset] & 0xf0) >> 4) * 10) + (p_pkg_str[offset] & 0x0f);
	offset++;
	p_time_abs->day = (((p_pkg_str[offset] & 0xf0) >> 4) * 10) + (p_pkg_str[offset] & 0x0f);
	offset++;
	p_time_abs->hours = (((p_pkg_str[offset] & 0xf0) >> 4) * 10) + (p_pkg_str[offset] & 0x0f);
	offset++;
	p_time_abs->minutes = (((p_pkg_str[offset] & 0xf0) >> 4) * 10) + (p_pkg_str[offset] & 0x0f);
	offset++;
	p_time_abs->seconds = (((p_pkg_str[offset] & 0xf0) >> 4) * 10) + (p_pkg_str[offset] & 0x0f);
	offset++;

	return offset;
}


int SmsPluginMsgCodec::encodeUserData(const unsigned char* src, unsigned char *dest, int src_size)
{
	int i, j;
	int shift = 0;

	unsigned char *tmp = (unsigned char *)calloc(1, src_size+1);
	for (i = 0; i < src_size; i++) {
		tmp[i] = src[i] << 1;
	}

	j = 0;
	for (i = 0; i < src_size; i++) {
		shift = j % 7;
		dest[j++] = (tmp[i] << shift) + (tmp[i+1] >> (7-shift));
		if (shift == 6) {
			i++;
		}
	}

	if (tmp) {
		free(tmp);
		tmp = NULL;
	}

	return j;
}


void SmsPluginMsgCodec::decodeCMASData(unsigned char *p_pkg_str, int pkg_len, sms_telesvc_cmasdata_s *p_cmas)
{
	MSG_BEGIN();

	int offset = 0, tmp_len = 0;
	unsigned char tmp_str[pkg_len+1];

	if ((p_pkg_str[offset] & 0xf8) != 0x00) {
		MSG_ERR("Wrong Encode Type = [%d]!! The type must be 0", (p_pkg_str[offset]&0xf8)>>3);
		return;
	} else {
		_shiftNBit_for_decode(p_pkg_str, pkg_len, 5);

		offset++;

		if (p_pkg_str[offset++] != 0x00) {
			MSG_ERR("Wrong protocol version = [%d]!! This field must be 0", p_pkg_str[offset-1]);
			p_cmas->is_wrong_recode_type  = TRUE;
			return;
		}

		while (offset < pkg_len - 1) {
			if (p_pkg_str[offset] == 0x00) {
				MSG_DEBUG("Type 0 Decode!");
				offset++;
				tmp_len = p_pkg_str[offset++];
				MSG_DEBUG("Type 0 length = [%d]", tmp_len);
				memset(tmp_str, 0x00, sizeof(tmp_str));
				memcpy(tmp_str, p_pkg_str+offset, tmp_len);

				switch (tmp_str[0] & 0xf8) {
				case 0x00:
					p_cmas->encode_type = SMS_ENCODE_OCTET;
					break;
				case 0x08:
					p_cmas->encode_type = SMS_ENCODE_EPM;
					break;
				case 0x10:
					p_cmas->encode_type = SMS_ENCODE_7BIT_ASCII;
					break;
				case 0x18:
					p_cmas->encode_type = SMS_ENCODE_IA5;
					break;
				case 0x20:
					p_cmas->encode_type = SMS_ENCODE_UNICODE;
					break;
				case 0x28:
					p_cmas->encode_type = SMS_ENCODE_SHIFT_JIS;
					break;
				case 0x30:
					p_cmas->encode_type = SMS_ENCODE_KOREAN;
					break;
				case 0x38:
					p_cmas->encode_type = SMS_ENCODE_LATIN_HEBREW;
					break;
				case 0x40:
					p_cmas->encode_type = SMS_ENCODE_LATIN;
					break;
				case 0x48:
					p_cmas->encode_type = SMS_ENCODE_GSM7BIT;
					break;
				case 0x50:
					p_cmas->encode_type = SMS_ENCODE_GSMDCS;
					break;
				case 0x80:
					/* reserved value, but SKT use this value for KSC5601 */
					p_cmas->encode_type = SMS_ENCODE_EUCKR;
					break;
				default :
					p_cmas->encode_type = SMS_ENCODE_RESERVED;
					break;
				}
				_shiftNBit_for_decode(tmp_str, tmp_len, 5);

				switch (p_cmas->encode_type) {
				case SMS_ENCODE_7BIT_ASCII:
				case SMS_ENCODE_IA5:
				case SMS_ENCODE_GSM7BIT:
					memset(p_cmas->alert_text, 0x00, sizeof(p_cmas->alert_text));
					p_cmas->data_len = (tmp_len*8-5) / 7;
					for (unsigned int i = 0; i < p_cmas->data_len; i++) {
						p_cmas->alert_text[i] = tmp_str[0] >> 1;
						_shiftNBit_for_decode(tmp_str, tmp_len, 7);
					}
					break;
				case SMS_ENCODE_EPM:
					break;
				case SMS_ENCODE_GSMDCS:
					break;
				default:
					p_cmas->data_len = tmp_len - 1;
					memset(p_cmas->alert_text, 0x00, sizeof(p_cmas->alert_text));
					memcpy(p_cmas->alert_text, tmp_str+offset, tmp_len-1);
					break;
				}

				offset += tmp_len;
			} else if (p_pkg_str[offset] == 0x01) {
				MSG_DEBUG("Type 1 Decode!");
				offset += 2;
				tmp_len = p_pkg_str[offset-1];
				MSG_DEBUG("Type 1 length = [%d]", tmp_len);
				p_cmas->category = (sms_cmae_category_t)p_pkg_str[offset++];
				p_cmas->response_type = (sms_cmae_response_type_t)p_pkg_str[offset++];
				p_cmas->severity = (sms_cmae_severity_t)(p_pkg_str[offset] >> 4);
				p_cmas->urgency = (sms_cmae_urgency_t)(p_pkg_str[offset++] & 0x0f);
				p_cmas->certainty = (sms_cmae_certainty_t)(p_pkg_str[offset++] >> 4);
			} else if (p_pkg_str[offset] == 0x02) {
				MSG_DEBUG("Type 2 Decode!");
				offset += 2;
				tmp_len = p_pkg_str[offset-1];
				MSG_DEBUG("Type 2 length = [%d]", tmp_len);
				_copy_char_to_short(&(p_cmas->id), p_pkg_str+offset);
				offset += 2;
				p_cmas->alert_handle = (sms_cmae_alert_handle_t)p_pkg_str[offset++];
				offset += decodeAbsTime(p_pkg_str+offset, &(p_cmas->expires));
				p_cmas->language = (sms_language_type_t)p_pkg_str[offset++];
			}

			MSG_DEBUG("offset = [%d], pkg_len = [%d]", offset, pkg_len);
		}
	}

	MSG_END();
}


void SmsPluginMsgCodec::decodeUserData(unsigned char *p_pkg_str, int pkg_len, sms_telesvc_userdata_s *p_user)
{
	switch (p_pkg_str[0] & 0xf8) {
	case 0x00:
		p_user->encode_type = SMS_ENCODE_OCTET;
		break;
	case 0x08:
		p_user->encode_type = SMS_ENCODE_EPM;
		break;
	case 0x10:
		p_user->encode_type = SMS_ENCODE_7BIT_ASCII;
		break;
	case 0x18:
		p_user->encode_type = SMS_ENCODE_IA5;
		break;
	case 0x20:
		p_user->encode_type = SMS_ENCODE_UNICODE;
		break;
	case 0x28:
		p_user->encode_type = SMS_ENCODE_SHIFT_JIS;
		break;
	case 0x30:
		p_user->encode_type = SMS_ENCODE_KOREAN;
		break;
	case 0x38:
		p_user->encode_type = SMS_ENCODE_LATIN_HEBREW;
		break;
	case 0x40:
		p_user->encode_type = SMS_ENCODE_LATIN;
		break;
	case 0x48:
		p_user->encode_type = SMS_ENCODE_GSM7BIT;
		break;
	case 0x50:
		p_user->encode_type = SMS_ENCODE_GSMDCS;
		break;
	case 0x80:
		/* reserved value, but SKT use this value for KSC5601 */
		p_user->encode_type = SMS_ENCODE_EUCKR;
		break;
	default :
		p_user->encode_type = SMS_ENCODE_RESERVED;
		break;
	}

	_shiftNBit_for_decode(p_pkg_str, pkg_len, 5);

	if (p_user->encode_type == SMS_ENCODE_EPM || p_user->encode_type == SMS_ENCODE_GSMDCS) {
		p_user->msg_type = p_pkg_str[0];
		_shiftNBit_for_decode(p_pkg_str, pkg_len, 8);
	}

	p_user->data_len = p_pkg_str[0];
	switch (p_user->encode_type) {
	case SMS_ENCODE_7BIT_ASCII:
	case SMS_ENCODE_IA5:
		memset(p_user->user_data, 0x00, sizeof(p_user->user_data));
		for (unsigned int i = 0; i < p_user->data_len; i++) {
			p_user->user_data[i] = p_pkg_str[1] >> 1;
			_shiftNBit_for_decode(p_pkg_str, pkg_len, 7);
		}
		break;
	case SMS_ENCODE_GSM7BIT:
		memset(p_user->user_data, 0x00, sizeof(p_user->user_data));
		UnpackGSM7bitData(&(p_pkg_str[1]), p_user->user_data, p_user->data_len);
		break;
	case SMS_ENCODE_EPM:
		break;
	case SMS_ENCODE_GSMDCS:
		break;
	case SMS_ENCODE_UNICODE:
		p_user->data_len*=2;
		memset(p_user->user_data, 0x00, sizeof(p_user->user_data));
		memcpy(p_user->user_data, p_pkg_str+1, p_user->data_len);
		break;
	default:
		memset(p_user->user_data, 0x00, sizeof(p_user->user_data));
		memcpy(p_user->user_data, p_pkg_str+1, p_user->data_len);
		break;
	}
}


sms_message_type_t SmsPluginMsgCodec::findMsgType(const unsigned char *p_pkg_str, int pkg_len)
{
	int offset = 0;
	while (offset < pkg_len) {
		if (p_pkg_str[offset] == 0x00) {
			return (sms_message_type_t)((p_pkg_str[offset+2]&0xf0)>>4);
		}
		offset += (p_pkg_str[offset+1]+2);
	}

	return SMS_TYPE_MAX_VALUE;
}
