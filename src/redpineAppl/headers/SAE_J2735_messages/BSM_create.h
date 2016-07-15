/****************************************************************************/
/// @file    BSM_create.h
/// @author
/// @author
/// @date
///
/****************************************************************************/
// @section LICENSE
//
// This software embodies materials and concepts that are confidential to Redpine
// Signals and is made available solely pursuant to the terms of a written license
// agreement with Redpine Signals
//

#ifndef __BSM_H__
#define __BSM_H__

#include "BasicSafetyMessage.h"
#include "utils.h"

typedef struct
{
    char				        *blob;
    vehiclesafetyExtension_t	*extention;  // op
}bsm_t;

#endif
