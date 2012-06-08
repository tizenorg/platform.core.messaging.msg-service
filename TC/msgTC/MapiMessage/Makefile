###################################################
# add your TestCase List Here
#
# e.g.,
# TC1 = utc_frameworkName_apiName_func
# TC2 = utc_ApplicationLib_recurGetDayOfWeek_func
TC1 = utc_msg_add_address
TC2 = utc_msg_get_time
TC3 = utc_msg_new_message
TC4 = utc_msg_mms_create_message
TC5 = utc_msg_is_in_sim
TC6 = utc_msg_release_message
TC7 = utc_msg_get_address_count
TC8 = utc_msg_is_mms
TC9 = utc_msg_reset_address
TC10 = utc_msg_get_dest_port
TC11 = utc_msg_is_protected
TC12 = utc_msg_set_direction_info
TC13 = utc_msg_get_direction_info
TC14 = utc_msg_is_read
TC15 = utc_msg_set_encode_type
TC16 = utc_msg_get_encode_type
TC17 = utc_msg_is_sms
TC18 = utc_msg_set_folder_id
TC19 = utc_msg_get_folder_id
TC10 = utc_msg_mms_add_attachment
TC21 = utc_msg_set_message_id
TC22 = utc_msg_get_ith_address
TC23 = utc_msg_mms_add_media
TC24 = utc_msg_set_message_type
TC25 = utc_msg_get_ith_contact_id
TC26 = utc_msg_mms_add_page
TC27 = utc_msg_set_network_status
TC28 = utc_msg_get_ith_name
TC29 = utc_msg_mms_add_region
TC30 = utc_msg_set_port
TC31 = utc_msg_get_ith_recipient_type
TC32 = utc_msg_mms_add_transition
TC33 = utc_msg_set_priority_info
TC34 = utc_msg_get_ith_thread_id
TC35 = utc_msg_mms_destroy_message
TC36 = utc_msg_set_protect_status
TC37 = utc_msg_get_message_body_size
TC38 = utc_msg_mms_get_attachment
TC39 = utc_msg_set_read_status
TC40 = utc_msg_get_message_id
TC41 = utc_msg_mms_get_media
TC42 = utc_msg_set_scheduled_time
TC43 = utc_msg_get_message_type
TC44 = utc_msg_mms_get_message_body
TC45 = utc_msg_set_storage_id
TC46 = utc_msg_get_network_status
TC47 = utc_msg_mms_get_meta
TC48 = utc_msg_set_subject
TC49 = utc_msg_get_priority_info
TC50 = utc_msg_mms_get_page
TC51 = utc_msg_set_time
TC52 = utc_msg_get_scheduled_time
TC53 = utc_msg_mms_get_smil_region
TC54 = utc_msg_sms_get_message_body
TC55 = utc_msg_get_src_port
TC56 = utc_msg_mms_get_transition
TC57 = utc_msg_sms_set_message_body
TC58 = utc_msg_get_storage_id
TC59 = utc_msg_mms_set_message_body
TC60 = utc_msg_get_subject
TC61 = utc_msg_mms_set_rootlayout
TC62 = utc_msg_mms_add_meta
TC63 = utc_msg_get_dest_port


###################################################
# add your Package Config Info Here
#
# e.g.,
# PKGS=calendar
PKGS=msg-service

LIBS  = `pkg-config --libs $(PKGS)`
LIBS +=$(TET_ROOT)/lib/tet3/tcm_s.o
LIBS +=-L$(TET_ROOT)/lib/tet3 -ltcm_s
LIBS +=-L$(TET_ROOT)/lib/tet3 -lapi_s

INCS  =  -I. `pkg-config --cflags $(PKGS)`
INCS  += -I$(TET_ROOT)/inc/tet3

CFLAGS = $(INCS)
CFLAGS += -D_TETWARE_MODE
#CC +=$(CFLAGS)
LDFLAGS = $(LIBS)

###################################################
# Modify here
# depending on the Test Case you want to build
#
# e.g.,
# TCLIST = $(TC1) $(TC2)
TCLIST = $(TC1) $(TC2) $(TC3) $(TC4) $(TC5) $(TC6) $(TC7) $(TC8) $(TC9) $(TC10) $(TC11) $(TC12) $(TC13) $(TC14) $(TC15) $(TC16) $(TC17) $(TC18) $(TC19) $(TC20) $(TC21) $(TC22) $(TC23) $(TC24) $(TC25) $(TC26) $(TC27) $(TC28) $(TC29) $(TC30) $(TC31) $(TC32) $(TC33) $(TC34) $(TC35) $(TC36) $(TC37) $(TC38) $(TC39) $(TC40) $(TC41) $(TC42) $(TC43) $(TC44) $(TC45) $(TC46) $(TC47) $(TC48) $(TC49) $(TC50) $(TC51) $(TC52) $(TC53) $(TC54) $(TC55) $(TC56) $(TC57) $(TC58) $(TC59) $(TC60) $(TC61) $(TC62) $(TC63)

all :
	for testcase in $(TCLIST) do\
		$(CC) -o $$(testcase) $$(testcase).c $(CFLAGS) $(LDFLAGS)
	done

clean :
	rm -rf *~ *.o $(TCLIST)


$(TC1) :
	$(CC) -o $(TC1) $(TC1).c  $(CFLAGS) $(LDFLAGS)

$(TC2) :
	$(CC) -o $(TC2) $(TC2).c  $(CFLAGS) $(LDFLAGS)

$(TC3) :
	$(CC) -o $(TC3) $(TC3).c  $(CFLAGS) $(LDFLAGS)

$(TC4) :
	$(CC) -o $(TC4) $(TC4).c  $(CFLAGS) $(LDFLAGS)

$(TC5) :
	$(CC) -o $(TC5) $(TC5).c  $(CFLAGS) $(LDFLAGS)

$(TC6) :
	$(CC) -o $(TC6) $(TC6).c  $(CFLAGS) $(LDFLAGS)

$(TC7) :
	$(CC) -o $(TC7) $(TC7).c  $(CFLAGS) $(LDFLAGS)

$(TC8) :
	$(CC) -o $(TC8) $(TC8).c  $(CFLAGS) $(LDFLAGS)

$(TC9) :
	$(CC) -o $(TC9) $(TC9).c  $(CFLAGS) $(LDFLAGS)

$(TC10) :
	$(CC) -o $(TC10) $(TC10).c  $(CFLAGS) $(LDFLAGS)

$(TC11) :
	$(CC) -o $(TC11) $(TC11).c  $(CFLAGS) $(LDFLAGS)

$(TC12) :
	$(CC) -o $(TC12) $(TC12).c  $(CFLAGS) $(LDFLAGS)

$(TC13) :
	$(CC) -o $(TC13) $(TC13).c  $(CFLAGS) $(LDFLAGS)

$(TC14) :
	$(CC) -o $(TC14) $(TC14).c  $(CFLAGS) $(LDFLAGS)

$(TC15) :
	$(CC) -o $(TC15) $(TC15).c  $(CFLAGS) $(LDFLAGS)

$(TC16) :
	$(CC) -o $(TC16) $(TC16).c  $(CFLAGS) $(LDFLAGS)

$(TC17) :
	$(CC) -o $(TC17) $(TC17).c  $(CFLAGS) $(LDFLAGS)

$(TC18) :
	$(CC) -o $(TC18) $(TC18).c  $(CFLAGS) $(LDFLAGS)

$(TC19) :
	$(CC) -o $(TC19) $(TC19).c  $(CFLAGS) $(LDFLAGS)

$(TC20) :
	$(CC) -o $(TC20) $(TC20).c  $(CFLAGS) $(LDFLAGS)

$(TC21) :
	$(CC) -o $(TC21) $(TC21).c  $(CFLAGS) $(LDFLAGS)

$(TC22) :
	$(CC) -o $(TC22) $(TC22).c  $(CFLAGS) $(LDFLAGS)

$(TC23) :
	$(CC) -o $(TC23) $(TC23).c  $(CFLAGS) $(LDFLAGS)

$(TC24) :
	$(CC) -o $(TC24) $(TC24).c  $(CFLAGS) $(LDFLAGS)

$(TC25) :
	$(CC) -o $(TC25) $(TC25).c  $(CFLAGS) $(LDFLAGS)

$(TC26) :
	$(CC) -o $(TC26) $(TC26).c  $(CFLAGS) $(LDFLAGS)

$(TC27) :
	$(CC) -o $(TC27) $(TC27).c  $(CFLAGS) $(LDFLAGS)

$(TC28) :
	$(CC) -o $(TC28) $(TC28).c  $(CFLAGS) $(LDFLAGS)

$(TC29) :
	$(CC) -o $(TC29) $(TC29).c  $(CFLAGS) $(LDFLAGS)

$(TC30) :
	$(CC) -o $(TC30) $(TC30).c  $(CFLAGS) $(LDFLAGS)

$(TC31) :
	$(CC) -o $(TC31) $(TC31).c  $(CFLAGS) $(LDFLAGS)

$(TC32) :
	$(CC) -o $(TC32) $(TC32).c  $(CFLAGS) $(LDFLAGS)

$(TC33) :
	$(CC) -o $(TC33) $(TC33).c  $(CFLAGS) $(LDFLAGS)

$(TC34) :
	$(CC) -o $(TC34) $(TC34).c  $(CFLAGS) $(LDFLAGS)

$(TC35) :
	$(CC) -o $(TC35) $(TC35).c  $(CFLAGS) $(LDFLAGS)

$(TC36) :
	$(CC) -o $(TC36) $(TC36).c  $(CFLAGS) $(LDFLAGS)

$(TC37) :
	$(CC) -o $(TC37) $(TC37).c  $(CFLAGS) $(LDFLAGS)

$(TC38) :
	$(CC) -o $(TC38) $(TC38).c  $(CFLAGS) $(LDFLAGS)

$(TC39) :
	$(CC) -o $(TC39) $(TC39).c  $(CFLAGS) $(LDFLAGS)

$(TC40) :
	$(CC) -o $(TC40) $(TC40).c  $(CFLAGS) $(LDFLAGS)

$(TC41) :
	$(CC) -o $(TC41) $(TC41).c  $(CFLAGS) $(LDFLAGS)

$(TC42) :
	$(CC) -o $(TC42) $(TC42).c  $(CFLAGS) $(LDFLAGS)

$(TC43) :
	$(CC) -o $(TC43) $(TC43).c  $(CFLAGS) $(LDFLAGS)

$(TC44) :
	$(CC) -o $(TC44) $(TC44).c  $(CFLAGS) $(LDFLAGS)

$(TC45) :
	$(CC) -o $(TC45) $(TC45).c  $(CFLAGS) $(LDFLAGS)

$(TC46) :
	$(CC) -o $(TC46) $(TC46).c  $(CFLAGS) $(LDFLAGS)

$(TC47) :
	$(CC) -o $(TC47) $(TC47).c  $(CFLAGS) $(LDFLAGS)

$(TC48) :
	$(CC) -o $(TC48) $(TC48).c  $(CFLAGS) $(LDFLAGS)

$(TC49) :
	$(CC) -o $(TC49) $(TC49).c  $(CFLAGS) $(LDFLAGS)

$(TC50) :
	$(CC) -o $(TC50) $(TC50).c  $(CFLAGS) $(LDFLAGS)

$(TC51) :
	$(CC) -o $(TC51) $(TC51).c  $(CFLAGS) $(LDFLAGS)

$(TC52) :
	$(CC) -o $(TC52) $(TC52).c  $(CFLAGS) $(LDFLAGS)

$(TC53) :
	$(CC) -o $(TC53) $(TC53).c  $(CFLAGS) $(LDFLAGS)

$(TC54) :
	$(CC) -o $(TC54) $(TC54).c  $(CFLAGS) $(LDFLAGS)

$(TC55) :
	$(CC) -o $(TC55) $(TC55).c  $(CFLAGS) $(LDFLAGS)

$(TC56) :
	$(CC) -o $(TC56) $(TC56).c  $(CFLAGS) $(LDFLAGS)

$(TC57) :
	$(CC) -o $(TC57) $(TC57).c  $(CFLAGS) $(LDFLAGS)

$(TC58) :
	$(CC) -o $(TC58) $(TC58).c  $(CFLAGS) $(LDFLAGS)

$(TC59) :
	$(CC) -o $(TC59) $(TC59).c  $(CFLAGS) $(LDFLAGS)

$(TC60) :
	$(CC) -o $(TC60) $(TC60).c  $(CFLAGS) $(LDFLAGS)

$(TC61) :
	$(CC) -o $(TC61) $(TC61).c  $(CFLAGS) $(LDFLAGS)

$(TC62) :
	$(CC) -o $(TC62) $(TC62).c  $(CFLAGS) $(LDFLAGS)

$(TC63) :
	$(CC) -o $(TC63) $(TC63).c  $(CFLAGS) $(LDFLAGS)