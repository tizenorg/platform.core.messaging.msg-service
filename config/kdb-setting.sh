vconftool set -t bool db/msg/general/keep_copy 1
vconftool set -t int db/msg/general/alert_tone 0
vconftool set -t bool db/msg/general/auto_erase 0

vconftool set -t int db/msg/sms_send/dcs 3
vconftool set -t int db/msg/network_mode 4
vconftool set -t bool db/msg/sms_send/reply_path 0
vconftool set -t bool db/msg/sms_send/delivery_report 0
vconftool set -t int db/msg/sms_send/save_storage 1

vconftool set -t int db/msg/mms_send/msg_class 0
vconftool set -t int db/msg/mms_send/priority 1
vconftool set -t int db/msg/mms_send/expiry 0
vconftool set -t bool db/msg/mms_send/custom_delivery 0
vconftool set -t bool db/msg/mms_send/sender_visibility 0
vconftool set -t bool db/msg/mms_send/delivery_report 1
vconftool set -t bool db/msg/mms_send/read_reply 1
vconftool set -t bool db/msg/mms_send/keep_copy 0
vconftool set -t bool db/msg/mms_send/body_replying 0
vconftool set -t bool db/msg/mms_send/hide_recipients 0
vconftool set -t bool db/msg/mms_send/report_allowed 1
vconftool set -t int db/msg/mms_send/reply_charging 0
vconftool set -t int db/msg/mms_send/reply_charging_deadline 0
vconftool set -t int db/msg/mms_send/reply_charging_size 0
vconftool set -t int db/msg/mms_send/delivery_time 0
vconftool set -t int db/msg/mms_send/creation_mode 2

vconftool set -t int db/msg/mms_recv/home_network 0
vconftool set -t int db/msg/mms_recv/abroad_network 0
vconftool set -t bool db/msg/mms_recv/read_receipt 1
vconftool set -t bool db/msg/mms_recv/delivery_receipt 1
vconftool set -t bool db/msg/mms_recv/reject_unknown 0
vconftool set -t bool db/msg/mms_recv/reject_advertisement 0

vconftool set -t bool db/msg/push_msg/recv_option 1
vconftool set -t int db/msg/push_msg/service_load 1

vconftool set -t bool db/msg/cb_msg/receive 0
vconftool set -t bool db/msg/cb_msg/all_channel 0
vconftool set -t int db/msg/cb_msg/max_sim_count 0
vconftool set -t int db/msg/cb_msg/channel_count 0
vconftool set -t bool db/msg/cb_msg/language/0 0
vconftool set -t bool db/msg/cb_msg/language/1 0
vconftool set -t bool db/msg/cb_msg/language/2 0
vconftool set -t bool db/msg/cb_msg/language/3 0
vconftool set -t bool db/msg/cb_msg/language/4 0
vconftool set -t bool db/msg/cb_msg/language/5 0
vconftool set -t bool db/msg/cb_msg/language/6 0
vconftool set -t bool db/msg/cb_msg/language/7 0
vconftool set -t bool db/msg/cb_msg/language/8 0
vconftool set -t bool db/msg/cb_msg/language/9 0

vconftool set -t int db/msg/size_opt/msg_size 300

