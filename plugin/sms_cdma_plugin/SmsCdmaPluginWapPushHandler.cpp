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

#include "MsgDebug.h"
#include "MsgCppTypes.h"
#include "MsgGconfWrapper.h"
#include "MsgException.h"
#include "MsgUtilFile.h"
#include "SmsCdmaPluginStorage.h"
#include "SmsCdmaPluginTransport.h"
#include "SmsCdmaPluginEventHandler.h"
#include "SmsCdmaPluginWapPushHandler.h"

#include <glib.h>
#include <gio/gio.h>
#if MSG_DRM_SUPPORT
#include <drm_client.h>
#endif

static unsigned short wapPushPortList[] = {0x0b84, 0x0b85, 0x23F0, 0x23F1, 0x23F2, 0x23F3, 0xC34F};

char gWapCodeBufferLeft[WSP_CODE_BUFFER_LEFT_LEN_MAX];
char gWapCodeBufferRight[WSP_CODE_BUFFER_RIGHT_LEN_MAX];

const unsigned long wspUintvarDecodeTable[] = { 0x00000001, 0x00000080, 0x00004000, 0x00200000, 0x10000000};

const unsigned char wspHeaderFieldCount  = 0x43;
const unsigned char wspContentsTypeCount = 0x34;
const unsigned long wspLanguageCount    = 0x11a;
const unsigned char wspSecurityTypeCount = 0x04;


static const SMS_WSP_CONTENTS_TYPE_S wspExtendedContentsType[] = {
	{ (char*)"text/vnd/wap/connectivity-xml", 0x35 },
	{ (char*)"application/vnd.wap.connectivity-wbxml", 0x36 },
	{ (char*)"application/pkcs7-mime",  0x37 },
	{ (char*)"application/vnd.wap.hashed-certificate", 0x38 },
	{ (char*)"application/vnd.wap.signed-certificate", 0x39  },
	{ (char*)"application/vnd.wap.cert-response", 0x3A },
	{ (char*)"application/xhtml+xml", 0x3B },
	{ (char*)"application/wml+xml", 0x3C  },
	{ (char*)"text/css", 0x3D },

	{ (char*)"application/vnd.wap.mms-message", 0x3E },

	{ (char*)"application/vnd.wap.rollover-certificate", 0x3F  },
	{ (char*)"application/vnd.wap.locc+wbxml", 0x40 },
	{ (char*)"application/vnd.wap.loc+xml", 0x41 },
	{ (char*)"application/vnd.syncml.dm+wbxml", 0x42 },
	{ (char*)"application/vnd.syncml.dm+xml", 0x43 },
	{ (char*)"application/vnd.syncml.notification", 0x44 },
	{ (char*)"application/vnd.wap.xhtml+xml", 0x45 },
	{ (char*)"application/vnd.wv.csp.cir", 0x46 },

	{ (char*)"application/vnd.oma.dd+xml", 0x47},
	{ (char*)"application/vnd.oma.drm.message", 0x48 },
	{ (char*)"application/vnd.oma.drm.content", 0x49 },
	{ (char*)"application/vnd.oma.drm.rights+xml", 0x4A },
	{ (char*)"application/vnd.oma.drm.rights+wbxml", 0x4B },
	{ (char*)"application/vnd.syncml.ds.notification", 0x4E},
	{ (char*)"application/mikey", 0x52},
	{ (char*)"", 0xff }
};


const char* wspHeaderFieldName[] = {
	(char*)"Accept", /* 0x00 */
	(char*)"Accept-Charset",
	(char*)"Accept-Encoding",
	(char*)"Accept-Language",
	(char*)"Accept-Ranges",
	(char*)"Age", /* 0x05 */
	(char*)"Allow",
	(char*)"Authorization",
	(char*)"Cache-Control",
	(char*)"Connection",
	(char*)"Content-Base", /* 0x0a */
	(char*)"Content-Encoding",
	(char*)"Content-Language",
	(char*)"Content-Length",
	(char*)"Content-Location",
	(char*)"Content-MD5",
	(char*)"Content-Range", /* 0x10 */
	(char*)"Content-Type",
	(char*)"Date",
	(char*)"ETag",
	(char*)"Expires",
	(char*)"From", /* 0x15 */
	(char*)"Host",
	(char*)"If-Modified-Since",
	(char*)"If-Match",
	(char*)"If-None-Match",
	(char*)"If-Range", /* 0x1a */
	(char*)"If-Unmodified-Since",
	(char*)"Location",
	(char*)"Last-Modified",
	(char*)"Max-Forwards",
	(char*)"Pragma",
	(char*)"Proxy-Authenticate", /* 0x20 */
	(char*)"Proxy-Authorization",
	(char*)"Public",
	(char*)"Range",
	(char*)"Referer",
	(char*)"Retry-After", /* 0x25 */
	(char*)"Server",
	(char*)"Transfer-Encodig",
	(char*)"Upgrade",
	(char*)"User-Agent",
	(char*)"Vary", /* 0x2a */
	(char*)"Via",
	(char*)"Warning",
	(char*)"Www-Authenticate",
	(char*)"Content-Disposition",
	(char*)"X-Wap-Application-Id",
	(char*)"X-Wap-Content-URI", /* 0x30 */
	(char*)"X-Wap-Iinitiator-URI",
	(char*)"Accept-Application", /* Back */
	(char*)"Bearer-Indication",
	(char*)"Push-Flag",
	(char*)"Profile", /* 0x35 */
	(char*)"Profile-Diff",
	(char*)"Profile-Warning", /* end of WAP 1.2 */
	(char*)"Expect",
	(char*)"Te",
	(char*)"Trailer", /* 0x3a */
	(char*)"Accept-Charset", /* Back */
	(char*)"Accept-Encoding", /* Back */
	(char*)"Cache-Control", /* back */
	(char*)"Content-Range",
	(char*)"X-Wap-Tod",
	(char*)"Content-ID", /*x40 */
	(char*)"Set-Cookie",
	(char*)"Cookie",
	(char*)"Encoding-Version"
};


const SMS_WSP_CHARSET_S wspCharset[] = {
	{ (char*)"big5", 0x07ea },
	{ (char*)"iso-10646-ucs-2", 0x03e8 },
	{ (char*)"iso-8859-1", 0x04 },
	{ (char*)"iso-8859-2", 0x05 },
	{ (char*)"iso-8859-3", 0x06 },
	{ (char*)"iso-8859-4", 0x07 },
	{ (char*)"iso-8859-5", 0x08 },
	{ (char*)"iso-8859-6", 0x09 },
	{ (char*)"iso-8859-7", 0x0a },
	{ (char*)"iso-8859-8", 0x0b },
	{ (char*)"iso-8859-9", 0x0c },
	{ (char*)"shift-JIS", 0x11 },
	{ (char*)"us-ascii", 0x03 },
	{ (char*)"utf-8", 0x6a },
	{ (char*)"none", 0x26 },
	{ (char*)"", 0xffff }
};


const char* wspEncodeMethod[] = {
	(char*)"Gzip",
	(char*)"Compress",
	(char*)"Deflate"
};


const SMS_WSP_CONTENTS_TYPE_S wspContentsType[] = {
	{ (char*)"*/*", 0x00 },
	{ (char*)"text/*", 0x01 },
	{ (char*)"text/html", 0x02 },
	{ (char*)"text/plain", 0x03 },
	{ (char*)"text/x-hdml", 0x04 },
	{ (char*)"text/x-ttml", 0x05 },
	{ (char*)"text/x-vCalendar", 0x06 },
	{ (char*)"text/x-vCard", 0x07 },
	{ (char*)"text/vnd.wap.wml", 0x08 },
	{ (char*)"text/vnd.wap.wmlscript", 0x09 },
	{ (char*)"text/vnd.wap.wta-event", 0x0a },
	{ (char*)"multipart/*", 0x0b },
	{ (char*)"multipart/mixed", 0x0c },
	{ (char*)"multipart/form-data", 0x0d },
	{ (char*)"multipart/byteranges", 0x0e },
	{ (char*)"multipart/alternative", 0x0f },
	{ (char*)"application/*", 0x10 },
	{ (char*)"application/java-vm", 0x11 },
	{ (char*)"application/x-www-form-urlencoded", 0x12 },
	{ (char*)"application/x-hdmlc", 0x13 },
	{ (char*)"application/vnd.wap.wmlc", 0x14 },
	{ (char*)"application/vnd.wap.wmlscriptc", 0x15 },
	{ (char*)"application/vnd.wap.wta-eventc", 0x16 },
	{ (char*)"application/vnd.wap.uaprof", 0x17 },
	{ (char*)"application/vnd.wap.wtls-ca-certificate", 0x18 },
	{ (char*)"application/vnd.wap.wtls-user-certificate", 0x19 },
	{ (char*)"application/x-x509-ca-cert", 0x1a },
	{ (char*)"application/x-x509-user-cert", 0x1b },
	{ (char*)"image/*", 0x1c },
	{ (char*)"image/gif", 0x1d },
	{ (char*)"image/jpeg", 0x1e },
	{ (char*)"image/tiff", 0x1f },
	{ (char*)"image/png", 0x20 },
	{ (char*)"image/vnd.wap.wbmp", 0x21 },
	{ (char*)"application/vnd.wap.multipart.*", 0x22 },
	{ (char*)"application/vnd.wap.multipart.mixed", 0x23 },
	{ (char*)"application/vnd.wap.multipart.form-data", 0x24 },
	{ (char*)"application/vnd.wap.multipart.byteranges", 0x25 },
	{ (char*)"application/vnd.wap.multipart.alternative", 0x26 },
	{ (char*)"application/xml", 0x27 },
	{ (char*)"text/xml", 0x28 },
	{ (char*)"application/vnd.wap.wbxml", 0x29 },
	{ (char*)"application/x-x968-cross-cert", 0x2a },
	{ (char*)"application/x-x968-ca-cert", 0x2b },
	{ (char*)"application/x-x968-user-cert", 0x2c },
	{ (char*)"text/vnd.wap.si", 0x2d },
	{ (char*)"application/vnd.wap.sic", 0x2e },
	{ (char*)"text/vnd.wap.sl", 0x2f },
	{ (char*)"application/vnd.wap.slc", 0x30 },
	{ (char*)"text/vnd.wap.co", 0x31 },
	{ (char*)"application/vnd.wap.coc", 0x32 },
	{ (char*)"application/vnd.wap.multipart.related", 0x33 },
	{ (char*)"application/vnd.wap.sia", 0x34 },

	{ (char*)"text/vnd/wap/connectivity-xml", 0x35 },
	{ (char*)"application/vnd.connectivity-wbxml", 0x36 },
	{ (char*)"application/pkcs7-mime",  0x37 },
	{ (char*)"application/vnd.wap.hashed-certificate", 0x38 },
	{ (char*)"application/vnd.wap.signed-certificate", 0x39  },
	{ (char*)"application/vnd.wap.cert-response", 0x3A },
	{ (char*)"application/xhtml+xml", 0x3B },
	{ (char*)"application/wml+xml", 0x3C  },
	{ (char*)"text/css", 0x3D },

	{ (char*)"application/vnd.wap.mms-message", 0x3E },

	{ (char*)"application/vnd.wap.rollover-certificate", 0x3F  },
	{ (char*)"application/vnd.wap.locc+wbxml", 0x40 },
	{ (char*)"application/vnd.wap.loc+xml", 0x41 },
	{ (char*)"application/vnd.syncml.dm+wbxml", 0x42 },
	{ (char*)"application/vnd.syncml.dm+xml", 0x43 },
	{ (char*)"application/vnd.syncml.notification", 0x44 },
	{ (char*)"application/vnd.wap.xhtml+xml", 0x45 },
	{ (char*)"application/vnd.wv.csp.cir", 0x46 }
};


static const SMS_WAP_UNREGISTER_CONTENTS_TYPE_S wspUnregisterContentsType[]= {
	{ (char*)"application/vnd.wap.emn+wbxml", 0x30A},
	{ (char*)"application/vnd.omaloc-supl-init", 0x312},
	{ (char*)"application/vnd.oma.drm.roap-trigger+wbxml", 0x316}
};


const unsigned long wspUnregisteredContentsTypeCount = sizeof(wspUnregisterContentsType)/sizeof(SMS_WAP_UNREGISTER_CONTENTS_TYPE_S);


const SMS_WSP_LANGUAGE_S wspLanguage[] = {
	{ (char*)"English", 0x19 },
	{ (char*)"en", 0x19 },
	{ (char*)"Korean", 0x3d },
	{ (char*)"*", 0x00 },
	{ (char*)"Afar", 0x01 },
	{ (char*)"aa", 0x01 },
	{ (char*)"Abkhazian", 0x02 },
	{ (char*)"ab", 0x02 },
	{ (char*)"Afrikaans", 0x03 },
	{ (char*)"af", 0x03 },
	{ (char*)"Amharic", 0x04 },
	{ (char*)"am", 0x04 },
	{ (char*)"Arabic", 0x05 },
	{ (char*)"ar", 0x05 },
	{ (char*)"Assamese", 0x06 },
	{ (char*)"as", 0x06 },
	{ (char*)"Aymara", 0x07 },
	{ (char*)"ay", 0x07 },
	{ (char*)"Azerbaijani", 0x08 },
	{ (char*)"az", 0x08 },
	{ (char*)"Bashkir", 0x09 },
	{ (char*)"ba", 0x09 },
	{ (char*)"Byelorussian", 0x0a },
	{ (char*)"be", 0x0a },
	{ (char*)"Bulgarian", 0x0b },
	{ (char*)"bg", 0x0b },
	{ (char*)"Bihari", 0x0c },
	{ (char*)"bh", 0x0c },
	{ (char*)"Bislama", 0x0d },
	{ (char*)"bi", 0x0f },
	{ (char*)"Bengali", 0x0e },
	{ (char*)"Bangla", 0x0e },
	{ (char*)"bn", 0x0e },
	{ (char*)"Tibetan", 0x0f },
	{ (char*)"bo", 0x0f },
	{ (char*)"Breton", 0x10 },
	{ (char*)"br", 0x10 },
	{ (char*)"Catalan", 0x11 },
	{ (char*)"ca", 0x11 },
	{ (char*)"Corsican", 0x12 },
	{ (char*)"co", 0x12 },
	{ (char*)"Czech", 0x13 },
	{ (char*)"cs", 0x13 },
	{ (char*)"Welsh", 0x14 },
	{ (char*)"cy", 0x14 },
	{ (char*)"Denish", 0x15 },
	{ (char*)"da", 0x15 },
	{ (char*)"German", 0x16 },
	{ (char*)"de", 0x16 },
	{ (char*)"Bhutani", 0x17 },
	{ (char*)"dz", 0x17 },
	{ (char*)"Greek", 0x18 },
	{ (char*)"el", 0x18 },
	{ (char*)"Esperanto", 0x81 },
	{ (char*)"eo", 0x1a },
	{ (char*)"Spanish", 0x1b },
	{ (char*)"es", 0x1b },
	{ (char*)"Estonian", 0x1c },
	{ (char*)"et", 0x1c },
	{ (char*)"Basque", 0x1d },
	{ (char*)"eu", 0x1d },
	{ (char*)"Persian", 0x1e },
	{ (char*)"fa", 0x1e },
	{ (char*)"Finnish", 0x1f },
	{ (char*)"fi", 0x1f },
	{ (char*)"Fiji", 0x20 },
	{ (char*)"fj", 0x20 },
	{ (char*)"Faeroese", 0x82 },
	{ (char*)"fo", 0x82 },
	{ (char*)"French", 0x22 },
	{ (char*)"fr", 0x22 },
	{ (char*)"Frisian", 0x83 },
	{ (char*)"fy", 0x83 },
	{ (char*)"Irish", 0x24 },
	{ (char*)"ga", 0x24 },
	{ (char*)"Scots Gaelic", 0x25 },
	{ (char*)"gd", 0x25 },
	{ (char*)"Galician", 0x26 },
	{ (char*)"gl", 0x26 },
	{ (char*)"Guarani", 0x27 },
	{ (char*)"gn", 0x27 },
	{ (char*)"Gujarati", 0x28 },
	{ (char*)"gu", 0x28 },
	{ (char*)"Hausa", 0x29 },
	{ (char*)"ha", 0x29 },
	{ (char*)"Hebrew", 0x2a },
	{ (char*)"he", 0x2a },
	{ (char*)"Hindi", 0x2b },
	{ (char*)"hi", 0x2b },
	{ (char*)"Croatian", 0x2c },
	{ (char*)"hr", 0x2c },
	{ (char*)"Hungarian", 0x2d },
	{ (char*)"hu", 0x2d },
	{ (char*)"Armenian", 0x2e },
	{ (char*)"hy", 0x2e },
	{ (char*)"Interlingua", 0x84 },
	{ (char*)"ia", 0x84 },
	{ (char*)"Indonesian", 0x30 },
	{ (char*)"id", 0x30 },
	{ (char*)"Interlingue", 0x86 },
	{ (char*)"ie", 0x86 },
	{ (char*)"Maori", 0x47 },
	{ (char*)"mi", 0x47 },
	{ (char*)"Macedonian", 0x48 },
	{ (char*)"mk", 0x48 },
	{ (char*)"Malayalam", 0x49 },
	{ (char*)"ml", 0x49 },
	{ (char*)"Mongolian", 0x4a },
	{ (char*)"mn", 0x4a },
	{ (char*)"Moldavian", 0x4b },
	{ (char*)"mo", 0x4d },
	{ (char*)"Marathi", 0x4c },
	{ (char*)"mr", 0x4c },
	{ (char*)"Malay", 0x4d },
	{ (char*)"ms", 0x4d },
	{ (char*)"Maltese", 0x4e },
	{ (char*)"mt", 0x4e },
	{ (char*)"Burmese", 0x4f },
	{ (char*)"my", 0x4f },
	{ (char*)"Nauru", 0x50 },
	{ (char*)"na", 0x50 },
	{ (char*)"Nepali", 0x51 },
	{ (char*)"ne", 0x51 },
	{ (char*)"Dutch", 0x52 },
	{ (char*)"nl", 0x52 },
	{ (char*)"Norwegian", 0x53 },
	{ (char*)"no", 0x53 },
	{ (char*)"Occitan", 0x54 },
	{ (char*)"oc", 0x54 },
	{ (char*)"(Afan) Oromo", 0x55 },
	{ (char*)"(Afan)Oromo", 0x55 },
	{ (char*)"om", 0x55 },
	{ (char*)"Oriya", 0x56 },
	{ (char*)"or", 0x56 },
	{ (char*)"Punjabi", 0x57 },
	{ (char*)"pa", 0x57 },
	{ (char*)"Polish", 0x58 },
	{ (char*)"po", 0x58 },
	{ (char*)"Pashto", 0x59 },
	{ (char*)"Pushto", 0x59 },
	{ (char*)"ps", 0x59 },
	{ (char*)"Portugurse", 0x5a },
	{ (char*)"pt", 0x5a },
	{ (char*)"Quechua", 0x5b },
	{ (char*)"qu", 0x5b },
	{ (char*)"Rhaeto-Romance", 0x8c },
	{ (char*)"rm", 0x8c },
	{ (char*)"Kirundi", 0x5d },
	{ (char*)"rn", 0x5d },
	{ (char*)"Romanian", 0x5e },
	{ (char*)"ro", 0x5e },
	{ (char*)"Russian", 0x5f },
	{ (char*)"ru", 0x5f },
	{ (char*)"Kinyarwanda", 0x60 },
	{ (char*)"rw", 0x60 },
	{ (char*)"Sanskrit", 0x61 },
	{ (char*)"sa", 0x61 },
	{ (char*)"Sindhi", 0x62 },
	{ (char*)"sd", 0x62 },
	{ (char*)"Sangho", 0x63 },
	{ (char*)"sg", 0x63 },
	{ (char*)"Serbo-Croatian", 0x64 },
	{ (char*)"sh", 0x64 },
	{ (char*)"Sinhalese", 0x65 },
	{ (char*)"si", 0x65 },
	{ (char*)"Slovak", 0x66 },
	{ (char*)"sk", 0x66 },
	{ (char*)"Slovenian", 0x67 },
	{ (char*)"sl", 0x67 },
	{ (char*)"Samoan", 0x68 },
	{ (char*)"sm", 0x68 },
	{ (char*)"Shona", 0x69 },
	{ (char*)"sn", 0x69 },
	{ (char*)"Somali", 0x6a },
	{ (char*)"so", 0x6a },
	{ (char*)"Albanian", 0x6b },
	{ (char*)"sq", 0x6b },
	{ (char*)"Serbian", 0x6c },
	{ (char*)"sr", 0x6c },
	{ (char*)"Siswati", 0x6d },
	{ (char*)"ss", 0x6d },
	{ (char*)"Sesotho", 0x6e },
	{ (char*)"st", 0x6e },
	{ (char*)"Sundanese", 0x6f },
	{ (char*)"su", 0x6f },
	{ (char*)"Swedish", 0x70 },
	{ (char*)"sv", 0x70 },
	{ (char*)"Swahili", 0x71 },
	{ (char*)"sw", 0x71 },
	{ (char*)"Tamil", 0x72 },
	{ (char*)"ta", 0x72 },
	{ (char*)"Telugu", 0x73 },
	{ (char*)"te", 0x73 },
	{ (char*)"Tajik", 0x74 },
	{ (char*)"tg", 0x74 },
	{ (char*)"Thai", 0x75 },
	{ (char*)"th", 0x75 },
	{ (char*)"Tigrinya", 0x76 },
	{ (char*)"ti", 0x76 },
	{ (char*)"Turkmen", 0x77 },
	{ (char*)"tk", 0x77 },
	{ (char*)"Inupiak", 0x87 },
	{ (char*)"ik", 0x87 },
	{ (char*)"Icelandic", 0x33 },
	{ (char*)"is", 0x33 },
	{ (char*)"Italian", 0x34 },
	{ (char*)"it", 0x34 },
	{ (char*)"Inuktitut", 0x89 },
	{ (char*)"iu", 0x89 },
	{ (char*)"Japanese", 0x36 },
	{ (char*)"ja", 0x36 },
	{ (char*)"Javanese", 0x37 },
	{ (char*)"jw", 0x37 },
	{ (char*)"Georgian", 0x38 },
	{ (char*)"ka", 0x38 },
	{ (char*)"Kazakh", 0x39 },
	{ (char*)"kk", 0x39 },
	{ (char*)"Gerrnlandic", 0x8a },
	{ (char*)"kl", 0x8a },
	{ (char*)"Cambodian", 0x3b },
	{ (char*)"km", 0x3b },
	{ (char*)"Kannada", 0x3c },
	{ (char*)"kn", 0x3c },
	{ (char*)"Kashmiri", 0x3e },
	{ (char*)"ks", 0x3e },
	{ (char*)"Kurdish", 0x3f },
	{ (char*)"ku", 0x3f },
	{ (char*)"Kirghiz", 0x40 },
	{ (char*)"ky", 0x40 },
	{ (char*)"Latin", 0x8b },
	{ (char*)"la", 0x8b },
	{ (char*)"Lingala", 0x42 },
	{ (char*)"ln", 0x42 },
	{ (char*)"Laothian", 0x43 },
	{ (char*)"lo", 0x43 },
	{ (char*)"Lithuanian", 0x44 },
	{ (char*)"lt", 0x44 },
	{ (char*)"Lavian", 0x45 },
	{ (char*)"Lettish", 0x45 },
	{ (char*)"lv", 0x45 },
	{ (char*)"Malagasy", 0x46 },
	{ (char*)"mg", 0x46 },
	{ (char*)"Tagalog", 0x78 },
	{ (char*)"tl", 0x78 },
	{ (char*)"Setswana", 0x79 },
	{ (char*)"tn", 0x79 },
	{ (char*)"Tonga", 0x7a },
	{ (char*)"to", 0x7a },
	{ (char*)"Turkish", 0x7b },
	{ (char*)"tr", 0x7b },
	{ (char*)"Tsonga", 0x7c },
	{ (char*)"ts", 0x7c },
	{ (char*)"Tatar", 0x7d },
	{ (char*)"tt", 0x7d },
	{ (char*)"Twi", 0x7e },
	{ (char*)"tw", 0x7e },
	{ (char*)"Uighur", 0x7f },
	{ (char*)"ug", 0x7f },
	{ (char*)"Ukrainian", 0x1a },
	{ (char*)"uk", 0x1a },
	{ (char*)"Urdu", 0x21 },
	{ (char*)"ur", 0x21 },
	{ (char*)"Uzbek", 0x23 },
	{ (char*)"uz", 0x23 },
	{ (char*)"Vietnamese", 0x2f },
	{ (char*)"vi", 0x2f },
	{ (char*)"Volapuk", 0x85 },
	{ (char*)"vo", 0x85 },
	{ (char*)"Wolof", 0x31 },
	{ (char*)"wo" , 0x31 },
	{ (char*)"Xhosa", 0x32 },
	{ (char*)"xh", 0x32 },
	{ (char*)"Yiddish", 0x88 },
	{ (char*)"yi", 0x88 },
	{ (char*)"Yoruba", 0x35 },
	{ (char*)"yo", 0x35 },
	{ (char*)"Zhuang", 0x3a },
	{ (char*)"z", 0x3a },
	{ (char*)"Chinese", 0x41 },
	{ (char*)"ch", 0x41 },
	{ (char*)"Zulu", 0x5c },
	{ (char*)"zu", 0x5c },
	{ (char*)"Ko", 0x3d }
};


const SMS_WSP_HEADER_PARAMETER_S wspHeaderApplId[] = {
	{ (char*)"x-wap-application:*",  0x00 },
	{ (char*)"x-wap-application:push.sia",  0x01},
	{ (char*)"x-wap-application:wml.ua",  0x02 },
	{ (char*)"x-wap-application:wta.ua", 0x03 },
	{ (char*)"x-wap-application:mms.ua", 0x04 },
	{ (char*)"x-wap-application:push.syncml", 0x05 },
	{ (char*)"x-wap-application:loc.ua",  0x06 },
	{ (char*)"x-wap-application:syncml.dm", 0x07 },
	{ (char*)"x-wap-application:drm.ua", 0x08 },
	{ (char*)"x-wap-application:emn.ua", 0x09 },
	{ (char*)"x-oma-application:ulp.ua ", 0x90 },
	{ (char*)"x-oma-docomo:open.ctl", 0x9055 },
	{ (char*)"x-oma-docomo:xmd.mail.ua", 0x905C },
	{ (char*)"x-oma-docomo:xmd.storage.ua", 0x905F },
	{ (char*)"x-oma-docomo:xmd.lcsapp.ua", 0x9060 },
	{ (char*)"x-oma-docomo:xmd.info.ua", 0x9061 },
	{ (char*)"x-oma-docomo:xmd.agent.ua", 0x9062 },
	{ (char*)"x-oma-docomo:xmd.sab.ua", 0x9063 },
	{ (char*)"x-oma-docomo:xmd.am.ua", 0x9064 },
	{ (char*)"x-oma-docomo:xmd.emdm.ua", 0x906B },
	{ (char*)"x-oma-docomo:xmd.lac.ua", 0x906C },
	{ (char*)"x-oma-docomo:xmd.osv.ua", 0x906D },
	{ (char*)"x-oma-docomo:xmd.dcs.ua", 0x906E },
	{ (char*)"x-oma-docomo:xmd.wipe.ua", 0x906F },
	{ (char*)"x-oma-docomo:xmd.vdapp.ua ", 0x9070 },
};


const char* wspCacheControl[] = {
	(char*)"No-cache",
	(char*)"No-store",
	(char*)"Max-age",
	(char*)"Max-stale",
	(char*)"Min-fresh",
	(char*)"Only-if-cached",
	(char*)"Public",
	(char*)"Private",
	(char*)"No-transform",
	(char*)"Must-revalidate",
	(char*)"Proxy-revalidate"
};

const SMS_WSP_METHOD_TYPE_S wspMethodType[] = {
	{ (char*)"GET", 0x40 },
	{ (char*)"OPTIONS", 0x41 },
	{ (char*)"HEAD", 0x42 },
	{ (char*)"DELETE", 0x43 },
	{ (char*)"TRACE", 0x44 },
	{ (char*)"POST", 0x60 },
	{ (char*)"PUT", 0x61 }
};


const SMS_WSP_SECURITY_TYPE_S wspSecurityType[] = {
	{(char*)"NETWPIN", 0x00},
	{(char*)"USERPIN", 0x01},
	{(char*)"USERNETWPIN", 0x02},
	{(char*)"USERPINMAC", 0x03}
};



const char* wspWeek[] = {
	(char*)"Sun",
	(char*)"Mon",
	(char*)"Tue",
	(char*)"Wed",
	(char*)"Thu",
	(char*)"Fri",
	(char*)"Sat"
};


const char* wspWeekDay[] = {
	(char*)"Sunday",
	(char*)"Monday",
	(char*)"Tuesday",
	(char*)"Wednesday",
	(char*)"Thursday",
	(char*)"Friday",
	(char*)"Saturday"
};

const char* wspMonth[] = {
	(char*)"Jan",
	(char*)"Feb",
	(char*)"Mar",
	(char*)"Apr",
	(char*)"May",
	(char*)"Jun",
	(char*)"Jul",
	(char*)"Aug",
	(char*)"Sep",
	(char*)"Oct",
	(char*)"Nov",
	(char*)"Dec"
};



/*==================================================================================================
                                     IMPLEMENTATION OF SmsPluginWapPushHandler - Member Functions
==================================================================================================*/
SmsPluginWapPushHandler* SmsPluginWapPushHandler::pInstance = NULL;


SmsPluginWapPushHandler::SmsPluginWapPushHandler()
{
	memset(&tmpAddress, 0x00, sizeof(tmpAddress));
}


SmsPluginWapPushHandler::~SmsPluginWapPushHandler()
{
}


SmsPluginWapPushHandler* SmsPluginWapPushHandler::instance()
{
	if (!pInstance)
		pInstance = new SmsPluginWapPushHandler();

	return pInstance;
}


bool SmsPluginWapPushHandler::IsWapPushMsg(unsigned short dstport)
{
	MSG_BEGIN();

	int PortCount = sizeof(wapPushPortList)/sizeof(wapPushPortList[0]);

	MSG_DEBUG("Port Count [%d]", PortCount);

	for (int i = 0; i < PortCount; i++) {
		if (dstport == wapPushPortList[i]) {
			MSG_DEBUG("Wap Push Msg : [%04x]", wapPushPortList[i]);
			return true;
		}
	}

	MSG_END();

	return false;
}

#if 0
sms_wap_app_code_t SmsPluginWapPushHandler::getAppCode(const char *pPushHeader)
{
	int appCount = sizeof(pushDefaultApplication)/sizeof(pushDefaultApplication[0]) - 1;

	unsigned char *header = NULL;
	SMS_WAP_APP_CODE_T	appCode = SMS_WAP_APPLICATION_DEFAULT;

	MSG_DEBUG("Application Count [%d]", appCount);

	for (int i = 0; i < appCount; i++) {
		header = (unsigned char*)strcasestr(pPushHeader, pushDefaultApplication[i].pContentType);

		if (header != NULL) {
			appCode = pushDefaultApplication[i].appCode;
			break;
		}
	}

	if (appCode == SMS_WAP_APPLICATION_DEFAULT) {
		MSG_DEBUG("Application Type is not supported");
	} else {
		MSG_DEBUG("Find Application [%d]", appCode);
	}

	return appCode;
}
#endif


void SmsPluginWapPushHandler::copyDeliverData(sms_trans_addr_s *pAddr)
{
	memcpy(&tmpAddress, pAddr, sizeof(tmpAddress));

	MSG_SEC_DEBUG("Address [%s]", tmpAddress.szData);

/*
	tmpTimeStamp.format = pDeliver->timeStamp.format;

	if (tmpTimeStamp.format == SMS_TIME_ABSOLUTE) {
		tmpTimeStamp.time.absolute.year = pDeliver->timeStamp.time.absolute.year;
		tmpTimeStamp.time.absolute.month = pDeliver->timeStamp.time.absolute.month;
		tmpTimeStamp.time.absolute.day = pDeliver->timeStamp.time.absolute.day;
		tmpTimeStamp.time.absolute.hour = pDeliver->timeStamp.time.absolute.hour;
		tmpTimeStamp.time.absolute.minute = pDeliver->timeStamp.time.absolute.minute;
		tmpTimeStamp.time.absolute.second = pDeliver->timeStamp.time.absolute.second;
		tmpTimeStamp.time.absolute.timeZone = pDeliver->timeStamp.time.absolute.timeZone;
	}
*/
}


void SmsPluginWapPushHandler::handleWapPushMsg(const char *pUserData, int DataSize)
{
	MSG_BEGIN();

#ifdef MSG_FW_FOR_DEBUG
	MSG_DEBUG("DataSize [%d]", DataSize);

	MSG_DEBUG("[pUserData]");
	for (int i = 0; i < DataSize; i++) {
		printf("[%02x]", pUserData[i]);
	}
	printf("\n\n");
#endif

	unsigned char* pPDUTypeData = (unsigned char*)pUserData;
	unsigned long PDUTypeDataLen = DataSize;

	char* pPushHeader = NULL;
	unique_ptr<char*, void(*)(char**)> pushHeaderBuf(&pPushHeader, unique_ptr_deleter);
	unsigned long pushHeaderLen = 0;

	char* pPushBody = NULL;
	unique_ptr<char*, void(*)(char**)> PushBodyBuf(&pPushBody, unique_ptr_deleter);
	unsigned long pushBodyLen = 0;

	unsigned long iPDU = 1;

	char* pWspHeader = NULL;
	unique_ptr<char*, void(*)(char**)> WspHeaderBuf(&pWspHeader, unique_ptr_deleter);
	unsigned long	wspHeaderLen = 0;

	char* pWspBody = NULL;
	unique_ptr<char*, void(*)(char**)> WspBodyBuf(&pWspBody, unique_ptr_deleter);
	unsigned long wspBodyLen = 0;

	/* pass PDU type */
	iPDU++;

	pushHeaderLen = wspRetriveUintvarDecode(pPDUTypeData, &iPDU);

	MSG_DEBUG("PDUTypeDataLen [%d]", PDUTypeDataLen);
	MSG_DEBUG("pushHeaderLen [%d]", pushHeaderLen);
	MSG_DEBUG("iPDU [%d]", iPDU);

	wspHeaderLen = pushHeaderLen + iPDU;

	MSG_DEBUG("wspBodyLen [%d]", wspBodyLen);

	if (wspHeaderLen > 0) {
		pWspHeader = new char[wspHeaderLen];

		if (pWspHeader == NULL) {
			MSG_DEBUG("pWspHeader Memory Allocation Failed");
			return;
		} else {
			memcpy(pWspHeader, pPDUTypeData, wspHeaderLen);
		}
	}

#ifdef MSG_FW_FOR_DEBUG
	MSG_DEBUG("wspHeaderLen [%d]", wspHeaderLen);

	MSG_DEBUG("[pWspHeader]");
	for (int i = 0; i < wspHeaderLen; i++) {
		printf("[%02x]", pWspHeader[i]);
	}
	printf("\n\n");
#endif

	/* return if it is below case error */
	if (PDUTypeDataLen < wspHeaderLen) {
		MSG_DEBUG("PDUTypeDataLen < wspHeaderLen !!!");
		return;
	}

	/* Finding wspBody Information */
	wspBodyLen = PDUTypeDataLen - wspHeaderLen;

	if (wspBodyLen > 0) {
		pWspBody = new char[wspBodyLen];

		if (pWspBody == NULL) {
			MSG_DEBUG("pWspBody Memory Allocation Failed");
			return;
		} else {
			memcpy(pWspBody, pPDUTypeData + wspHeaderLen, wspBodyLen);
		}
	} else {
		pWspBody = NULL;
	}

	wspDecodeHeader(pPDUTypeData + iPDU, pushHeaderLen, PDUTypeDataLen - (iPDU + pushHeaderLen), TRUE, &pPushHeader);

	iPDU = iPDU + pushHeaderLen;

	pushBodyLen = PDUTypeDataLen - iPDU ;

	if (pushBodyLen > 0) {
		pPushBody = new char[pushBodyLen];

		if (pPushBody == NULL) {
			MSG_DEBUG("pPushBody Memory Allocation Failed");
			return;
		} else {
			memcpy(pPushBody, pPDUTypeData+ iPDU, pushBodyLen);
		}
	} else {
		pPushBody = NULL;
		return;
	}

	handleWapPushCallback((char *)pPushHeader, (char *)pPushBody, (int)pushBodyLen, (char *)pWspHeader, (int)wspHeaderLen, (char *)pWspBody, (int)wspBodyLen);

	MSG_END();
}

#if 0
void SmsPluginWapPushHandler::handleWapPushCallback(char* pPushHeader, char* pPushBody, int PushBodyLen, char* pWspHeader, int WspHeaderLen, char* pWspBody, int WspBodyLen)
{
	MSG_BEGIN();

	if (pPushBody == NULL) {
		MSG_DEBUG("pPushBody is NULL");
		return;
	}

	SMS_WAP_APP_CODE_T	appCode = getAppCode(pPushHeader);

	/*  check Push message receive setting */
	bool bPushRecv = false;

	MsgSettingGetBool(PUSH_RECV_OPTION, &bPushRecv);

	if ((bPushRecv == false) && (appCode != SMS_WAP_APPLICATION_MMS_UA)) {
		MSG_DEBUG("Push Message Receive option is OFF. Drop Push Message.");
		return;
	}

	switch (appCode) {
	case SMS_WAP_APPLICATION_MMS_UA:
		MSG_DEBUG("Received MMS Notification");
		handleMMSNotification(pPushBody, PushBodyLen);
		break;
	case SMS_WAP_APPLICATION_PUSH_SI:
		MSG_DEBUG("Received WAP Push (Service Indication Textual form)");
		handleSIMessage(pPushBody, PushBodyLen, true);
		break;
	case SMS_WAP_APPLICATION_PUSH_SIC:
		MSG_DEBUG("Received WAP Push (Service Indication Tokenised form)");
		handleSIMessage(pPushBody, PushBodyLen, false);
	break;
	case SMS_WAP_APPLICATION_PUSH_SL:
		MSG_DEBUG("Received WAP Push (Service Loading Textual form)");
		handleSLMessage(pPushBody, PushBodyLen, true);
		break;
	case SMS_WAP_APPLICATION_PUSH_SLC:
		MSG_DEBUG("Received WAP Push (Service Loading Tokenised form)");
		handleSLMessage(pPushBody, PushBodyLen, false);
	break;
	case SMS_WAP_APPLICATION_PUSH_CO:
		MSG_DEBUG("Received WAP Push (Cache Operation Textual form)");
		handleCOMessage(pPushBody, PushBodyLen, true);
		break;
	case SMS_WAP_APPLICATION_PUSH_COC:
		MSG_DEBUG("Received WAP Push (Cache Operation Tokenised form)");
		handleCOMessage(pPushBody, PushBodyLen, false);
		break;
	case SMS_WAP_APPLICATION_SYNCML_DM_BOOTSTRAP:
		MSG_DEBUG("Received DM BOOTSTRAP");
		SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DM_WBXML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
		break;
	case SMS_WAP_APPLICATION_SYNCML_DM_BOOTSTRAP_XML:
		MSG_DEBUG("Received DM BOOTSTRAP");
		SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DM_XML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
		break;
	case SMS_WAP_APPLICATION_PUSH_PROVISIONING_XML:
		MSG_DEBUG("Received Provisioning");
		SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(CP_XML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
		break;
	case SMS_WAP_APPLICATION_PUSH_PROVISIONING_WBXML:
		MSG_DEBUG("Received Provisioning");
		SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(CP_WBXML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
		break;
	case SMS_WAP_APPLICATION_PUSH_BROWSER_SETTINGS:
	case SMS_WAP_APPLICATION_PUSH_BROWSER_BOOKMARKS:
		MSG_DEBUG("Received Provisioning");
		SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(OTHERS, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
		break;
	case SMS_WAP_APPLICATION_SYNCML_DM_NOTIFICATION:
		MSG_DEBUG("Received DM Notification");
		SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DM_NOTIFICATION, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
		break;
	case SMS_WAP_APPLICATION_SYNCML_DS_NOTIFICATION:
		MSG_DEBUG("Received DS Notification");
		SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DS_NOTIFICATION, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
		break;
	case SMS_WAP_APPLICATION_SYNCML_DS_NOTIFICATION_WBXML:
		MSG_DEBUG("Received DS Notification");
		SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DS_WBXML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
		break;
	case SMS_WAP_APPLICATION_DRM_UA_RIGHTS_XML:
	case SMS_WAP_APPLICATION_DRM_UA_RIGHTS_WBXML:
		MSG_DEBUG("Received DRM UA");
		if (pPushBody != NULL)
			handleDrmVer1(pPushBody, PushBodyLen);
		break;
	case SMS_WAP_APPLICATION_DRM_V2_RO_XML:
	case SMS_WAP_APPLICATION_DRM_V2_ROAP_PDU_XML:
	case SMS_WAP_APPLICATION_DRM_V2_ROAP_TRIGGER_XML:
	case SMS_WAP_APPLICATION_DRM_V2_ROAP_TRIGGER_WBXML:
		MSG_DEBUG("Received DRM V2");
		/* TODO: DRM V2 */
		break;
	case SMS_WAP_APPLICATION_PUSH_EMAIL:
	case SMS_WAP_APPLICATION_PUSH_EMAIL_XML:
	case SMS_WAP_APPLICATION_PUSH_EMAIL_WBXML:
		MSG_DEBUG("Received Email");
		/* TODO: Email */
		break;
	case SMS_WAP_APPLICATION_PUSH_IMPS_CIR:
		MSG_DEBUG("Received IMPS CIR");
		/* TODO: IMPS CIR */
		break;
	case SMS_WAP_APPLICATION_LBS:
		MSG_DEBUG("Received LBS related message");
		SmsPluginEventHandler::instance()->handleLBSMsgIncoming(pPushHeader, pPushBody, PushBodyLen);
		/* TODO: LBS */
		break;
	case SMS_WAP_APPLICATION_PUSH_SIA:
		MSG_DEBUG("Received SIA");
		/* TODO: SIA */
		break;
	default:
		MSG_DEBUG("Unknown Application [%d]", appCode);
		break;
	}

	MSG_END();
}
#else

static void launchProcessByAppcode(int appcode)
{
	MSG_BEGIN();
	GError *error = NULL;
	GDBusConnection *connection_agent = NULL;
	GDBusProxy *dbus_proxy_agent = NULL;
	GDBusConnection *connection_service = NULL;
	GDBusProxy *dbus_proxy_service = NULL;
	GVariant *result_agent = NULL;
	GVariant *result_service = NULL;

	switch (appcode) {
	case SMS_WAP_APPLICATION_SYNCML_DM_BOOTSTRAP:
	case SMS_WAP_APPLICATION_SYNCML_DM_BOOTSTRAP_XML:
	case SMS_WAP_APPLICATION_PUSH_PROVISIONING_XML:
	case SMS_WAP_APPLICATION_PUSH_PROVISIONING_WBXML:
	case SMS_WAP_APPLICATION_PUSH_BROWSER_SETTINGS:
	case SMS_WAP_APPLICATION_PUSH_BROWSER_BOOKMARKS:
	case SMS_WAP_APPLICATION_SYNCML_DM_NOTIFICATION: {
			connection_agent = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
			if (error) {
				MSG_DEBUG("Connecting to system bus failed: %s\n", error->message);
				goto _DBUS_ERROR;
			}
				dbus_proxy_agent = g_dbus_proxy_new_sync(connection_agent, G_DBUS_PROXY_FLAGS_NONE,
										NULL, "org.tizen.omadmagent", "/org/tizen/omadmagent",
										"org.tizen.omadmagent", NULL, &error);
			if (error) {
				MSG_DEBUG("Connecting to agent proxy failed: %s\n", error->message);
				goto _DBUS_ERROR;
			}
				result_agent = g_dbus_proxy_call_sync(dbus_proxy_agent, "Hello_Agent", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
			if (error) {
				MSG_DEBUG("invoking agent proxy call failed: %s\n", error->message);
				goto _DBUS_ERROR;
			}
				connection_service = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
			if (error) {
				MSG_DEBUG("Connecting to system bus failed: %s\n", error->message);
				goto _DBUS_ERROR;
			}
				dbus_proxy_service = g_dbus_proxy_new_sync(connection_service, G_DBUS_PROXY_FLAGS_NONE, NULL,
										"org.tizen.omadmservice", "/org/tizen/omadmservice",
										"org.tizen.omadmservice", NULL, &error);
			if (error) {
				MSG_DEBUG("Connecting to service proxy failed: %s\n", error->message);
				goto _DBUS_ERROR;
			}
				result_service = g_dbus_proxy_call_sync(dbus_proxy_service, "wakeup", NULL, G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
			if (error) {
				MSG_DEBUG("invoking service proxy call failed: %s\n", error->message);
				goto _DBUS_ERROR;
			}
			Mutex mx;
			CndVar cv;
			mx.lock();
			cv.timedwait(mx.pMutex(), 2);
			mx.unlock();
			MSG_END();
		}
	break;
	case SMS_WAP_APPLICATION_SYNCML_DS_NOTIFICATION:
	case SMS_WAP_APPLICATION_SYNCML_DS_NOTIFICATION_WBXML: {
			connection_agent = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
			if (error) {
				MSG_DEBUG("Connecting to system bus failed: %s\n", error->message);
				goto _DBUS_ERROR;
			}

			dbus_proxy_agent = g_dbus_proxy_new_sync(connection_agent, G_DBUS_PROXY_FLAGS_NONE, NULL,
										"org.tizen.omadsagent", "/org/tizen/omadsagent",
										"org.tizen.omadsagent", NULL, &error);
			if (error) {
				MSG_DEBUG("Connecting to agent proxy failed: %s\n", error->message);
				goto _DBUS_ERROR;
			}
			result_agent = g_dbus_proxy_call_sync(dbus_proxy_agent, "Hello_Agent", NULL,
										G_DBUS_CALL_FLAGS_NONE, -1, NULL, &error);
			if (error) {
				MSG_DEBUG("invoking service error: %s\n", error->message);
				goto _DBUS_ERROR;
			}
		}
		break;
	default:
		break;
	}

_DBUS_ERROR:
	if (error) {
		g_error_free(error);
		error = NULL;
	}

	if (connection_agent) {
		g_object_unref(connection_agent);
		connection_agent = NULL;
	}

	if (dbus_proxy_agent) {
		g_object_unref(dbus_proxy_agent);
		dbus_proxy_agent = NULL;
	}

	if (result_agent) {
		g_object_unref(result_service);
		result_service = NULL;
	}

	if (connection_service) {
		g_object_unref(connection_service);
		connection_service = NULL;
	}

	if (dbus_proxy_service) {
		g_object_unref(dbus_proxy_service);
		dbus_proxy_service = NULL;
	}

	if (result_service) {
		g_object_unref(result_service);
		result_service = NULL;
	}

	MSG_END();
}


void SmsPluginWapPushHandler::handleWapPushCallback(char* pPushHeader, char* pPushBody, int PushBodyLen, char* pWspHeader, int WspHeaderLen, char* pWspBody, int WspBodyLen)
{
	MSG_BEGIN();

	if (pPushBody == NULL) {
		MSG_DEBUG("pPushBody is NULL");
		return;
	}

	msg_error_t err = MSG_SUCCESS;
	int pushEvt_cnt = 0;
	char app_id[MAX_WAPPUSH_ID_LEN] = {0, };
	char content_type[MAX_WAPPUSH_CONTENT_TYPE_LEN] = {0, };
	SmsPluginStorage *storageHandler = SmsPluginStorage::instance();

	err = storageHandler->getRegisteredPushEvent(pPushHeader, &pushEvt_cnt, app_id, sizeof(app_id), content_type, sizeof(content_type));
	MSG_DEBUG("pushEvt_cnt: %d", pushEvt_cnt);
	if (err != MSG_SUCCESS) {
		MSG_DEBUG("Fail to get registered push event");
		return;
	}

	for (int i = 0; i < pushEvt_cnt; ++i) {
		/*  check Push message receive setting */
		bool bPushRecv = false;
		int appcode = 0;
		MsgSettingGetBool(PUSH_RECV_OPTION, &bPushRecv);

		storageHandler->getnthPushEvent(i, &appcode);
		MSG_DEBUG("pushEvt_cnt: %d, appcode: %d", pushEvt_cnt, appcode);
		if ((bPushRecv == false) && (appcode != SMS_WAP_APPLICATION_MMS_UA)) {
			MSG_DEBUG("Push Message Receive option is OFF. Drop Push Message.");
			return;
		}

#ifdef FEATURE_MMS_DISABLE
		if (appcode == SMS_WAP_APPLICATION_MMS_UA) {
			MSG_DEBUG("Drop MMS Notification for DOCOMO");
			return;
		}

#endif
		launchProcessByAppcode(appcode);

		switch (appcode) {
		case SMS_WAP_APPLICATION_MMS_UA:
			MSG_DEBUG("Received MMS Notification");
			handleMMSNotification(pPushBody, PushBodyLen);
			break;
		case SMS_WAP_APPLICATION_PUSH_SI:
			MSG_DEBUG("Received WAP Push (Service Indication Textual form)");
			/* handleSIMessage(pPushBody, PushBodyLen, true); */
			break;
		case SMS_WAP_APPLICATION_PUSH_SIC:
			MSG_DEBUG("Received WAP Push (Service Indication Tokenised form)");
			/* handleSIMessage(pPushBody, PushBodyLen, false); */
			break;
		case SMS_WAP_APPLICATION_PUSH_SL:
			MSG_DEBUG("Received WAP Push (Service Loading Textual form)");
			/* handleSLMessage(pPushBody, PushBodyLen, true); */
			break;
		case SMS_WAP_APPLICATION_PUSH_SLC:
			MSG_DEBUG("Received WAP Push (Service Loading Tokenised form)");
			/* handleSLMessage(pPushBody, PushBodyLen, false); */
			break;
		case SMS_WAP_APPLICATION_PUSH_CO:
			MSG_DEBUG("Received WAP Push (Cache Operation Textual form)");
			/* handleCOMessage(pPushBody, PushBodyLen, true); */
			break;
		case SMS_WAP_APPLICATION_PUSH_COC:
			MSG_DEBUG("Received WAP Push (Cache Operation Tokenised form)");
			/* handleCOMessage(pPushBody, PushBodyLen, false); */
			break;
		case SMS_WAP_APPLICATION_SYNCML_DM_BOOTSTRAP:
			MSG_DEBUG("Received DM BOOTSTRAP");
			SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DM_WBXML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
			break;
		case SMS_WAP_APPLICATION_SYNCML_DM_BOOTSTRAP_XML:
			MSG_DEBUG("Received DM BOOTSTRAP");
			SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DM_XML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
			break;
		case SMS_WAP_APPLICATION_PUSH_PROVISIONING_XML:
			MSG_DEBUG("Received Provisioning");
			SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(CP_XML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
			break;
		case SMS_WAP_APPLICATION_PUSH_PROVISIONING_WBXML:
			MSG_DEBUG("Received Provisioning");
			SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(CP_WBXML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
			break;
		case SMS_WAP_APPLICATION_PUSH_BROWSER_SETTINGS:
		case SMS_WAP_APPLICATION_PUSH_BROWSER_BOOKMARKS:
			MSG_DEBUG("Received Provisioning");
			SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(OTHERS, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
			break;
		case SMS_WAP_APPLICATION_SYNCML_DM_NOTIFICATION:
			MSG_DEBUG("Received DM Notification");
			SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DM_NOTIFICATION, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
			break;
		case SMS_WAP_APPLICATION_SYNCML_DS_NOTIFICATION:
			MSG_DEBUG("Received DS Notification");
			SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DS_NOTIFICATION, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
			break;
		case SMS_WAP_APPLICATION_SYNCML_DS_NOTIFICATION_WBXML:
			MSG_DEBUG("Received DS Notification");
			SmsPluginEventHandler::instance()->handleSyncMLMsgIncoming(DS_WBXML, pPushBody, PushBodyLen, pWspHeader, WspHeaderLen);
			break;
		case SMS_WAP_APPLICATION_DRM_UA_RIGHTS_XML:
		case SMS_WAP_APPLICATION_DRM_UA_RIGHTS_WBXML:
			MSG_DEBUG("Received DRM UA");
			if (pPushBody != NULL)
				handleDrmVer1(pPushBody, PushBodyLen);
			break;
		case SMS_WAP_APPLICATION_DRM_V2_RO_XML:
		case SMS_WAP_APPLICATION_DRM_V2_ROAP_PDU_XML:
		case SMS_WAP_APPLICATION_DRM_V2_ROAP_TRIGGER_XML:
		case SMS_WAP_APPLICATION_DRM_V2_ROAP_TRIGGER_WBXML:
			MSG_DEBUG("Received DRM V2");
			/* TODO: DRM V2 */
			break;
		case SMS_WAP_APPLICATION_PUSH_EMAIL:
		case SMS_WAP_APPLICATION_PUSH_EMAIL_XML:
		case SMS_WAP_APPLICATION_PUSH_EMAIL_WBXML:
			MSG_DEBUG("Received Email");
			/* TODO: Email */
			break;
		case SMS_WAP_APPLICATION_PUSH_IMPS_CIR:
			MSG_DEBUG("Received IMPS CIR");
			/* TODO: IMPS CIR */
			break;
		case SMS_WAP_APPLICATION_LBS:
			MSG_DEBUG("Received LBS related message");
			SmsPluginEventHandler::instance()->handleLBSMsgIncoming(pPushHeader, pPushBody, PushBodyLen);
			/* TODO: LBS */
			break;
		case SMS_WAP_APPLICATION_PUSH_SIA:
			MSG_DEBUG("Received SIA");
			/* TODO: SIA */
			break;
		default:
			SmsPluginEventHandler::instance()->handlePushMsgIncoming(pPushHeader, pPushBody, PushBodyLen, app_id, content_type);
			break;
		}
	}
	storageHandler->releasePushEvent();

	MSG_END();
}
#endif

void SmsPluginWapPushHandler::handleMMSNotification(const char *pPushBody, int PushBodyLen)
{
	MSG_BEGIN();

#ifdef MSG_FW_FOR_DEBUG
	printf("\n\n[handleMMSNotification] Push Body.\n");

	for (int i = 0; i < PushBodyLen; i++) {
		printf(" [%02x]", pPushBody[i]);
	}
	printf("\n\n");
#endif

	/* Make MSG_MESSAGE_INFO_S */
	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0x00, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	createMsgInfo(&msgInfo);

	/* Convert Type values */
	msgInfo.msgType.mainType = MSG_MMS_TYPE;
	msgInfo.msgType.subType = MSG_NOTIFICATIONIND_MMS;
	msgInfo.msgType.classType = MSG_CLASS_NONE;
	msgInfo.storageId = MSG_STORAGE_PHONE;
	msgInfo.dataSize = PushBodyLen;

	if (msgInfo.dataSize > MAX_MSG_TEXT_LEN) {
		msgInfo.bTextSms = false;

		/* Save Message Data into File */
		char fileName[MSG_FILENAME_LEN_MAX+1];
		memset(fileName, 0x00, sizeof(fileName));

		if (MsgCreateFileName(fileName) == false) {
			THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");
		}

		if (MsgWriteIpcFile(fileName, pPushBody, msgInfo.dataSize) == false) {
			THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");
		}

		strncpy(msgInfo.msgData, fileName, MAX_MSG_DATA_LEN);
	} else {
		msgInfo.bTextSms = true;

		memcpy(msgInfo.msgText, pPushBody, msgInfo.dataSize);
		msgInfo.msgText[msgInfo.dataSize] = '\0';
	}

	msg_error_t err = MSG_SUCCESS;

	/* Add MMS Noti Msg into DB */
	err = SmsPluginStorage::instance()->checkMessage(&msgInfo);

	if (err == MSG_SUCCESS) {
		/*  Callback */
		err = SmsPluginEventHandler::instance()->callbackMsgIncoming(&msgInfo);

		if (err != MSG_SUCCESS)
			MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);

	} else {
		MSG_DEBUG("checkMessage() Error !! [%d]", err);
	}

	MSG_END();
}


void SmsPluginWapPushHandler::handleSIMessage(char* pPushBody, int PushBodyLen, bool isText)
{
	MSG_BEGIN();

	MSG_PUSH_MESSAGE_S pushMsg = {};

	xmlDocPtr	xmlDoc = NULL;
	xmlNodePtr	topNode = NULL;
	xmlNodePtr	indNode = NULL;

	xmlChar*		tmpXmlChar = NULL;

	if (pPushBody == NULL) {
		MSG_DEBUG("pPushBody is NULL");
		return;
	}

	getXmlDoc(pPushBody, PushBodyLen, &xmlDoc, isText);

	if (xmlDoc == NULL) {
		MSG_DEBUG("xmlDoc is NULL");
		return;
	}

	topNode = xmlDocGetRootElement(xmlDoc);

	if (topNode == NULL) {
		MSG_DEBUG("topNode is NULL");
		xmlFreeDoc(xmlDoc);
		return;
	}

	indNode = topNode->xmlChildrenNode;

	while (indNode != NULL) {
		if (!xmlStrcmp(indNode->name, (const xmlChar*) "indication")) {
			MSG_SEC_DEBUG("indNode->name = %s\n", indNode->name);
			break;
		}

		indNode = indNode->next;
	}

	if (indNode == NULL) {
		MSG_DEBUG("indNode is NULL.");
		return;
	}

	/*  temporary set to max. */
	pushMsg.expires = 0xFFFFFFFF;

	/* setting received time */
	time_t 	t 		= 	time(NULL);
	time_t 	utfTime	= 	time(&t);

	pushMsg.received = (unsigned long)utfTime;

	tmpXmlChar = xmlGetProp(indNode, (xmlChar*)SMS_PUSH_XML_HREF_TAG);

	if (tmpXmlChar == NULL) {
		MSG_DEBUG("href is NULL.");
		return;
	}

	if (tmpXmlChar != NULL)
		strncpy(pushMsg.href, (char*)tmpXmlChar, MAX_WAPPUSH_HREF_LEN-1);

	tmpXmlChar = xmlGetProp(indNode, (xmlChar*)SMS_PUSH_XML_SI_ID_TAG);

	if (tmpXmlChar != NULL)
		strncpy(pushMsg.id, (char*)tmpXmlChar, MAX_WAPPUSH_ID_LEN-1);

	tmpXmlChar = xmlGetProp(indNode, (xmlChar*)SMS_PUSH_XML_CREATED_TAG);

	if (tmpXmlChar != NULL)
		pushMsg.created = convertXmlCharToSec((char*)tmpXmlChar);

	if (pushMsg.created == 0)
		pushMsg.created = pushMsg.received;

	tmpXmlChar = xmlGetProp(indNode, (xmlChar*)SMS_PUSH_XML_EXPIRES_TAG);

	if (tmpXmlChar != NULL)
		pushMsg.expires = convertXmlCharToSec((char*)tmpXmlChar);

	tmpXmlChar = xmlGetProp(indNode, (xmlChar*)SMS_PUSH_XML_ACTION_TAG);

	pushMsg.action = convertSIActionStrToEnum((char*)tmpXmlChar);

	tmpXmlChar = xmlNodeListGetString(xmlDoc, indNode->xmlChildrenNode, 1);

	if (tmpXmlChar == NULL) {
		MSG_DEBUG("contents is NULL.");
		return;
	}

	strncpy(pushMsg.contents, (char*)tmpXmlChar, MAX_WAPPUSH_CONTENTS_LEN-1);

	/* Write push Msg to file */
	char fileName[MSG_FILENAME_LEN_MAX+1];
	memset(fileName, 0x00, sizeof(fileName));

	if (MsgCreateFileName(fileName) == false) {
		xmlFree(xmlDoc);
		xmlFree(tmpXmlChar);
		THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");
	}

	if (MsgWriteIpcFile(fileName, (char*)(&pushMsg), sizeof(pushMsg)) == false) {
		xmlFree(xmlDoc);
		xmlFree(tmpXmlChar);
		THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");
	}

	/*  Pack Message Info Structure */
	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	createMsgInfo(&msgInfo);

	strncpy(msgInfo.msgData, fileName, MAX_MSG_DATA_LEN);

	/*  Convert Type values */
	msgInfo.msgType.mainType = MSG_SMS_TYPE;
	msgInfo.msgType.subType = MSG_WAP_SI_SMS;

	msgInfo.dataSize = sizeof(pushMsg);

	xmlFree(xmlDoc);
	xmlFree(tmpXmlChar);

	msg_error_t err = MSG_SUCCESS;

	/* Add WAP Push Msg into DB */
	err = SmsPluginStorage::instance()->checkMessage(&msgInfo);

	if (err == MSG_SUCCESS) {
		/* Callback */
		err = SmsPluginEventHandler::instance()->callbackMsgIncoming(&msgInfo);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);
		}
	} else {
		MSG_DEBUG("checkMessage() Error !! [%d]", err);
	}

	MSG_END();

	return;
}


void SmsPluginWapPushHandler::handleSLMessage(char* pPushBody, int PushBodyLen, bool isText)
{
	MSG_BEGIN();

	MSG_PUSH_MESSAGE_S pushMsg = {};

	xmlDocPtr	xmlDoc = NULL;
	xmlNodePtr	topNode = NULL;
	xmlNodePtr	indNode = NULL;

	xmlChar*		tmpXmlChar = NULL;

	msg_error_t err = MSG_SUCCESS;

	if (pPushBody == NULL) {
		MSG_DEBUG("pPushBody is NULL \n");
		return;
	}

	getXmlDoc(pPushBody, PushBodyLen, &xmlDoc, isText);

	if (xmlDoc == NULL) {
		MSG_DEBUG("xmlDoc is NULL \n");
		return;
	}

	topNode = xmlDocGetRootElement(xmlDoc);

	if (topNode == NULL) {
		MSG_DEBUG("Empty Document.");
		xmlFreeDoc(xmlDoc);
		return;
	} else {
		MSG_SEC_DEBUG("Not an empty Document and topNode->name = %s \n", topNode->name);
	}

	indNode = topNode;

	while (indNode != NULL) {
		if (!xmlStrcmp(indNode->name, (const xmlChar*) "sl")) {
			MSG_SEC_DEBUG("indNode->name = %s\n", indNode->name);
			break;
		}

		indNode = indNode->next;
	}

	/*  setting received time setting */
	time_t 	t 		= 	time(NULL);
	time_t 	utfTime	= 	time(&t);

	pushMsg.received = (unsigned long)utfTime;

	tmpXmlChar = xmlGetProp(indNode, (xmlChar*)SMS_PUSH_XML_HREF_TAG);

	if (tmpXmlChar == NULL) {
		MSG_DEBUG("href is NULL.");
		return;
	}

	strncpy(pushMsg.href, (char*)tmpXmlChar, MAX_WAPPUSH_HREF_LEN-1);

	tmpXmlChar = xmlGetProp(indNode, (xmlChar*)SMS_PUSH_XML_ACTION_TAG);
	pushMsg.action = convertSLActionStrToEnum((char*)tmpXmlChar);

	/* Setting other parameters in default values */
	pushMsg.created = pushMsg.received;
	/* temporary set to MAX value. */
	pushMsg.expires = 0xFFFFFFFF;

	MSG_DEBUG("check pushMsg data");
	MSG_DEBUG("pushMsg.action : [%d]", pushMsg.action);
	MSG_DEBUG("pushMsg.received : [%d]", pushMsg.received);
	MSG_DEBUG("pushMsg.created : [%d]", pushMsg.created);
	MSG_DEBUG("pushMsg.expires : [%d]", pushMsg.expires);
	MSG_SEC_DEBUG("pushMsg.id : [%s]", pushMsg.id);
	MSG_DEBUG("pushMsg.href : [%s]", pushMsg.href);
	MSG_DEBUG("pushMsg.contents : [%s]", pushMsg.contents);

	/* Write push Msg to file */
	char fileName[MSG_FILENAME_LEN_MAX+1];
	memset(fileName, 0x00, sizeof(fileName));

	if (MsgCreateFileName(fileName) == false)
		THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");

	if (MsgWriteIpcFile(fileName, (char*)(&pushMsg), sizeof(pushMsg)) == false)
		THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");

	/* Pack Message Info Structure */
	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	createMsgInfo(&msgInfo);

	strncpy(msgInfo.msgData, fileName, MAX_MSG_DATA_LEN);

	/* Convert Type values */
	msgInfo.msgType.mainType = MSG_SMS_TYPE;
	msgInfo.msgType.subType = MSG_WAP_SL_SMS;

	/* Update Msg Text */
	strncpy(msgInfo.msgText, pushMsg.href, MAX_MSG_TEXT_LEN);

	msgInfo.dataSize = sizeof(pushMsg);

	MSG_DEBUG("dataSize : %d", msgInfo.dataSize);

	/* Add WAP Push Msg into DB */
	err = SmsPluginStorage::instance()->checkMessage(&msgInfo);

	if (err == MSG_SUCCESS) {
		/* Callback to MSG FW */
		err = SmsPluginEventHandler::instance()->callbackMsgIncoming(&msgInfo);

		if (err != MSG_SUCCESS)
			MSG_DEBUG("callbackMsgIncoming is failed, err=[%d]", err);
	} else {
		MSG_DEBUG("checkMessage() Error !! [%d]", err);
	}

	xmlFree(xmlDoc);
	xmlFree(tmpXmlChar);

	MSG_END();

	return;
}


void SmsPluginWapPushHandler::handleCOMessage(char* pPushBody, int PushBodyLen, bool isText)
{
	MSG_PUSH_CACHEOP_S cacheOp;

	xmlDocPtr	xmlDoc = NULL;
	xmlNodePtr	topNode = NULL;
	xmlNodePtr	indNode = NULL;

	memset(&cacheOp, 0x00, sizeof(cacheOp));

	MSG_DEBUG("Enter handleCOMessage");

	if (pPushBody == NULL) {
		MSG_DEBUG("pPushBody is NULL \n");
		return;
	}

	getXmlDoc(pPushBody, PushBodyLen, &xmlDoc, isText);

	if (xmlDoc == NULL) {
		MSG_DEBUG("xmlDoc is NULL \n");
		return;
	}

	topNode = xmlDocGetRootElement(xmlDoc);
	if (topNode == NULL) {
		MSG_DEBUG("Warning:Empty Document\n");
		xmlFreeDoc(xmlDoc);
		return;
	}

	indNode = topNode->xmlChildrenNode;

	while (indNode != NULL) {
		xmlChar* tmpUrl = NULL;
		if (!xmlStrcmp(indNode->name, (const xmlChar*) SMS_PUSH_XML_INVAL_OBJ)) {
			MSG_SEC_DEBUG("indNode->name = %s\n", indNode->name);

			tmpUrl = xmlGetProp(indNode, (xmlChar*) SMS_PUSH_XML_CO_URI);

			if (tmpUrl != NULL) {
				strncpy(cacheOp.invalObjectUrl[cacheOp.invalObjectCnt++], (char*)tmpUrl, MAX_PUSH_CACHEOP_MAX_URL_LEN-1);

				MSG_DEBUG("%dth invalObjectUrl is <%s>\n", cacheOp.invalObjectCnt, cacheOp.invalObjectUrl[cacheOp.invalObjectCnt-1]);
			} else {
				MSG_DEBUG("NO href value from the xmlDoc\n");
			}
		} else if (!xmlStrcmp(indNode->name, (const xmlChar*) SMS_PUSH_XML_INVAL_SVC)) {
			MSG_SEC_DEBUG("indNode->name = %s\n", indNode->name);
			tmpUrl = xmlGetProp(indNode, (xmlChar*)SMS_PUSH_XML_CO_URI);

			if (tmpUrl != NULL) {
				strncpy(cacheOp.invalServiceUrl[cacheOp.invalServiceCnt++], (char*)tmpUrl, MAX_PUSH_CACHEOP_MAX_URL_LEN-1);
				MSG_DEBUG("%dth invalServiceUrl is <%s>\n", cacheOp.invalServiceCnt, cacheOp.invalServiceUrl[cacheOp.invalServiceCnt-1]);
			} else {
				MSG_DEBUG("NO href value from the xmlDoc\n");
			}
		}

		if (tmpUrl != NULL)
			xmlFree(tmpUrl);

		indNode = indNode->next;
	}

	/*  Write push Msg to file */
	char fileName[MSG_FILENAME_LEN_MAX+1];
	memset(fileName, 0x00, sizeof(fileName));

	if (MsgCreateFileName(fileName) == false) {
		xmlFree(xmlDoc);
		THROW(MsgException::FILE_ERROR, "MsgCreateFileName error");
	}

	if (MsgWriteIpcFile(fileName, (char*)(&cacheOp), sizeof(cacheOp)) == false) {
		xmlFree(xmlDoc);
		THROW(MsgException::FILE_ERROR, "MsgWriteIpcFile error");
	}

	/*  Pack Message Info Structure */
	MSG_MESSAGE_INFO_S msgInfo;
	memset(&msgInfo, 0, sizeof(MSG_MESSAGE_INFO_S));

	msgInfo.addressList = NULL;
	unique_ptr<MSG_ADDRESS_INFO_S*, void(*)(MSG_ADDRESS_INFO_S**)> addressListBuf(&msgInfo.addressList, unique_ptr_deleter);

	createMsgInfo(&msgInfo);

	strncpy(msgInfo.msgData, fileName, MAX_MSG_DATA_LEN);

	/*  Convert Type values */
	msgInfo.msgType.mainType = MSG_SMS_TYPE;
	msgInfo.msgType.subType = MSG_WAP_CO_SMS;

	msgInfo.dataSize = sizeof(cacheOp);

	msg_error_t err = MSG_SUCCESS;

	/* Add WAP Push Msg into DB */
	err = SmsPluginStorage::instance()->checkMessage(&msgInfo);

	if (err == MSG_SUCCESS) {
		/* Callback */
		err = SmsPluginEventHandler::instance()->callbackMsgIncoming(&msgInfo);

		if (err != MSG_SUCCESS) {
			MSG_DEBUG("callbackMsgIncoming() Error !! [%d]", err);
		}
	} else {
		MSG_DEBUG("checkMessage() Error !! [%d]", err);
	}

	xmlFree(xmlDoc);

	return;
}


void SmsPluginWapPushHandler::handleDrmVer1(char* pPushBody, int PushBodyLen)
{
#if MSG_DRM_SUPPORT
	int drmRt = DRM_RETURN_SUCCESS;
	char* cid = NULL;
	unique_ptr<char*, void(*)(char**)> buf(&cid, unique_ptr_deleter);

	MSG_DEBUG("Received DRM RIGHTS OBJECT Type WAP Push Message.");
	drm_request_type_e request_type = DRM_REQUEST_TYPE_REGISTER_LICENSE;
	drm_register_lic_info_s lic_req_info;
	drm_register_lic_resp_s lic_resp_info;

	bzero(&lic_req_info, sizeof(drm_register_lic_info_s));
	bzero(&lic_resp_info, sizeof(drm_register_lic_resp_s));
	bzero(lic_req_info.lic_data, sizeof(lic_req_info.lic_data));

	memcpy(lic_req_info.lic_data, pPushBody, PushBodyLen);

	lic_req_info.lic_version = DRM_OMA_DRMV1_RIGHTS;
	lic_req_info.roap_init_src = DRM_ROAP_INIT_FROM_WAPPUSH;
	lic_req_info.operation_callback.callback = NULL;

	drmRt = drm_process_request(request_type, &lic_req_info, &lic_resp_info);
	if (drmRt == DRM_RETURN_SUCCESS) {
		MSG_DEBUG("DRM successfully registed to drm-service.");
	} else {
		MSG_DEBUG("Fail to regist DRM to drm-service.");
	}
#endif
	return;
}


void SmsPluginWapPushHandler::createMsgInfo(MSG_MESSAGE_INFO_S* pMsgInfo)
{
	/* Convert class Type values */
	pMsgInfo->msgType.classType = MSG_CLASS_NONE;

	/*  set folder id (temporary) */
	pMsgInfo->folderId = MSG_INBOX_ID;

	pMsgInfo->networkStatus = MSG_NETWORK_RECEIVED;
	pMsgInfo->bRead = false;
	pMsgInfo->bProtected = false;
	pMsgInfo->priority = MSG_MESSAGE_PRIORITY_NORMAL;
	pMsgInfo->direction = MSG_DIRECTION_TYPE_MT;

	time_t rawtime = time(NULL);
	/* Comment below lines to save local UTC time..... (it could be used later.) */
	/*
	if (tmpTimeStamp.format == SMS_TIME_ABSOLUTE) {

		MSG_DEBUG("year : %d", tmpTimeStamp.time.absolute.year);
		MSG_DEBUG("month : %d", tmpTimeStamp.time.absolute.month);
		MSG_DEBUG("day : %d", tmpTimeStamp.time.absolute.day);
		MSG_DEBUG("hour : %d", tmpTimeStamp.time.absolute.hour);
		MSG_DEBUG("minute : %d", tmpTimeStamp.time.absolute.minute);
		MSG_DEBUG("second : %d", tmpTimeStamp.time.absolute.second);
		MSG_DEBUG("timezone : %d", tmpTimeStamp.time.absolute.timeZone);

		char displayTime[32];
		struct tm * timeTM;

		struct tm timeinfo;
		memset(&timeinfo, 0x00, sizeof(tm));

		timeinfo.tm_year = (tmpTimeStamp.time.absolute.year + 100);
		timeinfo.tm_mon = (tmpTimeStamp.time.absolute.month - 1);
		timeinfo.tm_mday = tmpTimeStamp.time.absolute.day;
		timeinfo.tm_hour = tmpTimeStamp.time.absolute.hour;
		timeinfo.tm_min = tmpTimeStamp.time.absolute.minute;
		timeinfo.tm_sec = tmpTimeStamp.time.absolute.second;
		timeinfo.tm_isdst = 0;

		rawtime = mktime(&timeinfo);

		MSG_DEBUG("tzname[0] [%s]", tzname[0]);
		MSG_DEBUG("tzname[1] [%s]", tzname[1]);
		MSG_DEBUG("timezone [%d]", timezone);
		MSG_DEBUG("daylight [%d]", daylight);

		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", &timeinfo);
		MSG_DEBUG("displayTime [%s]", displayTime);

		rawtime -= (tmpTimeStamp.time.absolute.timeZone * (3600/4));

		timeTM = localtime(&rawtime);
		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", timeTM);
		MSG_DEBUG("displayTime [%s]", displayTime);

		rawtime -= timezone;

		timeTM = localtime(&rawtime);
		memset(displayTime, 0x00, sizeof(displayTime));
		strftime(displayTime, 32, "%Y-%02m-%02d %T %z", timeTM);
		MSG_DEBUG("displayTime [%s]", displayTime);
	}
	*/

	pMsgInfo->displayTime = rawtime;

	/*  Convert Address values */
	pMsgInfo->nAddressCnt = 1;

	pMsgInfo->addressList = (MSG_ADDRESS_INFO_S *)new char[sizeof(MSG_ADDRESS_INFO_S)];
	memset(pMsgInfo->addressList, 0x00, sizeof(MSG_ADDRESS_INFO_S));

	pMsgInfo->addressList[0].addressType = MSG_ADDRESS_TYPE_PLMN;
	strncpy(pMsgInfo->addressList[0].addressVal, tmpAddress.szData, MAX_ADDRESS_VAL_LEN);

	pMsgInfo->msgPort.valid = false;
	pMsgInfo->msgPort.dstPort = 0;
	pMsgInfo->msgPort.srcPort = 0;
}


void SmsPluginWapPushHandler::getXmlDoc(const char* pPushBody, const int PushBodyLen, xmlDocPtr *pXmlDoc, const bool isText)
{
	if (pPushBody == NULL) {
		MSG_DEBUG("pPushBody is NULL");
		return;
	}

	if (isText) {
		*pXmlDoc = xmlParseMemory(pPushBody, AcStrlen(pPushBody));
	} else {
		WB_UTINY*	xmldata = NULL;
		WBXMLConvWBXML2XML *conv = NULL;
		WBXMLError ret = WBXML_OK;

		ret = wbxml_conv_wbxml2xml_create(&conv);
		if (ret != WBXML_OK)
			return;

		ret = wbxml_conv_wbxml2xml_run(conv, (WB_UTINY*)pPushBody, (WB_ULONG)PushBodyLen, &xmldata, NULL);

		wbxml_conv_wbxml2xml_destroy(conv);

		if (ret != WBXML_OK ||xmldata == NULL) {
			MSG_DEBUG("xmldata is NULL. Error code is [%d].\n", ret);
			return;
		}

		MSG_DEBUG("xmldata : \n%s\n", xmldata);

		*pXmlDoc = xmlParseMemory((char*)xmldata, AcStrlen((char*)xmldata));
	}
}


unsigned long SmsPluginWapPushHandler::convertXmlCharToSec(char* pDate)
{
	struct tm	timeStruct;
	time_t		nTimeInSec = 0;
	char			tmpBuf[10];
	int 			i = 0, index = 0;

	memset(tmpBuf, 0x00, sizeof(tmpBuf));
	memset(&timeStruct, 0x00, sizeof(struct tm));

	/* check pDate */
	if (AcStrlen(pDate) < 20)
		return 0;

	MSG_DEBUG("pDate [%s]", pDate);

	/* Year */
	for (i = 0; i < 4; i++) {
		tmpBuf[i] = pDate[index++];
	}
	tmpBuf[i] = '\0';
	index++;
	timeStruct.tm_year = (atoi(tmpBuf)-1900);

	/*  Month */
	for (i = 0; i < 2; i++) {
		tmpBuf[i] = pDate[index++];
	}
	tmpBuf[i] = '\0';
	index++;
	timeStruct.tm_mon = (atoi(tmpBuf) - 1);

	/* Date */
	for (i = 0; i < 2; i++) {
		tmpBuf[i] = pDate[index++];
	}
	tmpBuf[i] = '\0';
	index++;
	timeStruct.tm_mday = atoi(tmpBuf);

	/* Hours */
	for (i = 0; i < 2; i++) {
		tmpBuf[i] = pDate[index++];
	}
	tmpBuf[i] = '\0';
	index++;
	timeStruct.tm_hour = atoi(tmpBuf);

	/* Minites */
	for (i = 0; i < 2; i++) {
		tmpBuf[i] = pDate[index++];
	}
	tmpBuf[i] = '\0';
	index++;
	timeStruct.tm_min = atoi(tmpBuf);

	/* Seconds */
	for (i = 0; i < 2; i++) {
		tmpBuf[i] = pDate[index++];
	}
	tmpBuf[i] = '\0';
	index++;
	timeStruct.tm_sec = atoi(tmpBuf);

	nTimeInSec = mktime(&timeStruct);

	return nTimeInSec;
}


msg_push_action_t SmsPluginWapPushHandler::convertSIActionStrToEnum(char* pAction)
{
	int comp = 0;

	if (pAction == NULL) {
		MSG_DEBUG("pAction is NULL. Setting to default action type.");
		return MSG_PUSH_SI_ACTION_SIGNAL_MEDIUM;
	}

	/* compare  with signal-none. */
	comp = g_strcmp0("signal-none", pAction);
	if (comp == 0)
		return MSG_PUSH_SI_ACTION_SIGNAL_NONE;

	/* compare  with signal-low. */
	comp = g_strcmp0("signal-low", pAction);
	if (comp == 0)
		return MSG_PUSH_SI_ACTION_SIGNAL_LOW;

	/*  compare  with signal-medium. */
	comp = g_strcmp0("signal-medium", pAction);
	if (comp == 0)
		return MSG_PUSH_SI_ACTION_SIGNAL_MEDIUM;

	/*  compare  with signal-high. */
	comp = g_strcmp0("signal-high", pAction);
	if (comp == 0)
		return MSG_PUSH_SI_ACTION_SIGNAL_HIGH;

	/*  compare  with delete. */
	comp = g_strcmp0("delete", pAction);
	if (comp == 0)
		return MSG_PUSH_SI_ACTION_DELETE;

	/*  signal-medium is default action value. */
	return MSG_PUSH_SI_ACTION_SIGNAL_MEDIUM;
}


msg_push_action_t SmsPluginWapPushHandler::convertSLActionStrToEnum(char* pAction)
{
	int comp = 0;

	if (pAction == NULL) {
		MSG_DEBUG("MSG_DEBUG is NULL. Setting to default action type.\n");
		return MSG_PUSH_SL_ACTION_EXECUTE_LOW;
	}

	/*  compare pSrcStr with execute-low. */
	comp = g_strcmp0("execute-low", pAction);
	if (comp == 0)
		return MSG_PUSH_SL_ACTION_EXECUTE_LOW;

	/*  compare pSrcStr with execute-high. */
	comp = g_strcmp0("execute-high", pAction);
	if (comp == 0)
		return MSG_PUSH_SL_ACTION_EXECUTE_HIGH;

	/* compare pSrcStr with cache. */
	comp = g_strcmp0("cache", pAction);
	if (comp == 0)
		return MSG_PUSH_SL_ACTION_CACHE;

	/* default SL action value is execute-low. */
	return MSG_PUSH_SL_ACTION_EXECUTE_LOW;
}


unsigned long SmsPluginWapPushHandler::wspRetriveUintvarDecode(unsigned char* sourceData, unsigned long* currentPointer)
{
	unsigned long i = 0;
	unsigned long decodedValue;

	while (sourceData[*currentPointer +i] >= 0x80) i++;

	decodedValue = wspDecodeUintvar(i+1, sourceData + *currentPointer);
	*currentPointer = *currentPointer + i + 1;
	MSG_DEBUG("wspRetriveUintvarDecode: decodedValue=%d .\n", decodedValue);
	return decodedValue;
}


unsigned long SmsPluginWapPushHandler::wspDecodeUintvar(unsigned long length, unsigned char* userVar)
{
	unsigned long i;
	unsigned long decodedUintvar = 0;


	for (i = 0 ; i < length; i++) {
		decodedUintvar = decodedUintvar +  (wspUintvarDecodeTable[i] * (userVar[length-(i+1)] & 0x7f));
	}

	return decodedUintvar;
}


void SmsPluginWapPushHandler::wspDecodeHeader(unsigned char* sEncodedHeader, unsigned long encodedHeaderLen, unsigned long contentsLength, bool fContentType, char** pHeader)
{
	unsigned long iField;
	bool   continueField = FALSE;
	unsigned long currentLength;

	char* encodedHeader = NULL;
	unique_ptr<char*, void(*)(char**)> encodedHeaderbuf(&encodedHeader, unique_ptr_deleter);

	char* outTemper = NULL;

	char* temper = NULL;
	unique_ptr<char*, void(*)(char**)> temperbuf(&temper, unique_ptr_deleter);

	unsigned char track;
	unsigned long iEncodedHeader = 0;
	unsigned char fieldCode = 0xff;

	/* outTemper is Decoded Headers.
	    temper is Single Decoded Header.
	*/
	if (NULL == (outTemper = new char[ WSP_STANDARD_STR_LEN_MAX * 5 ])) {
		MSG_DEBUG("outTemper Memory allocation is failed.\n");
		return;
	}
	memset(outTemper, 0, (WSP_STANDARD_STR_LEN_MAX * 5));
	currentLength = WSP_STANDARD_STR_LEN_MAX;

	MSG_DEBUG("wspDecodeHeader: Message header decoding started.\n");

	int loop;
	char szBuf[64];

	szBuf[0] = 0x00;
	MSG_DEBUG("wspDecodeHeader: RAW data \n");
	for (loop = 0 ; loop < (int)encodedHeaderLen; loop++) {
		char szTempBuf[5];
		szTempBuf[0] = 0x00;
		snprintf(szTempBuf, sizeof(szTempBuf), "%2X ", sEncodedHeader[loop]);

		if (AcStrlen(szBuf) + 7 < 64) {
			strncat(szBuf, szTempBuf, sizeof(szBuf)-AcStrlen(szBuf)-1);
		} else {
			strncat(szBuf, "\n", sizeof(szBuf)-AcStrlen(szBuf)-1);
			MSG_DEBUG("[%s]", szBuf);
			szBuf[0] = 0x00;
			strncat(szBuf, szTempBuf, sizeof(szBuf)-AcStrlen(szBuf)-1);
		}
	}
	strncat(szBuf, "\n", sizeof(szBuf)-AcStrlen(szBuf)-1);
	MSG_DEBUG("[%s]", szBuf);
	MSG_DEBUG("fContentType=%d  \n", fContentType);
	/* operation for content-type */
	/* makes psuedo- content-type fieldcode */
	/* content - type is processed with header. But it's come without field code. So existence of fContentType can decide adding content type header field code whether ContentType + general header is or not. */

	if (fContentType) {
		encodedHeader = new char[ encodedHeaderLen + 1 ];
		if (encodedHeader == NULL) {
			MSG_DEBUG("encodedHeader Memory allocation is failed.\n");
			return;
		}
		encodedHeader[0] = 0x91;
		memcpy(encodedHeader + 1, sEncodedHeader, (size_t)encodedHeaderLen);
	} else {
		encodedHeader = new char[ encodedHeaderLen ];
		if (encodedHeader == NULL) {
			MSG_DEBUG("encodedHeader Memory allocation is failed.\n");
			return;
		}

		memcpy(encodedHeader, sEncodedHeader, (size_t)encodedHeaderLen);
	}

	/* Is it reacehd end of header? */
	while (iEncodedHeader < encodedHeaderLen) {
		/* Get memory for single header */
		if (temper == NULL) {
			temper = new char[ WSP_STANDARD_STR_LEN_MAX * 5 ];

			if (temper == NULL) {
				MSG_DEBUG("temper Memory allocation is failed.\n");
				return;
			}
			memset(temper, 0x00, (WSP_STANDARD_STR_LEN_MAX * 5));
		} else {
			memset(temper, 0x00, (WSP_STANDARD_STR_LEN_MAX * 5));
		}


		/* this section presents header code page shifting procedure
		   This part can be implemented by future request.
		if (track == 0x 7f)
		*/
		track = encodedHeader[iEncodedHeader];

		if (track == 0x00) {
			MSG_DEBUG("WspLDecodeHeader: fieldcode  is 0 \n");
			strncpy((char*) temper, (char*)"", (WSP_STANDARD_STR_LEN_MAX * 5)-1);
			fieldCode = 0xff;
			iEncodedHeader++;
		} else if ((track > 0) && (track < 0x20)) {
			iEncodedHeader++;
		} else if ((track < 0x7f) && (track > 0x1f)) { /* In this case, first byte is normal string. So it's considered to unknown header. */
			unsigned char* fieldName = (unsigned char*)gWapCodeBufferLeft;
			unsigned char* fieldValue = (unsigned char*)gWapCodeBufferRight;

			strncpy((char*)fieldName, (char*)(encodedHeader + iEncodedHeader), WSP_CODE_BUFFER_LEFT_LEN_MAX-1);
			fieldName[WSP_CODE_BUFFER_LEFT_LEN_MAX-1] = '\0';
			iEncodedHeader = iEncodedHeader + AcStrlen((char*)fieldName) + 1;
			strncpy((char*)fieldValue, (char*)(encodedHeader + iEncodedHeader), WSP_CODE_BUFFER_RIGHT_LEN_MAX-1);
			fieldValue[WSP_CODE_BUFFER_RIGHT_LEN_MAX-1] = '\0';
			iEncodedHeader = iEncodedHeader + AcStrlen((char*)fieldValue) + 1;

			strncat((char*)temper, (char*)fieldName, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
			strncat((char*)temper, ": ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
			strncat((char*)temper, (char*)fieldValue, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
			/* this means 'don't process anymore.' */
			fieldCode = 0xff;

		} else if (track > 0x7f) {
			/* In case of first byte is field code, else case is error. */
			/*if ((track & 0x7f) <= wspHeaderFieldCount) { */
				unsigned long  fieldValueLen = encodedHeader[iEncodedHeader + 1];
				unsigned char fieldValue[1275];
				fieldCode = track & 0x7f;
				/*
				if ((fieldValueLen == 0) || (fieldValueLen == 0x80)) {
					dprint(DNET_WAP,DNET_DBG_HIGH, "%X %X %X %X %X %X\n" , fieldCode, encodedHeader[iEncodedHeader + 1], encodedHeader[iEncodedHeader + 2],encodedHeader[iEncodedHeader + 3],encodedHeader[iEncodedHeader + 4], encodedHeader[iEncodedHeader + 5]);
				}
				*/
				memset(fieldValue, 0, 1275);

				/* add field name */
				/* This continueField flag show whether previous field code and current field code are same or not. If it's same, there are some sequential display effect by omitting field name addition process. The reason why it should be do that can be found in encoding example of spec. */
				if (!continueField) {
					strncat((char*)temper, (char*)wspHeaderFieldName[fieldCode], (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					strncat((char*)temper, ": ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					continueField = FALSE;
				}

				/* field value is string */
				/* In this case, it just copy field value. */
				if ((fieldValueLen > LENGTH_QUOTE) && (fieldValueLen < 0x80)) {
					/* string field value should be NULL terminated */
					strncat((char*)temper, (char*)(encodedHeader + iEncodedHeader + 1), (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					strncat((char*)temper, (char*)fieldValue, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);

					iEncodedHeader = iEncodedHeader + AcStrlen((char*)encodedHeader + iEncodedHeader + 1) + 2;
					fieldCode = 0xff;
				}
				/* first field value is length */
				/* If first byte of field value is length value, allocate field value by the length.
				In field value, data is
				1D 03 8F 24 24  - Then 8F 24 24 is field value.
				1D 1F 33.... - Then  allocate 33H for FieldValue.
				1D 8F - Then 8F
				1D 'Hi man!' Like 00, if string is come, then process without calculating field value.
				1D 8F 24 24 - In this case, original data is wrong.
				If  accept-charset: ISO-10646-ucs-2;Q=0.7 is
				01 03 03 E8 47
				01 - field code
				03 - field value length
				03 E8 47 - field value
				it's decoded by above value.
				*/
				if (fieldValueLen < 0x20) {
					if (fieldValueLen  == LENGTH_QUOTE) {
						/* field length is encoded in UINTVAR */
						unsigned long  uintvarLen = 0;
						fieldValueLen = wspRetriveUintvarDecode((unsigned char*) encodedHeader + iEncodedHeader + 2, &uintvarLen);
						memcpy(fieldValue, encodedHeader + iEncodedHeader + 2 + uintvarLen, (size_t)fieldValueLen);
						iEncodedHeader = iEncodedHeader + fieldValueLen + uintvarLen + 2;

					} else {
						if (fieldValueLen == 1) {
							/* field value is one byte integer over 0x80 */
							/* make it two byte integer */
							fieldValue[0] = 0x00;
							memcpy(fieldValue + 1, encodedHeader + iEncodedHeader + 2, (size_t)fieldValueLen);
							fieldValueLen = 2;
							iEncodedHeader = iEncodedHeader + 1 + 2;
						} else {
							memcpy(fieldValue, encodedHeader + iEncodedHeader + 2, (size_t)fieldValueLen);
							fieldValue[fieldValueLen] = 0;
							iEncodedHeader = iEncodedHeader + fieldValueLen + 2;
							if ((fieldValueLen == 0) || (fieldValueLen == 0x80)) {
								MSG_DEBUG("%X \n",  encodedHeader[iEncodedHeader]);
							}
						}
					}
				}

				/* field value is single encoded */
				if (fieldValueLen > 0x7f) {
					fieldValue[0] = encodedHeader[iEncodedHeader + 1];
					fieldValueLen = 1;
					iEncodedHeader = iEncodedHeader + 2;
				}
				/* processing normal pre-defined field decoding */

				MSG_DEBUG("WspLDecodeHeader: FieldCode %X\n", fieldCode);
				MSG_DEBUG("WspLDecodeHeader: fieldSize %d\n", fieldValueLen);

				if ((fieldCode  > wspHeaderFieldCount) && (fieldCode != 0xff)) {
					MSG_DEBUG("WspLDecodeHeader: unknown fieldcode %X \n", track);
					strncpy((char*) temper, (char*)"", (WSP_STANDARD_STR_LEN_MAX * 5)-1);
					fieldCode = 0xff;
				}


				switch (fieldCode) {
				/* accept charset */
				/* It's normal way of field process. */
				case 0x01: {
						unsigned long  i = 0;
						unsigned long  code;

						/* Case of length of charset greater than 1 are two thigins.
						1. code length of charset is greater than 1.
						2. It include parameter.
						3. Or both of two
						*/
						if (1 != fieldValueLen) {
							code = wspHeaderDecodeInteger(fieldValue);
							/* Calculate iField. */
							if (fieldValue[0] < 0x80)
								iField = fieldValue[0];
							else
								iField = 1;

							while (wspCharset[i].charsetCode != code)
								i++;
							strncat((char*)temper, (char*)wspCharset[i].charsetName, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							/* If parameter exist */
							if (iField < fieldValueLen) {
								char* param = NULL;
								unique_ptr<char*, void(*)(char**)> parambuf(&param, unique_ptr_deleter);
								wspHeaderDecodeQValue(fieldValueLen - iField, fieldValue + iField, &param);
								strncat((char*)temper, (char*)param, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							}
						} else {
							code = fieldValue[0] & 0x7f;

							while ((wspCharset[i].charsetCode != code) && (wspCharset[i].charsetCode != 0xffff)) i++;
							strncat((char*)temper, (char*)wspCharset[i].charsetName, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
						}
					}
				break;

				/* type encoding */
				/* Like below routine, Same decoding routine process together. */
				/* Accept-encoding */
				case 0x02:
				/* content-encoding */
				case 0x0b: {
						int integerValue;

						integerValue = wspHeaderDecodeIntegerByLength(fieldValue, fieldValueLen);
						if (integerValue > 2) {
							MSG_DEBUG("WspLDecodeHeader: integerValue is over limit(2).\n");
							break;
						}
						strncat((char*)temper, (char*)wspEncodeMethod[integerValue], (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					break;
				/* contents type decoder */
				/* accept */
				case 0x00:
				/* content-type */
				case 0x11: {
						unsigned long  contentsTypeCode;
						unsigned long  i = 0;
						/* encoded content type length body */
						unsigned long  tempLen;
						MSG_DEBUG("fieldValueLen: %d", fieldValueLen);

						/* Like HTTP result state 304, it's for processing without Content type. This part doesn't defined. */
						if (0 == fieldValueLen) {
							strncat((char*)temper, (char*)"None" , (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							break;
						}
						/* 01 AE --> 00 AE --> AE*/
						if (fieldValueLen == 2  &&  fieldValue[0] == 0) {
							memcpy(fieldValue, encodedHeader + iEncodedHeader -1, (size_t)fieldValueLen-1);
							MSG_DEBUG("WspLDecodeHeader:For mmO2 problem\r\n");
						}

						if ((fieldValue[0] < 0x20) || (fieldValue[0] >= 0x80)) {
							if (fieldValue[0] >= 0x80) {
								tempLen = 1;
							} else if (fieldValueLen == 2 && fieldValue[0] == 0x03 && fieldValue[1] == 0x0A) { /* 06 05 02 03 0A AF 89 */
								fieldValue[3] = fieldValue[2];
								fieldValue[2] = fieldValue[1];
								fieldValue[1] = fieldValue[0];
								fieldValue[0] = 0x02;
								tempLen = 2;
								fieldValueLen = 3;
								MSG_DEBUG("WspLDecodeHeader:For CPE problem\r\n");
							} else {
								tempLen = fieldValue[0]; /* 06 06 03 02 03 16 AF 88 */
							}

							if (tempLen == 1) {
								char* szExtendedContent;

								contentsTypeCode = fieldValue[0] & 0x7f;
								while ((wspContentsType[i].contentsTypeCode != contentsTypeCode) && (i < wspContentsTypeCount)) i++;

								/* If specified content type doesn't exist */
								if (i < wspContentsTypeCount)
									strncat((char*)temper, (char*)wspContentsType[i].contentsTypeName, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);

								szExtendedContent =  wspExtendedDecodeType((char)contentsTypeCode);

								if (szExtendedContent != NULL) {
									MSG_DEBUG("WspLDecodeHeader: Tele2 server problem \n ");
									strncat((char*)temper, (char*)szExtendedContent, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
								}
							} else {
								contentsTypeCode = wspHeaderDecodeInteger(fieldValue);

								while ((i < wspUnregisteredContentsTypeCount) && (wspUnregisterContentsType[i].contentsTypeCode != contentsTypeCode))
									i++;

								/* If there is a Content-Type assigned, */
								if (i < wspUnregisteredContentsTypeCount)
									strncat((char*)temper, (char*)wspUnregisterContentsType[i].contentsTypeName, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);

									tempLen +=1;
							}
						} else {
							tempLen = AcStrlen((char*)fieldValue) + 1;

							strncat((char*)temper, (char*)fieldValue, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							MSG_DEBUG("WspLDecodeHeader: Attention, Decoding Check of Content-Type\n ", tempLen);
						}

						/* If there is a parameter */
						if (tempLen < fieldValueLen) {
							char* param = NULL;
							unique_ptr<char*, void(*)(char**)> parambuf(&param, unique_ptr_deleter);
							wspHeaderDecodeParameter(fieldValue + tempLen, fieldValueLen - tempLen, &param);
							if (param != NULL) {
								strncat((char*)temper, "; ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
								strncat((char*)temper, (char*)param, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							}
						}
					}
					break;

				/* language */
				/* content-language */
				case 0x0c:
				/* accept-language */
				case 0x03: {
						unsigned long i = 0;
						unsigned long code;
						unsigned long tempLen;
						if ((fieldValue[0] < 0x20) || (fieldValue[0] > 0x80)) {
							if (fieldValue[0] > 0x80)
								tempLen = 1;
							else
								tempLen = fieldValue[0];
						} else {
							tempLen = AcStrlen((char*)fieldValue) + 1;
						}

						if (tempLen == 1) {
							code = wspHeaderDecodeInteger(fieldValue);
							while (wspLanguage[i].languageCode != code) {
								i++;
								if (i == wspLanguageCount)
									break;
							}

							if (i < wspLanguageCount) {
								strncat((char*)temper, (char*)wspLanguage[i].languageName, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							}
						} else {
							strncat((char*)temper, (char*)fieldValue, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
						}

						if (tempLen < fieldValueLen) {
							char* param = NULL;
							unique_ptr<char*, void(*)(char**)> parambuf(&param, unique_ptr_deleter);
							wspHeaderDecodeQValue(fieldValueLen - tempLen, fieldValue + tempLen, &param);
							strncat((char*)temper, (char*)param, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
						}
					}
					break;

				/* integer */
				/* Max-forwards */
				case 0x1e:
				/* content-length */
				case 0x0d:
				/* age */
				case 0x05:
				/* Bearer-indication */
				case 0x33:
				/* Push-Flag */
				case 0x34: {
						unsigned char temp[16];
						/*
						if ((fieldValueLen == 2) && (fieldValue[0] > 0x7f))
							AcSprintf((char*)temp, "%u", (unsigned int)fieldValue[1]);
						else
						*/
						snprintf((char*)temp, sizeof(temp), "%u", (unsigned int)wspHeaderDecodeIntegerByLength(fieldValue, fieldValueLen));
						strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					break;
				/* X-Wap-Application-Id */
				case 0x2f: {
						unsigned char temp[64];
						int         integerValue;

						if (fieldValueLen == 2 &&  fieldValue[0] == 0) {
							memcpy(fieldValue, encodedHeader + iEncodedHeader -1, (size_t)fieldValueLen-1);
							MSG_DEBUG("WspLDecodeHeader:For mmO2 problem\r\n");
							fieldValueLen = 1;
						}

						integerValue = wspHeaderDecodeIntegerByLength(fieldValue, fieldValueLen);

						int count = sizeof(wspHeaderApplId)/sizeof(SMS_WSP_HEADER_PARAMETER_S);
						for (int i = 0; i < count ; ++i) {
							if ((unsigned int)integerValue == wspHeaderApplId[i].parameterCode) {
								snprintf((char*)temp, 64, "%s", wspHeaderApplId[i].parameterToken);
								strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen(temper)-1);
								break;
							}
						}
					}
					break;
				/* Accept-Application */
				case 0x32:
					if (0x80 == fieldValue[0]) {
						strncat((char*)temper, "*", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen(temper)-1);
					} else {
						unsigned char temp[16];
						/*
						   if ((fieldValueLen == 2) && (fieldValue[0] == 1))
							   AcSprintf((char*)temp, "%u", (unsigned int)fieldValue[0]);
						   else
						*/
						snprintf((char*)temp, sizeof(temp), "%u", (unsigned int)wspHeaderDecodeIntegerByLength(fieldValue, fieldValueLen));
						strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen(temper)-1);
					}
					break;


				/* date */
				/* last-modified */
				case 0x1d:
				/* if-unmodified-since */
				case 0x1b:
				/* if-range */
				case 0x1a:
				/* if-modified-since */
				case 0x17:
				/* expires */
				case 0x14:
				/* date */
				case 0x12: {
						char* decodedString = NULL;
						unique_ptr<char*, void(*)(char**)> decodedStringbuf(&decodedString, unique_ptr_deleter);
						wspHeaderDecodeDateValue(fieldValueLen, fieldValue, &decodedString);
						strncat((char*)temper, (char*)decodedString, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen(temper)-1);
					}
					break;

				/* connection */
				case 0x09:
					if (fieldValue[0] == 0x80)
						strncat((char*)temper, "Close", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen(temper)-1);
					break;
				/* accept-ranges */
				case 0x04:
					if (fieldValue[0] == 0x80)
						strncat((char*)temper, "None", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen(temper)-1);
					if (fieldValue[0] == 0x81)
						strncat((char*)temper, "Bytes", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen(temper)-1);
					break;
				/* content-md5 */
				case 0x0f: {
						unsigned char temp[1275];
						memcpy(temp, fieldValue, (size_t)fieldValueLen);
						temp[fieldValueLen] = 0;
						wspHeaderCopyDecodedString(temp, &currentLength, &temper);
					}
					break;
				/* Credential */
				/* authorization */
				case 0x07:
				/* proxy - authorization */
				case 0x21:
					if (fieldValue[0] == 0x80) {
						char* addString = NULL;
						unique_ptr<char*, void(*)(char**)> addStringbuf(&addString, unique_ptr_deleter);
						wspHeaderDecodeAuth(fieldValueLen, fieldValue, &addString);
						strncat((char*)temper, addString, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					} else {
						iField = AcStrlen((char*)fieldValue) + 1;

						strncat((char*)temper, (char*)fieldValue, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
						if (iField < fieldValueLen) {
							char* param = NULL;
							unique_ptr<char*, void(*)(char**)> parambuf(&param, unique_ptr_deleter);
							wspHeaderDecodeParameter(fieldValue + 1, fieldValueLen - 1, &param);
							if (param != NULL) {
								strncat((char*)temper, ", ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
								strncat((char*)temper, (char*)param, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							}
						}
					}
					break;

				/* Challenge */
				/* www - auth */
				case 0x2d:
				/* Proxy-authenticate */
				case 0x20:
					if (0 == fieldValueLen)
						break;
					if (fieldValue[0] == 0x80) {
						char* addString = NULL;
						unique_ptr<char*, void(*)(char**)> addStringbuf(&addString, unique_ptr_deleter);
						wspHeaderDecodeChallenge(fieldValueLen, fieldValue, &addString);
						strncat((char*)temper, addString, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					} else {
						unsigned char  authScheme[WSP_STANDARD_STR_LEN_MAX + 1];
						unsigned char  realmValue[WSP_STANDARD_STR_LEN_MAX];
						unsigned char  addedString[WSP_STANDARD_STR_LEN_MAX];

						strncpy((char*)authScheme, (char*)fieldValue, WSP_STANDARD_STR_LEN_MAX -1);
						iField = AcStrlen((char*)authScheme) + 1;
						strncpy((char*)realmValue, (char*)(fieldValue + iField), WSP_STANDARD_STR_LEN_MAX-1);
						iField = iField + AcStrlen((char*)realmValue) + 1;
						snprintf((char*)addedString, sizeof(addedString), "%s %s", authScheme, realmValue);
						wspHeaderCopyDecodedString(addedString, &currentLength, &temper);

						if (iField < fieldValueLen) {
							char* param = NULL;
							unique_ptr<char*, void(*)(char**)> parambuf(&param, unique_ptr_deleter);
							wspHeaderDecodeParameter(fieldValue + iField, fieldValueLen - iField, &param);
							if (param != NULL) {
								strncat((char*)temper, ", ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
								wspHeaderCopyDecodedString((unsigned char*)param, &currentLength, &temper);
							}
						}
					}
					break;

				/* content -range */
				case 0x10: {
						unsigned long  first, len, last;

						unsigned char  temp[16];
						iField = 0;

						strncat((char*)temper, " bytes ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);

						first = wspRetriveUintvarDecode(fieldValue, &iField);
						len = wspRetriveUintvarDecode(fieldValue, &iField);
						/* Originally range of HTTP include entity length. But WSP omit it. Finally to calculate this, it should be get content length from export. If this field is included without content length, then it can get wrong result. This content length can be get by calculating PDU length. */
						last = first + contentsLength - 1;

						snprintf((char*)temp, sizeof(temp), "%u-%u/%u", (unsigned int)first, (unsigned int)last, (unsigned int)len);
						strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					break;

				/* cache-control */
				case 0x08: {
						char* cacheString = NULL;
						unique_ptr<char*, void(*)(char**)> cacheStringbuf(&cacheString, unique_ptr_deleter);

						wspHeaderDecodeCacheControl(fieldValue, fieldValueLen, &cacheString);
						strncat((char*)temper, (char*)cacheString, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					break;

				/* pragma */
				case 0x1f:
					if (fieldValue[0] == 0x80) {
						strncat((char*)temper, (char*)wspCacheControl[0], (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					} else {
						if (1 < fieldValueLen) {
							char* param = NULL;
							unique_ptr<char*, void(*)(char**)> parambuf(&param, unique_ptr_deleter);
							wspHeaderDecodeParameter(fieldValue, fieldValueLen, &param);

							if (param != NULL) {
								strncat((char*)temper, "; ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
								strncat((char*)temper, (char*)param, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							}
						}
					}
					break;

				/* public */
				case 0x22:
				/* allow */
				case 0x06: {
						unsigned long  i = 0;
						while (wspHeaderDecodeIntegerByLength(fieldValue, fieldValueLen) != wspMethodType[i].methodCode) i++;
						strncat((char*)temper, (char*)wspMethodType[i].methodName, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					break;
				/* range */
				case 0x23:
					strncat((char*)temper, "bytes=", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					if (fieldValue[0] == 0x80) {
						unsigned char temp[16];
						unsigned long  first, last;
						iField = 0;

						first = wspRetriveUintvarDecode(fieldValue, &iField);
						last = wspRetriveUintvarDecode(fieldValue, &iField);

						snprintf((char*)temp, sizeof(temp), "%u-%u", (unsigned int)first, (unsigned int)last);
						strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					if (fieldValue[0] == 0x81) {
						unsigned char temp[16];
						unsigned long  suffix;

						suffix = wspRetriveUintvarDecode(fieldValue, &iField);

						snprintf((char*)temp, sizeof(temp), "-%u", (unsigned int)suffix);
					}
					break;
				/* retry-after */
				case 0x25:
					if (fieldValue[0] == 0x80) {
						char* temp = NULL;
						unique_ptr<char*, void(*)(char**)> tempbuf(&temp, unique_ptr_deleter);

						wspHeaderDecodeDateValue(fieldValueLen - 1, fieldValue + 1, &temp);
						strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}

					if (fieldValue[0] == 0x81) {
						unsigned char temp[16];

						snprintf((char*)temp, sizeof(temp), "%u", (unsigned int)wspHeaderDecodeIntegerByLength(fieldValue, fieldValueLen));
						strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					break;
				/* transfer-encoding */
				case 0x27:
					/* No other cases allowed */
					if (fieldValue[0] == 0x80)
						strncat((char*)temper, "chunked", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);

					break;
				/* vary */
				case 0x2a: {
						int integerValue = wspHeaderDecodeIntegerByLength(fieldValue, fieldValueLen);
						if (integerValue > wspHeaderFieldCount) {
							MSG_DEBUG("WspLDecodeHeader: integerValue is over limit(0x%x).\n", wspHeaderFieldCount);
							break;
						}
						strncat((char*)temper, (char*)wspHeaderFieldName[integerValue], (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					break;
				/* warning */
				case 0x2c: {
						unsigned char temp[WSP_STANDARD_STR_LEN_MAX];

						if (fieldValue[0] < 0x20)
							iField = fieldValue[0];
						else
							iField = 1;

						snprintf((char*)temp, sizeof(temp), "%u", (unsigned int)wspHeaderDecodeIntegerByLength(fieldValue, iField));
						strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
						if (iField < fieldValueLen) {
							unsigned char agent[WSP_STANDARD_STR_LEN_MAX];
							unsigned char text[WSP_STANDARD_STR_LEN_MAX];
							strncpy((char*)agent, (char*)(fieldValue + iField), WSP_STANDARD_STR_LEN_MAX-1);
							iField = iField + AcStrlen((char*)agent) + 1;
							strncpy((char*)text, (char*)(fieldValue + iField), WSP_STANDARD_STR_LEN_MAX-1);
							snprintf((char*)temp, sizeof(temp), " %s %s", agent, text);
							wspHeaderCopyDecodedString(temp, &currentLength, &temper);
						}
					}
					break;
				/* content-disposition */
				case 0x2e:
					if (fieldValue[0] == 0x80)
						strncat((char*)temper, "form-data", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);

					if (fieldValue[0] == 0x81)
						strncat((char*)temper, "attachment", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);

					if (1 < fieldValueLen) {
						char* param = NULL;
						unique_ptr<char*, void(*)(char**)> parambuf(&param, unique_ptr_deleter);
						wspHeaderDecodeParameter(fieldValue + 1, fieldValueLen - 1, &param);

						if (param != NULL) {
							strncat((char*)temper, "; ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
							strncat((char*)temper, (char*)param, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
						}
					}
					break;
				/* Profile-diff */
				case 0x36:
					temper[AcStrlen((char*)temper) + fieldValueLen] = '\0';
					memcpy(temper, fieldValue, (size_t)fieldValueLen);
					break;
				/* Profile-Warning */
				case 0x37: {
						unsigned char temp[WSP_STANDARD_STR_LEN_MAX];

						snprintf((char*)temp, sizeof(temp), "%lX", wspHeaderDecodeInteger(fieldValue));
						temp[2] = temp[1];
						temp[1] = (unsigned char)0x30;
						temp[3] = '\0';
						if (fieldValueLen > 1) {
							/* copy warn-target - URI */
							strncat((char*)temp, (char*)(fieldValue + 1), WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)temp)-1);
							if (fieldValueLen > (AcStrlen((char*)(fieldValue + 1)) + 1)) {
								/* copy warn-date */
								char* decodedString = NULL;
								unique_ptr<char*, void(*)(char**)> decodedStringbuf(&decodedString, unique_ptr_deleter);
								wspHeaderDecodeDateValue(fieldValueLen - (AcStrlen((char*)(fieldValue + 1)) + 2), fieldValue + AcStrlen((char*)(fieldValue + 1)) + 1, &decodedString);
								strncat((char*)temp, (char*)decodedString, WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)temp)-1);
							}
						}
						strncat((char*)temper, (char*)temp, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
					}
					break;

				default:
					break;
				}
			/*}*/
		}
		/* It deosn't finished decoding yet. */
		if ((iEncodedHeader < encodedHeaderLen) && (fieldCode != 0xff)) {
			/* In here, iEncodedHeader already point next field code to be decoded. */
			/* If this code is same, then set continueField else add CRLF. */
			if (fieldCode == (encodedHeader[iEncodedHeader] & 0x7f)) {
				strncat((char*)temper, ", ", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
				continueField = TRUE;
			} else {
				strncat((char*)temper, "\r\n", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
				continueField = FALSE;
			}
		} else {
			strncat((char*)temper, "\r\n", (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)temper)-1);
		}

		/* add single header to total headers */
		strncat((char*)outTemper, (char*)temper, (WSP_STANDARD_STR_LEN_MAX * 5)-AcStrlen((char*)outTemper)-1);
		MSG_DEBUG("WspLDecodeHeader: Single Header : %s\r\n", temper);
	}


	MSG_DEBUG("WspLDecodeHeader: Header decoding ended.\n");

	*pHeader = outTemper;

	return;
}


unsigned long SmsPluginWapPushHandler::wspHeaderDecodeInteger(unsigned char* data)
{
	/* we only can handle max 32bit integer */
	unsigned long i;

	union {
		unsigned long integer;
		unsigned char seg[4];
	} returner;

	returner.integer = 0;

	if (data[0] < 0x80) {
		unsigned long IntLen = 0;

		IntLen = (data[0] > 0x04) ? 0x04:data[0];

		MSG_DEBUG("WspLHeaderDecodeInteger: input %d , length %d\n", data[0], IntLen);

		for (i = 0; i < IntLen; i++)
			returner.seg[IntLen-(i+1)] = data[i+1];

		return returner.integer;
	}

	return data[0] & 0x7f;
}


void SmsPluginWapPushHandler::wspHeaderDecodeQValue(unsigned long length, unsigned char* data, char** pDecodedString)
{
	unsigned short qBase = 0;
	float  qValue;

	*pDecodedString = new char[WSP_STANDARD_STR_LEN_MAX];
	if (*pDecodedString == NULL) {
		MSG_DEBUG("WspLHeaderDecodeQValue:MemAlloc failed\n");
		return;
	}

	memcpy(&qBase, data, (size_t)length);
	qValue = (float)qBase;
	if (qValue > 100) {
		qValue = qValue - 100;
		qValue = qValue / 1000;
		sprintf((char*)*pDecodedString, "; q=%.3f", qValue);
	} else {
		/* qValue variable is divided by 100. And it's multiplied by 100.
		   It's to resolve problem of changed 0.01 of qValue. */
		unsigned long qValueTemp;
		qValue = qValue - 1;
		qValue = qValue / 100;
		qValueTemp = (unsigned long)(qValue * 100);
		if (0 == (qValueTemp % 10))
			sprintf((char*)*pDecodedString, "; q=%.1f", qValue);
		else
			sprintf((char*)*pDecodedString, "; q=%.2f", qValue);
	}
	return;
}


unsigned long SmsPluginWapPushHandler::wspHeaderDecodeIntegerByLength(unsigned char* data, unsigned long length)
{
	unsigned long i;

	union {
		unsigned long integer;
		unsigned short  seg[4];
	} returner;

	returner.integer = 0;

	if (length == 1)
		return data[0] & 0x7f;

	returner.integer = 0;

	for (i = 0 ; i < length; i++) {
		returner.integer  =  returner.integer + (data[i]  *  (0x1  << ((length - (i + 1)) * 8)));
		MSG_DEBUG("WspLHeaderDecodeIntegerByLength: %d \n", returner.integer);
	}

	return returner.integer;
}


char* SmsPluginWapPushHandler::wspExtendedDecodeType(char contentType)
{
	int i = 0;

	while (wspExtendedContentsType[i].contentsTypeCode != contentType) {
		if (wspExtendedContentsType[i].contentsTypeCode == 0xff)
			return NULL;
		i++;
	}

	return (char*)wspExtendedContentsType[i].contentsTypeName;
}


void SmsPluginWapPushHandler::wspHeaderDecodeParameter(unsigned char* data, unsigned long length, char** pParam)
{
	char* param = *pParam;

	unsigned long SecurityTypeCode;
	unsigned long  i = 0;

	if (data[0] < 0x80) {
		/* unknown parameter type */
		param = new char[WSP_STANDARD_STR_LEN_MAX];

		if (param == NULL) {
			MSG_DEBUG("WspLHeaderDecodeParameter:MemAlloc failed\n");
			return;
		}

		strncpy((char*)param, (char*)data, WSP_STANDARD_STR_LEN_MAX - 1);

		if (NO_VALUE == data[AcStrlen((char*)param) + 1]) {
			*pParam = param;
			return;
		}

		strncat((char*)param, "=", WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)param)-1);
		strncat((char*)param, (char*)(data + AcStrlen((char*)param)), WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)param)-1);

		*pParam = param;

		return;
	}

	switch (data[0] & 0x7f) {
	case 0x00:
		wspHeaderDecodeQValue(length - 1, data + 1, &param);
		break;
	case 0x01:
		wspHeaderDecodeCharset(length - 1 , data + 1, &param);
		break;
	case 0x02:
		wspHeaderDecodeVersion(length - 1, data + 1, &param);
		break;
		/* integer */
	case 0x03:
		/* param = (unsigned char *)malloc((size_t)WSP_STANDARD_STR_LEN_MAX); */
		param = new char[WSP_STANDARD_STR_LEN_MAX];
		if (param == NULL) {
			MSG_DEBUG("WspLHeaderDecodeParameter: 0x03 MemAlloc failed\n");
			return;
		} else {
			sprintf((char*)param, "Type=%i", (int)wspHeaderDecodeInteger(data + 1));
		}
		break;
	case 0x08:
		param = new char[WSP_STANDARD_STR_LEN_MAX];

		if (param == NULL) {
			MSG_DEBUG("WspLHeaderDecodeParameter:0x08 MemAlloc failed\n");
			return;
		} else {
			sprintf((char*)param, "Padding=%i", (int)wspHeaderDecodeInteger(data + 1));
		}
		break;
	case 0x05:
		param = new char[WSP_STANDARD_STR_LEN_MAX];

		if (param == NULL) {
			MSG_DEBUG("WspLHeaderDecodeParameter:0x05 MemAlloc failed\n");
			return;
		} else {
			strncpy((char*)param, "Name=", WSP_STANDARD_STR_LEN_MAX-1);
			memcpy(param + 5, data + 1, length - 1);
			param[5 + length - 1] = '\0';
		}
		break;
	case 0x06:
		param = new char[WSP_STANDARD_STR_LEN_MAX];

		if (param == NULL) {
			MSG_DEBUG("WspLHeaderDecodeParameter:0x06 MemAlloc failed\n");
			return;
		} else {
			strncpy((char*)param, "Filename=", WSP_STANDARD_STR_LEN_MAX-1);
			memcpy(param + 9, (char*)(data + 1), (size_t)(length - 1));
			param[9 + length - 1] = '\0';
		}
		break;
	case 0x07:
		param = NULL;
		/* TBI */
		break;
		/*OMA Provisioning*/
	case 0x11:
		param = new char[WSP_STANDARD_STR_LEN_MAX];

		if (param == NULL) {
			MSG_DEBUG("WspLHeaderDecodeParameter:0x11 MemAlloc failed\n");
			return;
		} else {
			strncpy((char*)param, "SEC=", WSP_STANDARD_STR_LEN_MAX-1);
			SecurityTypeCode = data[1] & 0x7f;
			while ((i < wspSecurityTypeCount) && (wspSecurityType[i].SecurityTypeCode != SecurityTypeCode))
				i++;

			if (i < wspSecurityTypeCount) {
				strncat((char*)param, (char*)wspSecurityType[i].SecurityTypeName, WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)param)-1);
			}

			if (0x12 == (data[2] & 0x7f)) {
				strncat((char*)param, "; MAC=", WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)param)-1);
				memcpy(param+AcStrlen((char*)param), (char*)(data+3), (size_t)length-3);
			}
		}
		break;

	default:
		param = NULL;
		break;
	}

	*pParam = param;
	return;
}


void SmsPluginWapPushHandler::wspHeaderDecodeCharset(unsigned long length, unsigned char* data, char**pDecodedString)
{
	*pDecodedString = new char[WSP_STANDARD_STR_LEN_MAX];

	if (*pDecodedString == NULL) {
		MSG_DEBUG("WspLHeaderDecodeCharset:MemAlloc failed\n");
		return;
	}

	strncpy((char*)*pDecodedString, "charset=", WSP_STANDARD_STR_LEN_MAX-1);

	if (data[0] > 0x80) {
		unsigned long code = wspHeaderDecodeInteger(data);
		unsigned long i = 0;
		while (wspCharset[i].charsetCode !=  code) {
			if (wspCharset[i].charsetCode == 0xffff) {
				return;
			}
			i++;
		}
		strncat((char*)*pDecodedString, (char*)wspCharset[i].charsetName, WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)*pDecodedString)-1);
	} else {
		unsigned long  lastLen = AcStrlen((char*)*pDecodedString);
		memcpy((char*)(*pDecodedString + lastLen), data, (size_t)length);
		*pDecodedString[length + lastLen] = '\0';
	}

	return;
}


void SmsPluginWapPushHandler::wspHeaderDecodeVersion(unsigned long length, unsigned char* data, char** pDecodedString)
{
	*pDecodedString = new char[WSP_STANDARD_STR_LEN_MAX];

	if (*pDecodedString == NULL) {
		MSG_DEBUG("WspLHeaderDecodeVersion:MemAlloc failed\n");
		return;
	}

	if (length > 1) {
		/* untyped version */
		memcpy(*pDecodedString, data, (size_t)length);
	} else {
		/* typed version */
		unsigned char majorVer  = ((data[0] & 0x7f) >> 4);
		unsigned char minorVer = data[0] & 0x0f;
		sprintf((char*)*pDecodedString, "level=%u.%u", majorVer, minorVer);
	}

	return;
}


void SmsPluginWapPushHandler::wspHeaderDecodeDateValue(unsigned long length, unsigned char* data, char** pDecodedString)
{
	time_t  lTime;
	struct  tm* pTMData;

	MSG_DEBUG("WspLHeaderDecodeDateValue:   \n");

	*pDecodedString = new char[WSP_STANDARD_STR_LEN_MAX];

	if (*pDecodedString == NULL) {
		MSG_DEBUG("WspLHeaderDecodeDateValue:MemAlloc failed\n");
		return;
	}

	lTime = wspHeaderDecodeIntegerByLength(data, length);

	pTMData = (struct tm*)gmtime((const time_t*)&lTime);

	if (pTMData == NULL) {
		MSG_DEBUG("WspLHeaderDecodeDateValue: Date decode fail \n");
		strncpy((char*)*pDecodedString, "Decoding Failed", WSP_STANDARD_STR_LEN_MAX-1);
		return;
	}

	/* check date value validity */
	if ((pTMData->tm_wday > 6) || (pTMData->tm_mon > 11) || (pTMData->tm_mday >  31)) {
		MSG_DEBUG("WspLHeaderDecodeDateValue: Date decode fail %d, %d, %d \n", pTMData->tm_wday, pTMData->tm_mon, pTMData->tm_mday);
		strncpy((char*)*pDecodedString, "Decoding Failed", WSP_STANDARD_STR_LEN_MAX-1);
		return;
	}

#ifdef MSG_FW_FOR_DEBUG
	/* Date type selection */
	switch (wspMachineStatus.dateType) {
		/* UNIX asciitime function */
	case UNIX_DATE_TYPE:
		snprintf((char*)decodedString, sizeof(decodedString), "%s %s %-2u %u:%u:%u %u GMT", wspWeek[pTMData->tm_wday], wspMonth[pTMData->tm_mon],
				   pTMData->tm_mday, pTMData->tm_hour, pTMData->tm_min, pTMData->tm_sec, pTMData->tm_year + 1900);
		break;
	case RFC1123_DATE_TYPE:
		snprintf((char*)decodedString, sizeof(decodedString), "%s, %u %s %u %u:%u:%u GMT", wspWeek[pTMData->tm_wday], pTMData->tm_mday,
				   wspMonth[pTMData->tm_mon], pTMData->tm_year + 1900, pTMData->tm_hour, pTMData->tm_min, pTMData->tm_sec);
		break;
	case RFC850_DATE_TYPE:
		/* Have some Y2K Problems */
		/* In RFC 850, date is represented like 11-May-99. So Y2K problem always can be occured. So remainer (year divided by 100) is used.			*/
		snprintf((char*)decodedString, sizeof(decodedString), "%s, %2u-%s-%2u %u:%u:%u GMT", wspWeekDay[pTMData->tm_wday], pTMData->tm_mday,
				   wspMonth[pTMData->tm_mon], pTMData->tm_year % CENTURY, pTMData->tm_hour, pTMData->tm_min, pTMData->tm_sec);
		break;
	}
#endif
	/*UNIX_DATE_TYPE : */
	snprintf((char*)*pDecodedString, (sizeof(char)*WSP_STANDARD_STR_LEN_MAX), "%s %s %-2u %u:%u:%u %u GMT", wspWeek[pTMData->tm_wday], wspMonth[pTMData->tm_mon],
											pTMData->tm_mday, pTMData->tm_hour, pTMData->tm_min, pTMData->tm_sec, pTMData->tm_year + 1900);

	return;
}


void SmsPluginWapPushHandler::wspHeaderCopyDecodedString(unsigned char* szDecodedString, unsigned long* currentLen, char** pTemper)
{
	unsigned long elementLen = AcStrlen((char*)szDecodedString);
	char* temper2 = NULL;

	/* CR+LF */
	*currentLen = *currentLen + elementLen + 2;

	if (*currentLen > AcStrlen((char*)* pTemper) + 2) {
		temper2 = new char[(*currentLen + 1)];

		if (temper2 == NULL) {
			MSG_DEBUG("WspLHeaderCopyDecodedString:MemAlloc failed\n");
			return;
		}
		strncpy((char*)temper2, (char*)* pTemper, *currentLen);
		delete[] *pTemper;
		strncpy((char*)temper2, (char*)szDecodedString, *currentLen);
	}

	*pTemper = temper2;

	return;
}


void SmsPluginWapPushHandler::wspHeaderDecodeAuth(unsigned long fieldValueLen, unsigned char* fieldValue, char** pDecodedString)
{
	unsigned char  userId[WSP_STANDARD_STR_LEN_MAX];
	unsigned char  passWd[WSP_STANDARD_STR_LEN_MAX];
	unsigned long iField = 0;
	char authStr[256];

	*pDecodedString = new char[WSP_STANDARD_STR_LEN_MAX * 2];

	if (*pDecodedString == NULL) {
		MSG_DEBUG("WspLHeaderDecodeAuth:MemAlloc failed\n");
		return;
	}

	/* skip 'basic' code */
	iField++;
	memset(authStr, 0x00, sizeof(authStr));
	snprintf(authStr, sizeof(authStr), "%%%ds", sizeof(userId));
	sscanf((char*)(fieldValue + iField), authStr, userId);
	iField = iField + AcStrlen((char*)userId) + 1;
	memset(authStr, 0x00, sizeof(authStr));
	snprintf(authStr, sizeof(authStr), "%%%ds", sizeof(passWd));
	sscanf((char*)(fieldValue + iField), authStr, passWd);
	iField = iField + AcStrlen((char*)userId) + 1;
	snprintf((char*)*pDecodedString, (sizeof(char)*WSP_STANDARD_STR_LEN_MAX*2), "basic %s/%s", userId, passWd);

	return;
}


void SmsPluginWapPushHandler::wspHeaderDecodeChallenge(unsigned long fieldValueLen, unsigned char* fieldValue, char** pDecodedString)
{
	unsigned char userId[WSP_STANDARD_STR_LEN_MAX];
	unsigned long iField = 0;
	char authStr[256];

	*pDecodedString = new char[WSP_STANDARD_STR_LEN_MAX];

	if (*pDecodedString == NULL) {
		MSG_DEBUG("WspLHeaderDecodeChallenge:MemAlloc failed\n");
		return;
	}

	/* skip 'basic' code */
	iField++;
	memset(authStr, 0x00, sizeof(authStr));
	snprintf(authStr, sizeof(authStr), "%%%ds", sizeof(userId));
	sscanf((char*)(fieldValue + iField), authStr, userId);
	iField = iField + AcStrlen((char*)userId) + 1;

	snprintf((char*)*pDecodedString, (sizeof(char)*WSP_STANDARD_STR_LEN_MAX), "basic realm=\"%s\"", userId);

	return;
}


void SmsPluginWapPushHandler::wspHeaderDecodeCacheControl(unsigned char* fieldValue, unsigned long fieldValueLen, char** pCacheString)
{
	unsigned char  paramString[WSP_STANDARD_STR_LEN_MAX];
	unsigned char  cacheCode;

	*pCacheString = new char[WSP_STANDARD_STR_LEN_MAX];
	if (*pCacheString == NULL) {
		MSG_DEBUG("WspLHeaderDecodeCacheControl:MemAlloc failed\n");
		return;
	}

	if (1 == fieldValueLen) {
		/* only one directive */
		if (fieldValue[0] > 0x8b) {
			return; /* It's error detection. can be omitted. */
		}
		strncpy((char*)*pCacheString, (char*)wspCacheControl[fieldValue[0] & 0x7f], WSP_STANDARD_STR_LEN_MAX-1);
		return;
	}

	if (fieldValue[0] > 0x7f) {
		/* directive that has parameter */
		cacheCode = fieldValue[0] & 0x7f;
		switch (cacheCode) {
		/* field name */
		/* no-cache */
		case 0x00:
		/* private */
		case 0x07:
			if (fieldValue[1] > 0x7f) {
				/* well known field name */
				strncpy((char*)paramString, (char*)wspHeaderFieldName[fieldValue[1] & 0x7f], WSP_STANDARD_STR_LEN_MAX-1);
				paramString[WSP_STANDARD_STR_LEN_MAX-1] = '\0';
			} else {
				/* unknown field name */
				strncpy((char*)paramString, (char*)fieldValue + 1 , WSP_STANDARD_STR_LEN_MAX-1);
			}
			break;
		/* secound */
		/* max-age */
		case 0x02:
		/* max- stale */
		case 0x03:
		/* min-fresh */
		case 0x04:
			snprintf((char*)paramString, sizeof(paramString), "%u", (unsigned int)wspHeaderDecodeInteger(fieldValue + 1));
			break;
		default:
			break;
		}
		snprintf((char*)*pCacheString, (sizeof(char)*WSP_STANDARD_STR_LEN_MAX), "%s=%s", (char*)wspCacheControl[cacheCode], (char*)paramString);
	} else {
		/* cache extentions */
		/* In case of come directive of doesn't specified string style */

		unsigned long stringLen;
		char szString[32];
		strncpy((char*)*pCacheString, (char*)fieldValue, WSP_STANDARD_STR_LEN_MAX-1);
		stringLen = AcStrlen((char*)*pCacheString);

		if (stringLen + 1 < fieldValueLen) {
			if (fieldValue[stringLen+ 1] > 0x7f) {
				int untyped = (int)wspHeaderDecodeIntegerByLength(fieldValue + stringLen + 1, fieldValueLen - (stringLen + 1));

				snprintf(szString, sizeof(szString), "%d", untyped);
				strncat((char*)*pCacheString, (char*)"=", WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)*pCacheString)-1);
				strncat((char*)*pCacheString, (char*)szString, WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)*pCacheString)-1);
			} else {
				if (fieldValue[fieldValueLen] == 0) {
					strncat((char*)*pCacheString, (char*)"=", WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)*pCacheString)-1);
					strncat((char*)*pCacheString, (char*)fieldValue + stringLen + 1 , WSP_STANDARD_STR_LEN_MAX-AcStrlen((char*)*pCacheString)-1);
				}
			}
		}
	}

	return;
}
