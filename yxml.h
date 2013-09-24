/* Copyright (c) 2013 Yoran Heling

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdint.h>
#include <stddef.h>


typedef enum {
	YXML_EEOF        = -6, /* Unexpected EOF                             */
	YXML_EREF        = -5, /* Invalid character or entity reference (&whatever;) */
	YXML_ECLOSE      = -4, /* Close tag does not match open tag (<Tag> .. </OtherTag>) */
	YXML_ESTACK      = -3, /* Stack overflow (too deeply nested tags or too long element/attribute name) */
	YXML_EATTR       = -2, /* Too long attribute name                    */
	YXML_ESYN        = -1, /* Syntax error (unexpected byte)             */
	YXML_OK          =  0, /* Character consumed, no new token present   */
	YXML_ELEMSTART   =  1, /* Start of an element:   '<Tag ..'           */
	YXML_CONTENT     =  2, /* Start of element content '.. />' or '.. >' */
	YXML_ELEMSTCONT  =  3, /* Same as YXML_ELEMSTART|YXML_CONTENT  */
	YXML_ELEMEND     =  4, /* End of an element:     '.. />' or '</Tag>' */
	YXML_DATA        =  5, /* Attribute value or element/PI contents     */
	YXML_ATTRSTART   =  6, /* Attribute:             'Name=..'           */
	YXML_ATTREND     =  7, /* End of attribute       '.."'               */
	YXML_PISTART     =  8, /* Start of a processing instruction          */
	YXML_PIEND       =  9  /* End of a processing instruction            */
} yxml_ret_t;

/* When, exactly, are tokens returned?
 *
 * <TagName
 *   '>' ELEMSTART | CONTENT
 *   '/' ELEMSTART | CONTENT, '>' ELEMEND
 *   ' ' ELEMSTART
 *     '>' CONTENT
 *     '/' CONTENT, '>' ELEMEND
 *     Attr
 *       '=' ATTRSTART
 *         "X DATA
 *           'Y'  DATA
 *             'Z'  DATA
 *               '"' ATTREND
 *                 '>' CONTENT
 *                 '/' CONTENT, '>' ELEMEND
 *
 * </TagName
 *   '>' ELEMEND
 */


typedef struct {
	/* PUBLIC (read-only) */

	/* Name of the current element, zero-length if not in any element. Changed
	 * after YXML_ELEMSTART. The pointer will remain valid up to and including
	 * the next YXML_CONTENT, the pointed-to buffer will remain valid up to and
	 * including the YXML_ELEMCLOSE for the corresponding element. */
	char *elem;

	/* The last read character(s) of an attribute value, element data, or
	 * processing instruction. Changed after YXML_DATA and only valid until the
	 * next yxml_parse() call. Usually, this string only consists of a single
	 * character, but multiple characters are returned in the following cases:
	 * - "<?SomePI ?x ?>": The two characters "?x"
	 * - "<![CDATA[ ]x ]]>": The two characters "]x"
	 * - "<![CDATA[ ]]x ]]>": The three characters "]]x"
	 */
	char data[8];

	/* Name of the current attribute. Changed after YXML_ATTRSTART, valid up to
	 * and including the next YXML_ATTREND. */
	char *attr;

	/* Name/target of the current processing instruction, zero-length if not in
	 * a PI. Changed after YXML_PISTART, valid up to (but excluding)
	 * the next YXML_PIEND. */
	char *pi;

	/* Line number, byte offset within that line, and total bytes read. These
	 * values refer to the position _after_ the last byte given to
	 * yxml_parse(). These are useful for debugging and error reporting. */
	uint64_t byte;
	uint64_t total;
	uint32_t line;


	/* PRIVATE */
	int state;
	unsigned char *stack; /* Stack of element names + attribute/PI name, separated by \0. Also starts with a \0. */
	size_t stacksize, stacklen;
	unsigned reflen;
	unsigned quote;
	int nextstate; /* Used for '@' state remembering and for the "string" consuming state */
	unsigned ignore;
	unsigned char *string;
} yxml_t;


void yxml_init(yxml_t *x, char *stack, size_t stacksize);


yxml_ret_t yxml_parse(yxml_t *x, int ch);


/* May be called after the last character has been given to yxml_parse().
 * Returns YXML_OK if the XML document is valid, YXML_EEOF otherwise.  Using
 * this function isn't really necessary, but can be used to detect documents
 * that don't end correctly. In particular, an error is returned when the XML
 * document did not contain a (complete) root element, or when the document
 * ended while in a comment or processing instruction. */
yxml_ret_t yxml_eof(yxml_t *x);


/* vim: set noet sw=4 ts=4: */
