  ############################################################################
  ## 
  ##  Makefile.am  
  ##
  ##  SNMP++ v3.3
  ##  -----------------------------------------------
  ##  Copyright (c) 2001-2013 Jochen Katz, Frank Fock
  ##
  ##  This software is based on SNMP++2.6 from Hewlett Packard:
  ##  
  ##    Copyright (c) 1996
  ##    Hewlett-Packard Company
  ##  
  ##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  ##  Permission to use, copy, modify, distribute andor sell this software 
  ##  andor its documentation is hereby granted without fee. User agrees 
  ##  to display the above copyright notice and this license notice in all 
  ##  copies of the software and any documentation of the software. User 
  ##  agrees to assume all liability for the use of the software; 
  ##  Hewlett-Packard and Jochen Katz make no representations about the 
  ##  suitability of this software for any purpose. It is provided 
  ##  "AS-IS" without warranty of any kind, either express or implied. User 
  ##  hereby grants a royalty-free license to any and all derivatives based
  ##  upon this software code base. 
  ##  
  ##########################################################################*

snmpppincdir = $(includedir)/snmp_pp

snmpppinc_HEADERS =	address.h \
			asn1.h \
			auth_priv.h \
			collect.h \
                        config_snmp_pp.h \
			counter.h \
			ctr64.h \
			eventlist.h \
			eventlistholder.h \
			gauge.h \
			idea.h \
			integer.h \
			IPv6Utility.h \
			log.h \
			Makefile.am \
			md5.h \
			mp_v3.h \
			msec.h \
			msgqueue.h \
			notifyqueue.h \
			octet.h \
			oid_def.h \
			oid.h \
			pdu.h \
			reentrant.h \
			sha.h \
			smi.h \
			smival.h \
			snmperrs.h \
			snmpmsg.h \
			snmp_pp.h \
			target.h \
			timetick.h \
			usm_v3.h \
			uxsnmp.h \
			v3.h \
			vb.h

install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(snmpppincdir)

EXTRA_DIST = config_snmp_pp.h.in

