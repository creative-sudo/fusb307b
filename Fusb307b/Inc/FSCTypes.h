/*******************************************************************************
 * @file     FSCTypes.h
 * @author   USB PD Firmware Team
 *
 * Copyright 2018 ON Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at
 * http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
 * ("ON Semiconductor Standard Terms and Conditions of Sale,
 *   Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 ******************************************************************************/
#ifndef _FSCTYPES_H_
#define _FSCTYPES_H_

#if !defined(__PACKED)
    #define __PACKED
#endif

/* Number of ports supported by the port manager */
#ifdef FSC_HAVE_MULTIPORT
#define FSC_NUMBER_OF_PORTS 3
#else
#define FSC_NUMBER_OF_PORTS 1
#endif /* FSC_HAVE_MULTIPORT */

typedef enum _BOOL { FALSE = 0, TRUE } FSC_BOOL;

typedef signed char         FSC_S8;
typedef signed short int    FSC_S16;
typedef signed long int     FSC_S32;

typedef unsigned char       FSC_U8;
typedef unsigned short int  FSC_U16;
typedef unsigned long int   FSC_U32;


#endif /* _FSCTYPES_H_ */
