###################################################
# add your TestCase List Here
#
# e.g.,
# TC1 = utc_frameworkName_apiName_func
# TC2 = utc_ApplicationLib_recurGetDayOfWeek_func
TC1 = utc_msg_get_config
TC2 = utc_msg_set_config
	
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
TCLIST = $(TC1) $(TC2) 
 
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
