This folder contains MIB module definition (source) files


MIB module definition (source) file V.S. MIB module database (compiled) file
----------------------------------------------------------------------------

A MIB module definition file is a plain ASCII text file written in MIB module definition language as specified by the SMI specification (SMIv1 or SMIv2). It contains a definition of MIB objects, their attributes and hierarchy. Such MIB definition files are typically supplied by the vendors of SNMP-manageable devices.

In order to use such a MIB module in an SNMP application, it must be compiled to a data format, which this application understands. In our case, this is the SMIDB binary format. This means that there will be typically two physical files for every MIB module, a source and a compiled MIB file.


To compile with MG-SOFT MIB Compiler:

Tools -> Scan for source files
Modules -> Select compiles -> Invert selection -> compile




CO-MIB1.smidb
 - asc
 - scp
 
NTCIP1202-2004.smidb




