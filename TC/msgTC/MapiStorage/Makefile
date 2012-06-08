###################################################
# add your TestCase List Here
#
# e.g.,
# TC1 = utc_frameworkName_apiName_func
# TC2 = utc_ApplicationLib_recurGetDayOfWeek_func
TC1 = utc_msg_add_folder
TC2 = utc_msg_add_message
TC3 = utc_msg_get_conversation_view_list
TC4 = utc_msg_add_syncml_message
TC5 = utc_msg_get_folder_list
TC7 = utc_msg_get_folder_view_list
TC8 = utc_msg_get_mem_size
TC9 = utc_msg_get_message
TC10 = utc_msg_get_quick_panel_data
TC11 = utc_msg_get_thread_view_list
TC12 = utc_msg_move_msg_to_folder
TC13 = utc_msg_move_msg_to_storage
TC14 = utc_msg_release_folder_list
TC15 = utc_msg_release_thread_view_list
TC16 = utc_msg_reset_database
TC17 = utc_msg_count_message
TC19 = utc_msg_count_msg_by_contact
TC20 = utc_msg_thread_view_get_address
TC21 = utc_msg_count_msg_by_type
TC22 = utc_msg_thread_view_get_contact_id
TC23 = utc_msg_delete_all_msgs_in_folder
TC24 = utc_msg_thread_view_get_data
TC25 = utc_msg_delete_folder
TC26 = utc_msg_thread_view_get_direction
TC27 = utc_msg_delete_message
TC28 = utc_msg_thread_view_get_image_path
TC29 = utc_msg_delete_thread_message_list
TC30 = utc_msg_thread_view_get_mms_cnt
TC31 = utc_msg_thread_view_get_name
TC32 = utc_msg_thread_view_get_sms_cnt
TC33 = utc_msg_thread_view_get_thread_id
TC34 = utc_msg_thread_view_get_time
TC35 = utc_msg_thread_view_get_unread_cnt
TC36 = utc_msg_update_folder
TC37 = utc_msg_update_message
TC38 = utc_msg_update_protected_status
TC39 = utc_msg_update_read_status
TC40 = utc_msg_generate_message
	
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
TCLIST = $(TC1) $(TC2) $(TC3) $(TC4) $(TC5) $(TC7) $(TC8) $(TC9) $(TC10) $(TC11) $(TC12) $(TC13) $(TC14) $(TC15) $(TC16) $(TC17) $(TC19) $(TC20) $(TC21) $(TC22) $(TC23) $(TC24) $(TC25) $(TC26) $(TC27) $(TC28) $(TC29) $(TC30) $(TC31) $(TC32) $(TC33) $(TC34) $(TC35) $(TC36) $(TC37) $(TC38) $(TC39) $(TC40)
 
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
