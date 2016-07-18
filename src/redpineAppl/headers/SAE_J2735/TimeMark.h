/*
 * Generated by asn1c-0.9.27 (http://lionet.info/asn1c)
 * From ASN.1 module "DSRC"
 * 	found in "J2735.asn"
 */

#ifndef	_TimeMark_H_
#define	_TimeMark_H_


#include <SAE_J2735/asn_application.h>

/* Including external dependencies */
#include <SAE_J2735/NativeInteger.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TimeMark */
typedef long	 TimeMark_t;

/* Implementation */
extern asn_TYPE_descriptor_t asn_DEF_TimeMark;
asn_struct_free_f TimeMark_free;
asn_struct_print_f TimeMark_print;
asn_constr_check_f TimeMark_constraint;
ber_type_decoder_f TimeMark_decode_ber;
der_type_encoder_f TimeMark_encode_der;
xer_type_decoder_f TimeMark_decode_xer;
xer_type_encoder_f TimeMark_encode_xer;

#ifdef __cplusplus
}
#endif

#endif	/* _TimeMark_H_ */
#include <SAE_J2735/asn_internal.h>