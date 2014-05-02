/* The Halfling Project - A Graphics Engine and Projects
 *
 * The Halfling Project is the legal property of Adrian Astley
 * Copyright Adrian Astley 2013 - 2014
 */

#ifndef COMMON_ENDIAN_H
#define COMMON_ENDIAN_H


/**
 * A wrapper macro used around four character constants, like 'DATA', to
 * ensure portability. Typical usage: MKTAG('D','A','T','A').
 *
 * Why is this necessary? The C/C++ standard does not define the endianess to
 * be used for character constants. Hence if one uses multi-byte character
 * constants, a potential portability problem opens up.
 */
#define MKTAG(a0,a1,a2,a3) ((uint32)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

/**
 * A wrapper macro used around two character constants, like 'wb', to
 * ensure portability. Typical usage: MKTAG16('w','b').
 */
#define MKTAG16(a0,a1) ((uint16)((a1) | ((a0) << 8)))


#endif
