###################################################
# add your TestCase List Here
#
# e.g.,
# TC1 = utc_frameworkName_apiName_func
# TC2 = utc_ApplicationLib_recurGetDayOfWeek_func
TC1 = utc_msg_mms_forward_message
TC2 = utc_msg_reg_lbs_message_callback
TC3 = utc_msg_sms_send
TC4 = utc_msg_mms_reject_message
TC5 = utc_msg_reg_mms_conf_message_callback
TC6 = utc_msg_sms_send_message
TC7 = utc_msg_mms_retrieve_message
TC8 = utc_msg_reg_sent_status_callback
TC9 = utc_msg_submit_req
TC10 = utc_msg_mms_send_message
TC11 = utc_msg_reg_sms_message_callback
TC12 = utc_msg_mms_send_read_report
TC13 = utc_msg_reg_syncml_message_callback

	
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
TCLIST = $(TC1) $(TC2) $(TC3) $(TC4) $(TC5) $(TC6) $(TC7) $(TC8) $(TC9) $(TC10) $(TC11) $(TC12) $(TC13)
 
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