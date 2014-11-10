#!/bin/sh

source /etc/tizen-platform.conf

if [ ! -f $TZ_USER_DB/.msg_service.db ]
then
    mkdir -p $TZ_USER_DB/
    sqlite3 $TZ_USER_DB/.msg_service.db "PRAGMA journal_mode = PERSIST;

    CREATE TABLE MSG_CONVERSATION_TABLE (
	CONV_ID INTEGER PRIMARY KEY ,
	UNREAD_CNT INTEGER DEFAULT 0 ,
	SMS_CNT INTEGER DEFAULT 0 ,
	MMS_CNT INTEGER DEFAULT 0 ,
	MAIN_TYPE INTEGER DEFAULT 0 ,
	SUB_TYPE INTEGER DEFAULT 0 ,
	MSG_DIRECTION INTEGER DEFAULT 0 ,
	DISPLAY_TIME DATETIME ,
	DISPLAY_NAME TEXT ,
	MSG_TEXT TEXT );

    CREATE TABLE MSG_ADDRESS_TABLE (
	ADDRESS_ID INTEGER PRIMARY KEY ,
	CONV_ID INTEGER  NOT NULL ,
	ADDRESS_TYPE INTEGER ,
	RECIPIENT_TYPE INTEGER ,
	ADDRESS_VAL TEXT ,
	CONTACT_ID INTEGER ,
	DISPLAY_NAME TEXT ,
	FIRST_NAME TEXT ,
	LAST_NAME TEXT ,
	IMAGE_PATH TEXT ,
	SYNC_TIME DATETIME ,
	FOREIGN KEY (CONV_ID) REFERENCES MSG_CONVERSATION_TABLE (CONV_ID) );

    CREATE TABLE MSG_FOLDER_TABLE (
	FOLDER_ID INTEGER PRIMARY KEY ,
	FOLDER_NAME TEXT NOT NULL ,
	FOLDER_TYPE INTEGER DEFAULT 0 );

    CREATE TABLE MSG_MESSAGE_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	CONV_ID INTEGER NOT NULL ,
	FOLDER_ID INTEGER NOT NULL ,
	STORAGE_ID INTEGER NOT NULL ,
	MAIN_TYPE INTEGER NOT NULL ,
	SUB_TYPE INTEGER NOT NULL ,
	DISPLAY_TIME DATETIME ,
	DATA_SIZE INTEGER DEFAULT 0 ,
	NETWORK_STATUS INTEGER DEFAULT 0 ,
	READ_STATUS INTEGER DEFAULT 0 ,
	PROTECTED INTEGER DEFAULT 0 ,
	PRIORITY INTEGER DEFAULT 0 ,
	MSG_DIRECTION INTEGER NOT NULL ,
	SCHEDULED_TIME DATETIME ,
	BACKUP INTEGER DEFAULT 0 ,
	SUBJECT TEXT ,
	MSG_DATA TEXT ,
	THUMB_PATH TEXT ,
	MSG_TEXT TEXT ,
	ATTACHMENT_COUNT INTEGER DEFAULT 0 ,
	FOREIGN KEY (CONV_ID) REFERENCES MSG_CONVERSATION_TABLE (CONV_ID) ,
	FOREIGN KEY (FOLDER_ID) REFERENCES MSG_FOLDER_TABLE (FOLDER_ID) );

    CREATE TABLE MSG_SIM_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	SIM_ID INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_PUSH_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	ACTION INTEGER ,
	CREATED INTEGER ,
	EXPIRES INTEGER ,
	ID TEXT ,
	HREF TEXT ,
	CONTENT TEXT ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_CBMSG_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	CB_MSG_ID INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_SYNCML_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	EXT_ID INTEGER NOT NULL ,
	PINCODE INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_SCHEDULED_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	ALARM_ID INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_SMS_SENDOPT_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	DELREP_REQ INTEGER NOT NULL ,
	KEEP_COPY INTEGER NOT NULL ,
	REPLY_PATH INTEGER NOT NULL ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_FILTER_TABLE (
	FILTER_ID INTEGER PRIMARY KEY ,
	FILTER_TYPE INTEGER NOT NULL ,
	FILTER_VALUE TEXT NOT NULL ,
	FILTER_ACTIVE INTEGER DEFAULT 0 );

    CREATE TABLE MSG_MMS_MESSAGE_TABLE (
	MSG_ID INTEGER PRIMARY KEY ,
	TRANSACTION_ID TEXT ,
	MESSAGE_ID TEXT ,
	FWD_MESSAGE_ID TEXT ,
	CONTENTS_LOCATION TEXT ,
	FILE_PATH TEXT ,
	VERSION INTEGER NOT NULL ,
	DATA_TYPE INTEGER DEFAULT -1 ,
	DATE DATETIME ,
	HIDE_ADDRESS INTEGER DEFAULT 0 ,
	ASK_DELIVERY_REPORT INTEGER DEFAULT 0 ,
	REPORT_ALLOWED INTEGER DEFAULT 0 ,
	READ_REPORT_ALLOWED_TYPE INTEGER DEFAULT 0 ,
	ASK_READ_REPLY INTEGER DEFAULT 0 ,
	READ INTEGER DEFAULT 0 ,
	READ_REPORT_SEND_STATUS INTEGER DEFAULT 0 ,
	READ_REPORT_SENT INTEGER DEFAULT 0 ,
	PRIORITY INTEGER DEFAULT 0 ,
	KEEP_COPY INTEGER DEFAULT 0 ,
	MSG_SIZE INTEGER NOT NULL ,
	MSG_CLASS INTEGER DEFAULT -1 ,
	EXPIRY_TIME DATETIME ,
	CUSTOM_DELIVERY_TIME INTEGER DEFAULT 0 ,
	DELIVERY_TIME DATETIME ,
	MSG_STATUS INTEGER DEFAULT -1 ,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_MMS_PREVIEW_INFO_TABLE (
	MSG_ID INTEGER NOT NULL ,
	TYPE INTEGER,
	VALUE TEXT,
	COUNT INTEGER,
	FOREIGN KEY(MSG_ID) REFERENCES MSG_MESSAGE_TABLE(MSG_ID) );

    CREATE TABLE MSG_REPORT_TABLE (
	MSG_ID INTEGER NOT NULL ,
	ADDRESS_VAL TEXT ,
	STATUS_TYPE INTEGER ,
	STATUS INTEGER DEFAULT 0 ,
	TIME DATETIME );

    CREATE TABLE MSG_PUSHCFG_TABLE (
	PUSH_ID INTEGER PRIMARY KEY ,
	CONTENT_TYPE TEXT,
	APP_ID TEXT,
	PKG_NAME TEXT,
	LAUNCH INTEGER,
	APPCODE INTEGER,
	SECURE INTEGER );

	CREATE TABLE MSG_TMP_MSGID_TABLE (
	MSG_ID INTEGER );

    CREATE INDEX MSG_CONVERSATION_INDEX ON MSG_CONVERSATION_TABLE(CONV_ID);
    CREATE INDEX MSG_FOLDER_INDEX ON MSG_FOLDER_TABLE(FOLDER_ID);
    CREATE INDEX MSG_MESSAGE_INDEX ON MSG_MESSAGE_TABLE(MSG_ID, CONV_ID, FOLDER_ID);

    INSERT INTO MSG_FOLDER_TABLE VALUES (1, 'INBOX', 1);
    INSERT INTO MSG_FOLDER_TABLE VALUES (2, 'OUTBOX', 2);
    INSERT INTO MSG_FOLDER_TABLE VALUES (3, 'SENTBOX', 2);
    INSERT INTO MSG_FOLDER_TABLE VALUES (4, 'DRAFT', 3);
    INSERT INTO MSG_FOLDER_TABLE VALUES (5, 'CBMSGBOX', 1);
    INSERT INTO MSG_FOLDER_TABLE VALUES (6, 'SPAMBOX', 4);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (1, 'text/vnd.wap.si', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 1, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (2, 'application/vnd.wap.sic', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 2, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (3, 'text/vnd.wap.sl', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 3, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (4, 'application/vnd.wap.slc', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 4, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (5, 'text/vnd.wap.co', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 5, 0);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (6, 'application/vnd.wap.coc', 'X-Wap-Application-Id: x-wap-application:wml.ua', '', 0, 6, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (7, 'application/vnd.wap.mms-message', 'X-Wap-Application-Id: x-wap-application:mms.ua', '', 0, 7, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (8, 'application/vnd.wap.sia', 'X-Wap-Application-Id: x-wap-application:push.sia', '', 0, 8, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (9, 'application/vnd.syncml.dm+wbxml', 'X-Wap-Application-Id: x-wap-application:push.syncml.dm', '', 0, 9, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (10, 'application/vnd.syncml.dm+xml', 'X-Wap-Application-Id: x-wap-application:push.syncml.dm', '', 0, 10, 0);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (11, 'application/vnd.syncml.notification', 'X-Wap-Application-Id: x-wap-application:push.syncml.dm', '', 0, 11, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (12, 'application/vnd.syncml.ds.notification', 'X-Wap-Application-Id: x-wap-application:push.syncml.ds', '', 0, 12, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (13, 'application/vnd.syncml+wbxml', 'X-Wap-Application-Id:x-wap-application:push.syncml', '', 0, 13, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (14, 'application/vnd.wap.locc+wbxml', 'X-Wap-Application-Id: x-wap-application:loc.ua', '', 0, 14, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (15, 'application/vnd.wap.loc+xml', 'X-Wap-Application-Id: x-wap-application:loc.ua', '', 0, 15, 0);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (16, 'application/vnd.oma.dd+xml', 'X-Wap-Application-Id: x-wap-application:loc.ua', '', 0, 16, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (17, 'application/vnd.oma.drm.message', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 17, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (18, 'application/vnd.oma.drm.content', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 18, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (19, 'application/vnd.oma.drm.rights+xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 19, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (20, 'application/vnd.oma.drm.rights+wbxml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 20, 0);

    INSERT INTO MSG_PUSHCFG_TABLE VALUES (21, 'application/vnd.oma.drm.ro+xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 21, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (22, 'application/vnd.oma.drm.roap-pdu+xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 22, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (23, 'application/vnd.oma.drm.roap-trigger+xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 23, 0);
    INSERT INTO MSG_PUSHCFG_TABLE VALUES (24, 'application/vnd.oma.drm.roap-trigger+wbxml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 24, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (25, 'text/vnd.wap.connectivity-xml', 'X-Wap-Application-Id: x-wap-application:drm.ua', '', 0, 26, 0);

	INSERT INTO MSG_PUSHCFG_TABLE VALUES (26, 'application/vnd.wap.connectivity-wbxml', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 27, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (27, 'application/x-wap-prov.browser-settings', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 28, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (28, 'application/x-wap-prov.browser-bookmarks', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 29, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (29, 'application/x-wap-prov.syncset+xml', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 30, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (30, 'application/x-wap-prov.syncset+wbxml', 'X-Wap-Application-Id: x-wap-samsung:provisioning.ua', '', 0, 31, 0);

	INSERT INTO MSG_PUSHCFG_TABLE VALUES (31, 'text/vnd.wap.emn+xml', 'X-Wap-Application-Id: x-wap-application:emn.ua', '', 0, 32, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (32, 'application/vnd.wap.emn+wbxml', 'X-Wap-Application-Id: x-wap-application:emn.ua', '', 0, 33, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (33, 'application/vnd.wv.csp.cir', 'X-Wap-Application-Id: x-wap-application:wv.ua', '', 0, 34, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (34, 'application/vnd.omaloc-supl-init', 'X-Wap-Application-Id: x-oma-application:ulp.ua', '', 0, 44, 0);
	INSERT INTO MSG_PUSHCFG_TABLE VALUES (35, 'application/vnd.wap.emn+wbxml', 'X-oma-docomo:xmd.mail.ua', '', 0, 45, 1);"
fi

chown :db_msg_service $TZ_USER_DB/.msg_service.db
chown :db_msg_service $TZ_USER_DB/.msg_service.db-journal
chmod 660 $TZ_USER_DB/.msg_service.db
chmod 660 $TZ_USER_DB/.msg_service.db-journal
chsmack -a 'User' $TZ_USER_DB/.msg_service.db*

########## Setting Config Value (Internal keys) ##########
vcuid=$UID
users_gid=$(getent group $TZ_SYS_USER_GROUP | cut -f3 -d':')
# Message Server Status
vconftool set -t bool memory/msg/ready 0 -i -g users_gid -u $vcuid

# SMS Send Options
vconftool set -t int db/msg/network_mode 2 -u $vcuid

# New Message Count
vconftool set -t int db/msg/recv_sms 0 -u $vcuid
vconftool set -t int db/msg/recv_mms 0 -u $vcuid

########## Setting Config Value (Private keys) ##########
# General Options
vconftool set -t bool db/private/msg-service/general/keep_copy 1 -u $vcuid
vconftool set -t bool db/private/msg-service/general/auto_erase 0 -u $vcuid
vconftool set -t bool db/private/msg-service/general/block_msg 0 -u $vcuid
vconftool set -t int db/private/msg-service/general/contact_sync_time 0 -u $vcuid

# SMS Send Options
vconftool set -t int db/private/msg-service/sms_send/dcs 3 -u $vcuid
vconftool set -t bool db/private/msg-service/sms_send/reply_path 0 -u $vcuid
vconftool set -t bool db/private/msg-service/sms_send/delivery_report 0 -u $vcuid
vconftool set -t int db/private/msg-service/sms_send/save_storage 1 -u $vcuid

# SMSC
vconftool set -t int db/private/msg-service/smsc/total_count 1 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/selected 0 -u $vcuid

vconftool set -t int db/private/msg-service/smsc/pid/0 1 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/val_period/0 255 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/name/0 "" -u $vcuid
vconftool set -t int db/private/msg-service/smsc/ton/0 1 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/npi/0 1 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/address/0 "" -u $vcuid

vconftool set -t int db/private/msg-service/smsc/pid/1 0 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/val_period/1 0 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/name/1 "" -u $vcuid
vconftool set -t int db/private/msg-service/smsc/ton/1 0 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/npi/1 0 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/address/1 "" -u $vcuid

vconftool set -t int db/private/msg-service/smsc/pid/2 0 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/val_period/2 0 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/name/2 "" -u $vcuid
vconftool set -t int db/private/msg-service/smsc/ton/2 0 -u $vcuid
vconftool set -t int db/private/msg-service/smsc/npi/2 0 -u $vcuid
vconftool set -t string db/private/msg-service/smsc/address/2 "" -u $vcuid

# MMS Send Options
vconftool set -t int db/private/msg-service/mms_send/msg_class 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/priority 1 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/expiry_time 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/custom_delivery 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/sender_visibility 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/delivery_report 1 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/read_reply 1 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/keep_copy 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/body_replying 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/hide_recipients 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_send/report_allowed 1 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/reply_charging 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/reply_charging_deadline 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/reply_charging_size 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/delivery_time 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_send/creation_mode 2 -u $vcuid

# MMS Receive Options
vconftool set -t int db/private/msg-service/mms_recv/home_network 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_recv/abroad_network 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_recv/read_receipt 1 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_recv/delivery_receipt 1 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_recv/reject_unknown 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_recv/reject_advertisement 0 -u $vcuid

# MMS Receive Options
vconftool set -t int db/private/msg-service/mms_style/font_size 30 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_style/font_style/bold 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_style/font_style/italic 0 -u $vcuid
vconftool set -t bool db/private/msg-service/mms_style/font_style/underline 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/font_color/red 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/font_color/green 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/font_color/blue 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/font_color/hue 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/bg_color/red 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/bg_color/green 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/bg_color/blue 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/bg_color/hue 255 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/page_dur 2 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/page_custom_dur 0 -u $vcuid
vconftool set -t int db/private/msg-service/mms_style/page_dur_manual 0 -u $vcuid

# Push Msg Options
vconftool set -t bool db/private/msg-service/push_msg/recv_option 1 -u $vcuid
vconftool set -t int db/private/msg-service/push_msg/service_load 1 -u $vcuid

# CB Msg Options
vconftool set -t bool db/private/msg-service/cb_msg/receive 1 -f -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/save 1 -f -u $vcuid
vconftool set -t int db/private/msg-service/cb_msg/max_sim_count 0 -u $vcuid
vconftool set -t int db/private/msg-service/cb_msg/channel_count 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/0 1 -f -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/1 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/2 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/3 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/4 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/5 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/6 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/7 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/8 0 -u $vcuid
vconftool set -t bool db/private/msg-service/cb_msg/language/9 0 -u $vcuid

# Voice Mail Options
vconftool set -t string db/private/msg-service/voice_mail/voice_mail_number "5500" -f -u $vcuid
vconftool set -t int db/private/msg-service/voice_mail/voice_mail_count 0 -u $vcuid

# MMS Size Options
vconftool set -t int db/private/msg-service/size_opt/msg_size 300 -u $vcuid

# SIM message count
vconftool set -t int db/private/msg-service/sim_count/used_cnt 0 -u $vcuid
vconftool set -t int db/private/msg-service/sim_count/total_cnt 0 -u $vcuid

# SIM information
vconftool set -t int memory/private/msg-service/sim_changed 0 -i -u $vcuid
vconftool set -t string memory/private/msg-service/sim_imsi "" -i -u $vcuid
vconftool set -t bool memory/private/msg-service/national_sim 0 -i -u $vcuid
vconftool set -t string memory/private/msg-service/msisdn "" -i -u $vcuid

vconftool set -t int db/private/msg-service/notification_priv_id 0 -u $vcuid
