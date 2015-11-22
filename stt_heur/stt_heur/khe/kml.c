
/*****************************************************************************/
/*                                                                           */
/*  THE KTS TIMETABLING SYSTEM                                               */
/*  COPYRIGHT (C) 2004, 2008 Jeffrey H. Kingston                             */
/*                                                                           */
/*  Jeffrey H. Kingston (jeff@it.usyd.edu.au)                                */
/*  School of Information Technologies                                       */
/*  The University of Sydney 2006                                            */
/*  AUSTRALIA                                                                */
/*                                                                           */
/*  FILE:         kml.c                                                      */
/*  MODULE:       XML reading and writing                                    */
/*                                                                           */
/*****************************************************************************/
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <expat.h>
#include "kml.h"
#include "m.h"

#define BUFF_SIZE 1024
#define KML_MAX_STR 200
#define DEBUG1 0
#define DEBUG2 0


/*****************************************************************************/
/*                                                                           */
/*  kml_file_rec - an XML direct write file                                  */
/*                                                                           */
/*****************************************************************************/

struct kml_file_rec {
  FILE			*fp;			/* file to write XML to      */
  int			curr_indent;		/* current indent            */
  int			indent_step;		/* indent step               */
  bool			attribute_allowed;	/* state of print            */
};


/*****************************************************************************/
/*                                                                           */
/*  kml_elt_rec - an XML element                                             */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KML_ELT) ARRAY_KML_ELT;

struct kml_elt_rec {
  int			line_num;		/* line number of element    */
  int			col_num;		/* column number of element  */
  char			*label;			/* label of element          */
  KML_ELT		parent;			/* parent of element         */
  ARRAY_STRING		attribute_names;	/* attribute names           */
  ARRAY_STRING		attribute_values;	/* attribute values          */
  ARRAY_KML_ELT		children;		/* children                  */
  char			*text;			/* text                      */
};


/*****************************************************************************/
/*                                                                           */
/*  KML_ERROR                                                                */
/*                                                                           */
/*****************************************************************************/

struct kml_error_rec {
  int			line_num;		/* line number of error      */
  int			col_num;		/* column number of error    */
  char			string[KML_MAX_STR];	/* error string              */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "KML_ERROR"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KmlErrorMake(KML_ERROR *ke, int line_num, int col_num,              */
/*    char *fmt, ...)                                                        */
/*                                                                           */
/*  Make a KML error object unless ke is NULL.  For convenience when using   */
/*  this function, *ke is set to the new object, and false is returned.      */
/*                                                                           */
/*****************************************************************************/

bool KmlErrorMake(KML_ERROR *ke, int line_num, int col_num, char *fmt, ...)
{
  va_list ap;
  if( ke != NULL )
  {
    MMake(*ke);
    (*ke)->line_num = line_num;
    (*ke)->col_num = col_num;
    va_start(ap, fmt);
    vsnprintf((*ke)->string, KML_MAX_STR, fmt, ap);
    va_end(ap);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlErrorLineNum(KML_ERROR ke)                                        */
/*                                                                           */
/*  Return the line number attribute of error ke.                            */
/*                                                                           */
/*****************************************************************************/

int KmlErrorLineNum(KML_ERROR ke)
{
  return ke->line_num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlErrorColNum(KML_ERROR ke)                                         */
/*                                                                           */
/*  Return the column number attribute of error ke.                          */
/*                                                                           */
/*****************************************************************************/

int KmlErrorColNum(KML_ERROR ke)
{
  return ke->col_num;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlErrorString(KML_ERROR ke)                                       */
/*                                                                           */
/*  Return the string attribute of error ke.                                 */
/*                                                                           */
/*****************************************************************************/

char *KmlErrorString(KML_ERROR ke)
{
  return ke->string;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "KML_FILE writing (not involving KML_ELT objects)"             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KML_FILE KmlMakeFile(FILE *fp, int indent_step, int initial_indent)      */
/*                                                                           */
/*****************************************************************************/

KML_FILE KmlMakeFile(FILE *fp, int initial_indent, int indent_step)
{
  KML_FILE res;
  res = (KML_FILE) malloc(sizeof(struct kml_file_rec));
  res->fp = fp;
  res->curr_indent = initial_indent;
  res->indent_step = indent_step;
  res->attribute_allowed = false;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlBegin(KML_FILE kf, char *label)                                  */
/*                                                                           */
/*  Begin printing an element with this label.                               */
/*                                                                           */
/*****************************************************************************/

void KmlBegin(KML_FILE kf, char *label)
{
  if( kf->attribute_allowed )
    fprintf(kf->fp, ">\n");
  fprintf(kf->fp, "%*s<%s", kf->curr_indent, "", label);
  kf->curr_indent += kf->indent_step;
  kf->attribute_allowed = true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAttribute(KML_FILE kf, char *name, char *value)                  */
/*                                                                           */
/*  Add an attribute to kf.                                                  */
/*                                                                           */
/*****************************************************************************/

void KmlAttribute(KML_FILE kf, char *name, char *value)
{
  assert(kf->attribute_allowed);
  fprintf(kf->fp, " %s=\"%s\"", name, value);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlPrintf(KML_FILE kf, char *fmt, ...)                              */
/*                                                                           */
/*  Like printf but for kf.                                                  */
/*                                                                           */
/*****************************************************************************/

void KmlPrintf(KML_FILE kf, char *fmt, ...)
{
  va_list args;
  if( kf->attribute_allowed )
    fprintf(kf->fp, ">");
  va_start(args, fmt);
  vfprintf(kf->fp, fmt, args);
  va_end(args);
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEnd(KML_FILE kf, char *label)                                    */
/*                                                                           */
/*  End print of category label.                                             */
/*                                                                           */
/*****************************************************************************/

void KmlEnd(KML_FILE kf, char *label)
{
  kf->curr_indent -= kf->indent_step;
  if( kf->attribute_allowed )
    fprintf(kf->fp, "/>\n");
  else
    fprintf(kf->fp, "%*s</%s>\n", kf->curr_indent, "", label);
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEndNoIndent(KML_FILE kf, char *label)                            */
/*                                                                           */
/*  Like KmlEnd but with no indent.                                          */
/*                                                                           */
/*****************************************************************************/

void KmlEndNoIndent(KML_FILE kf, char *label)
{
  kf->curr_indent -= kf->indent_step;
  if( kf->attribute_allowed )
    fprintf(kf->fp, "/>\n");
  else
    fprintf(kf->fp, "</%s>\n", label);
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "complete elements which are simple combinations of the above" */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KmlEltAttribute(KML_FILE kf, char *label, char *name, char *value)  */
/*                                                                           */
/*  Print category label with one attribute and no children or text.         */
/*                                                                           */
/*****************************************************************************/

void KmlEltAttribute(KML_FILE kf, char *label, char *name, char *value)
{
  KmlBegin(kf, label);
  KmlAttribute(kf, name, value);
  KmlEnd(kf, label);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEltPrintf(KML_FILE kf, char *label, char *fmt, ...)              */
/*                                                                           */
/*  Printf within category label.                                            */
/*                                                                           */
/*  Implementation note.   This is basically                                 */
/*                                                                           */
/*    KmlBegin(kf, label);                                                   */
/*    KmlPrintf(kf, fmt, ...);                                               */
/*    KmlEnd(kf, label);                                                     */
/*                                                                           */
/*  but all on one line.                                                     */
/*                                                                           */
/*****************************************************************************/

void KmlEltPrintf(KML_FILE kf, char *label, char *fmt, ...)
{
  va_list args;
  KmlBegin(kf, label);
  if( kf->attribute_allowed )
    fprintf(kf->fp, ">");
  va_start(args, fmt);
  vfprintf(kf->fp, fmt, args);
  va_end(args);
  kf->attribute_allowed = false;
  KmlEndNoIndent(kf, label);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlEltAttributeEltPrintf(KML_FILE kf, char *label, char *name,      */
/*    char *value, char *label2, char *fmt, ...)                             */
/*                                                                           */
/*  Equivalent to                                                            */
/*                                                                           */
/*    KmlEltAttribute(kf, label, name, value);                               */
/*    KmlEltPrintf(kf, label2, fmt, ...);                                    */
/*                                                                           */
/*  only all on one line.                                                    */
/*                                                                           */
/*****************************************************************************/

void KmlEltAttributeEltPrintf(KML_FILE kf, char *label, char *name,
  char *value, char *label2, char *fmt, ...)
{
  va_list args;
  KmlBegin(kf, label);
  KmlAttribute(kf, name, value);
  fprintf(kf->fp, "><%s>", label2);
  va_start(args, fmt);
  vfprintf(kf->fp, fmt, args);
  va_end(args);
  fprintf(kf->fp, "</%s></%s>\n", label2, label);
  kf->curr_indent -= kf->indent_step;
  kf->attribute_allowed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "string handling (private)"                                    */
/*                                                                           */
/*****************************************************************************/
#define is_space(ch) ((ch)=='\n' || (ch)=='\r' || (ch)=='\t' || (ch)==' ')
#define is_digit(ch) ((ch) >= '0' && ch <= '9')

/*****************************************************************************/
/*                                                                           */
/*  char *KmlStringCopy(const char *str)                                     */
/*                                                                           */
/*  Return a copy of str in malloced memory.                                 */
/*                                                                           */
/*****************************************************************************/

char *KmlStringCopy(const char *str)
{
  char *res;
  res = (char *) malloc((strlen(str) + 1) * sizeof(char));
  strcpy(res, str);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlStringHasNonWhite(const char *str, int len)                      */
/*                                                                           */
/*  Return true if str has non-white spaces in it.                           */
/*                                                                           */
/*****************************************************************************/

static bool KmlStringHasNonWhite(const char *str, int len)
{
  int i;
  for( i = 0;  i < len;  i++ )
    if( !is_space(str[i]) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlStringContainsOneIntegerOnly(char *s)                            */
/*                                                                           */
/*  Return true if s contains exactly one integer, possibly with white space.*/
/*                                                                           */
/*****************************************************************************/

static bool KmlStringContainsOneIntegerOnly(char *s)
{
  char *p;
  if( s == NULL )
    return false;
  
  /* skip zero or more preceding spaces */
  for( p = s;  is_space(*p);  p++ );

  /* skip one or more digits */
  if( !is_digit(*p) )
    return false;
  do { p++; } while( is_digit(*p) );

  /* skip zero or more following spaces */
  while( is_space(*p) )  p++;

  /* that must be the end */
  return *p == '\0';
}




/*****************************************************************************/
/*                                                                           */
/*  Submodule "KML_ELT construction"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KML_ELT KmlMakeElt(int line_num, int col_num, char *label)               */
/*                                                                           */
/*  Make a new XML element with these attributes.  Initially the node        */
/*  has no parent, but if it is added as a child by KmlAddChild it will      */
/*  get a parent then.                                                       */
/*                                                                           */
/*****************************************************************************/

KML_ELT KmlMakeElt(int line_num, int col_num, char *label)
{
  KML_ELT res;
  res = (KML_ELT) malloc(sizeof(struct kml_elt_rec));
  res->line_num = line_num;
  res->col_num = col_num;
  res->label = label;
  res->parent = NULL;
  MArrayInit(res->attribute_names);
  MArrayInit(res->attribute_values);
  MArrayInit(res->children);
  res->text = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAddAttribute(KML_ELT elt, char *name, char *value)               */
/*                                                                           */
/*  Add an attribute with this name and value to elt.                        */
/*                                                                           */
/*****************************************************************************/

void KmlAddAttribute(KML_ELT elt, char *name, char *value)
{
  assert(name != NULL && strlen(name)>0 && value != NULL && strlen(value)>0);
  MArrayAddLast(elt->attribute_names, name);
  MArrayAddLast(elt->attribute_values, value);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAddChild(KML_ELT elt, KML_ELT child)                             */
/*                                                                           */
/*  Add this child to elt.                                                   */
/*                                                                           */
/*****************************************************************************/

void KmlAddChild(KML_ELT elt, KML_ELT child)
{
  if( DEBUG2 )
  {
    int i;  KML_ELT c;
    fprintf(stderr, "[ KmlAddChild(%d:%d:<%s>, %d:%d:<%s>)\n",
      elt->line_num, elt->col_num, elt->label,
      child->line_num, child->col_num, child->label);
    fprintf(stderr, "  %d children:\n", MArraySize(elt->children));
    MArrayForEach(elt->children, &c, &i)
      fprintf(stderr, "    %d: %s\n", i, c->label);
  }
  assert(elt->text == NULL);
  MArrayAddLast(elt->children, child);
  child->parent = elt;
  if( DEBUG2 )
    fprintf(stderr, "] KmlAddChild returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlDeleteChild(KML_ELT elt, KML_ELT child)                          */
/*                                                                           */
/*  Delete child from elt.  It must be present.                              */
/*                                                                           */
/*****************************************************************************/

void KmlDeleteChild(KML_ELT elt, KML_ELT child)
{
  int pos;
  if( !MArrayContains(elt->children, child, &pos) )
    assert(false);
  MArrayRemove(elt->children, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAddTextLen(KML_ELT elt, const char *text, int len)               */
/*                                                                           */
/*  Add text, which is known to have the given length, to any text that      */
/*  is already present.  There may be no children.                           */
/*                                                                           */
/*  If the string to be added contains nothing but white space, then it      */
/*  is not added at all.                                                     */
/*                                                                           */
/*****************************************************************************/

static void KmlAddTextLen(KML_ELT elt, const char *text, int len)
{
  int curr_len;
  if( KmlStringHasNonWhite(text, len) )
  {
    assert(MArraySize(elt->children) == 0);
    curr_len = (elt->text != NULL ? strlen(elt->text) : 0);
    elt->text = (char *) realloc(elt->text, (curr_len+len+1) * sizeof(char));
    strncpy(&elt->text[curr_len], text, len);
    elt->text[curr_len + len] = '\0';
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlAddText(KML_ELT elt, char *text)                                 */
/*                                                                           */
/*  Add text to elt, appending it to any text already present.  There may    */
/*  not be any children beforehand.                                          */
/*                                                                           */
/*****************************************************************************/

void KmlAddText(KML_ELT elt, char *text)
{
  KmlAddTextLen(elt, text, strlen(text));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "KML_ELT query"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KmlLineNum(KML_ELT elt)                                              */
/*                                                                           */
/*  Return the line number of elt.                                           */
/*                                                                           */
/*****************************************************************************/

int KmlLineNum(KML_ELT elt)
{
  return elt->line_num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlColNum(KML_ELT elt)                                               */
/*                                                                           */
/*  Return the column number of elt.                                         */
/*                                                                           */
/*****************************************************************************/

int KmlColNum(KML_ELT elt)
{
  return elt->col_num;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlLabel(KML_ELT elt)                                              */
/*                                                                           */
/*  Return the label of elt.                                                 */
/*                                                                           */
/*****************************************************************************/

char *KmlLabel(KML_ELT elt)
{
  return elt->label;
}


/*****************************************************************************/
/*                                                                           */
/*  KML_ELT KmlParent(KML_ELT elt)                                           */
/*                                                                           */
/*  Return the parent of elt.  This may be NULL.                             */
/*                                                                           */
/*****************************************************************************/

KML_ELT KmlParent(KML_ELT elt)
{
  return elt->parent;
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlAttributeCount(KML_ELT elt)                                       */
/*                                                                           */
/*  Return the number of attributes of elt.                                  */
/*                                                                           */
/*****************************************************************************/

int KmlAttributeCount(KML_ELT elt)
{
  return MArraySize(elt->attribute_names);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlAttributeName(KML_ELT elt, int index)                           */
/*                                                                           */
/*  Return the name of the index'th attribute of elt, where                  */
/*  0 <= index <= KmlAttributeCount(elt).                                    */
/*                                                                           */
/*****************************************************************************/

char *KmlAttributeName(KML_ELT elt, int index)
{
  if( index < 0 )
    index = KmlAttributeCount(elt) + index;
  assert(index < MArraySize(elt->attribute_names));
  return MArrayGet(elt->attribute_names, index);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlAttributeValue(KML_ELT elt, int index)                          */
/*                                                                           */
/*  Return the value of the index'th attribute of elt, where                 */
/*  0 <= index <= KmlAttributeCount(elt).                                    */
/*                                                                           */
/*****************************************************************************/

char *KmlAttributeValue(KML_ELT elt, int index)
{
  if( index < 0 )
    index = KmlAttributeCount(elt) + index;
  assert(index < MArraySize(elt->attribute_values));
  return MArrayGet(elt->attribute_values, index);
}


/*****************************************************************************/
/*                                                                           */
/*  int KmlChildCount(KML_ELT elt)                                           */
/*                                                                           */
/*  Return the number of children of elt.                                    */
/*                                                                           */
/*****************************************************************************/

int KmlChildCount(KML_ELT elt)
{
  return MArraySize(elt->children);
}


/*****************************************************************************/
/*                                                                           */
/*  KML_ELT KmlChild(KML_ELT elt, int index)                                 */
/*                                                                           */
/*  Return the index'th child of elt, where 0 <= index < KmlChildCount(elt), */
/*  or if index is negative, from the back (-1 means last, etc.).            */
/*                                                                           */
/*****************************************************************************/

KML_ELT KmlChild(KML_ELT elt, int index)
{
  if( index < 0 )
    index = KmlChildCount(elt) + index;
  assert(index < MArraySize(elt->children));
  return MArrayGet(elt->children, index);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlContainsChild(KML_ELT elt, char *label, KML_ELT *child_elt)      */
/*                                                                           */
/*  If elt contains at least one child with this label, return true and      */
/*  set *child_elt to the first such child.  Otherwise return false and      */
/*  set *child_elt to NULL.                                                  */
/*                                                                           */
/*****************************************************************************/

bool KmlContainsChild(KML_ELT elt, char *label, KML_ELT *child_elt)
{
  KML_ELT e;  int i;
  MArrayForEach(elt->children, &e, &i)
    if( strcmp(e->label, label) == 0 )
    {
      *child_elt = e;
      return true;
    }
  *child_elt = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlText(KML_ELT elt)                                               */
/*                                                                           */
/*  Return the text of elt.  This could be NULL.                             */
/*                                                                           */
/*****************************************************************************/

char *KmlText(KML_ELT elt)
{
  return elt->text;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "KML_ELT reading and writing"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void CharacterDataHandler(void *userData, const XML_Char *s, int len)    */
/*                                                                           */
/*  Character data handler.                                                  */
/*                                                                           */
/*****************************************************************************/

static void CharacterDataHandler(void *userData, const XML_Char *s, int len)
{
  XML_Parser p;  KML_ELT elt;
  p = (XML_Parser) userData;
  elt = XML_GetUserData(p);
  KmlAddTextLen(elt, s, len);
}


/*****************************************************************************/
/*                                                                           */
/*  void StartElementHandler(void *userData, const XML_Char *name,           */
/*     const XML_Char **atts)                                                */
/*                                                                           */
/*  Handler for starting an element.                                         */
/*                                                                           */
/*****************************************************************************/

static void StartElementHandler(void *userData, const XML_Char *name,
   const XML_Char **atts)
{
  XML_Parser p;  KML_ELT parent, child;  int i;

  /* get parent */
  p = (XML_Parser) userData;
  parent = XML_GetUserData(p);

  /* create child and add to parent */
  child = KmlMakeElt(XML_GetCurrentLineNumber(p),
    XML_GetCurrentColumnNumber(p)+1, KmlStringCopy(name));
  KmlAddChild(parent, child);

  /* add attributes to child */
  for( i = 0;  atts[i] != NULL;  i += 2 )
    KmlAddAttribute(child, KmlStringCopy(atts[i]), KmlStringCopy(atts[i+1]));

  /* set user data to child */
  XML_SetUserData(p, (void *) child);
}


/*****************************************************************************/
/*                                                                           */
/*  void EndElementHandler(void *userData, const XML_Char *name)             */
/*                                                                           */
/*  Handler for ending an element.                                           */
/*                                                                           */
/*****************************************************************************/

static void EndElementHandler(void *userData, const XML_Char *name)
{
  XML_Parser p;  KML_ELT elt;  int i;
  p = (XML_Parser) userData;
  elt = XML_GetUserData(p);
  assert(elt != NULL);
  XML_SetUserData(p, (void *) elt->parent);

  /* remove trailing white space from text */
  if( elt->text != NULL )
  {
    for( i = strlen(elt->text) - 1;  i >= 0 && is_space(elt->text[i]);  i-- );
      elt->text[i+1] = '\0';
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlRead(FILE *fp, KML_ELT *res, KML_ERROR *ke)                      */
/*                                                                           */
/*  Read an XML element from fp.  If successful, set *res to the element     */
/*  and return true.  Otherwise set *ke to an error object describing the    */
/*  first problem encountered, and return false.                             */
/*                                                                           */
/*****************************************************************************/

bool KmlRead(FILE *fp, KML_ELT *res, KML_ERROR *ke)
{
  int bytes_read;  void *buff;  XML_Parser p;  KML_ELT root, final_root;

  /* set up the expat parser */
  p = XML_ParserCreate(NULL);
  XML_UseParserAsHandlerArg(p);
  XML_SetElementHandler(p, &StartElementHandler, &EndElementHandler);
  XML_SetCharacterDataHandler(p, &CharacterDataHandler);
  root = KmlMakeElt(0, 0, "Root");
  XML_SetUserData(p, (void *) root);

  /* read the file */
  *res = NULL;
  do
  {
    buff = XML_GetBuffer(p, BUFF_SIZE);
    assert(buff != NULL);

    bytes_read = fread(buff, sizeof(char), BUFF_SIZE, fp);
    if( !XML_ParseBuffer(p, bytes_read, bytes_read == 0) )
      return KmlErrorMake(ke, XML_GetCurrentLineNumber(p),
	XML_GetCurrentColumnNumber(p), "%s",
	XML_ErrorString(XML_GetErrorCode(p)));
  } while( bytes_read > 0 );

  /* check the result */
  final_root = XML_GetUserData(p);
  if( final_root != root )
    return KmlErrorMake(ke, XML_GetCurrentLineNumber(p),
      XML_GetCurrentColumnNumber(p), "input file terminated early");
  if( MArraySize(root->children) != 1 )
    return KmlErrorMake(ke, XML_GetCurrentLineNumber(p),
      XML_GetCurrentColumnNumber(p), "%d outer elements in input file",
      MArraySize(root->children));

  /* return the first child as result */
  *res = MArrayFirst(root->children);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlReadString(char *str, KML_ELT *res, KML_ERROR *ke)               */
/*                                                                           */
/*  Like KmlRead just above, but reading from a string, not from a file.     */
/*                                                                           */
/*****************************************************************************/

bool KmlReadString(char *str, KML_ELT *res, KML_ERROR *ke)
{
  XML_Parser p;  KML_ELT root, final_root;

  /* set up the expat parser */
  p = XML_ParserCreate(NULL);
  XML_UseParserAsHandlerArg(p);
  XML_SetElementHandler(p, &StartElementHandler, &EndElementHandler);
  XML_SetCharacterDataHandler(p, &CharacterDataHandler);
  root = KmlMakeElt(0, 0, "Root");
  XML_SetUserData(p, (void *) root);

  /* parse the string */
  *res = NULL;
  if( !XML_Parse(p, str, strlen(str), true) )
    return KmlErrorMake(ke, XML_GetCurrentLineNumber(p),
      XML_GetCurrentColumnNumber(p), "%s",
      XML_ErrorString(XML_GetErrorCode(p)));

  /* check the result */
  final_root = XML_GetUserData(p);
  if( final_root != root )
    return KmlErrorMake(ke, XML_GetCurrentLineNumber(p),
      XML_GetCurrentColumnNumber(p), "input string terminated early");
  if( MArraySize(root->children) != 1 )
    return KmlErrorMake(ke, XML_GetCurrentLineNumber(p),
      XML_GetCurrentColumnNumber(p), "%d outer elements in input string",
      MArraySize(root->children));

  /* return the first child as result */
  *res = MArrayFirst(root->children);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "verification"                                                 */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  TERM_COLON,
  TERM_COMPULSORY,
  TERM_OPTIONAL,
  TERM_SEQUENCE
} TERM_TYPE;

typedef enum {
  FACTOR_NO_TEXT,
  FACTOR_OPTIONAL_TEXT,
  FACTOR_INTEGER_TEXT
} FACTOR_TYPE;


/*****************************************************************************/
/*                                                                           */
/*  bool NextTerm(char **fmt, char *name, TERM_TYPE *et, FACTOR_TYPE *ft)    */
/*                                                                           */
/*  If *fmt contains a next term, set name, *et, and *ft to its              */
/*  attributes, move *fmt past the term ready to read the next term.        */
/*  and return true.  If there is no next term, return false.                */
/*                                                                           */
/*****************************************************************************/

static bool NextTerm(char **fmt, char *name, TERM_TYPE *et, FACTOR_TYPE *ft)
{
  /* skip initial spaces and return false if exhausted */
  while( **fmt == ' ')
    (*fmt)++;
  if( **fmt == '\0' )
    return false;

  /* colon is special case */
  if( **fmt == ':' )
  {
    *et = TERM_COLON;
    (*fmt)++;
    return true;
  }

  /* optional + or * */
  if( **fmt == '+' )
  {
    *et = TERM_OPTIONAL;
    (*fmt)++;
  }
  else if( **fmt == '*' )
  {
    *et = TERM_SEQUENCE;
    (*fmt)++;
  }
  else
    *et = TERM_COMPULSORY;

  /* optional $ or # */
  if( **fmt == '$' )
  {
    *ft = FACTOR_OPTIONAL_TEXT;
    (*fmt)++;
  }
  if( **fmt == '#' )
  {
    *ft = FACTOR_INTEGER_TEXT;
    (*fmt)++;
  }
  else
    *ft = FACTOR_NO_TEXT;

  /* label proper */
  sscanf(*fmt, "%s", name);
  *fmt += strlen(name);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool NextAttribute(KML_ELT elt, int *index, char *name, char *val)       */
/*                                                                           */
/*  If elt contains a next attribute (at *index, that is) with the given     */
/*  name, set *val to its value, move *index past it ready for the next      */
/*  attribute, and return true.  Otherwise return false.                     */
/*                                                                           */
/*****************************************************************************/

static bool NextAttribute(KML_ELT elt, int *index, char *name, char **val)
{
  if( *index < KmlAttributeCount(elt) &&
      strcmp(MArrayGet(elt->attribute_names, *index), name) == 0 )
  {
    *val = MArrayGet(elt->attribute_values, *index);
    (*index)++;
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool NextChild(KML_ELT elt, int *index, char *name, KML_ELT *child)      */
/*                                                                           */
/*  If elt contains a next child (at *index, that is) with the given         */
/*  name, set *child to that child, move *index past it ready for the next   */
/*  child, and return true.  Otherwise return false.                         */
/*                                                                           */
/*****************************************************************************/

static bool NextChild(KML_ELT elt, int *index, char *name, KML_ELT *child)
{
  if( *index < KmlChildCount(elt) &&
      strcmp(KmlLabel(MArrayGet(elt->children, *index)), name) == 0 )
  {
    *child = MArrayGet(elt->children, *index);
    (*index)++;
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KmlCheck(KML_ELT elt, char *fmt, KML_ERROR *ke)                     */
/*                                                                           */
/*  If the attributes and children of elt conform to fmt, return true.       */
/*  Otherwise, return false and set *ke to an error object describing the    */
/*  first problem encountered.                                               */
/*                                                                           */
/*  Each fmt string is a sequences of zero or more terms separated by        */
/*  one or more space characters:                                            */
/*                                                                           */
/*    fmt ::= { term } [ ":" { term } ]                                      */
/*                                                                           */
/*  Terms before the colon (if any) describe attributes; terms after the     */
/*  colon describe children.  Each term may be any one of                    */
/*                                                                           */
/*    term ::= factor               Exactly one must be present              */
/*    term ::= "+" factor           Zero or one must be present              */
/*    term ::= "*" factor           Zero or more must be present             */
/*                                                                           */
/*  Each factor may be any one of                                            */
/*                                                                           */
/*    factor ::= label              No text allowed                          */
/*    factor ::= "$" label          Text may be present, children may not    */
/*    factor ::= "#" label          Text denoting integer must be present    */
/*                                                                           */
/*  There may be no spaces within terms and factors.                         */
/*                                                                           */
/*****************************************************************************/

#define test_attribute()						\
{									\
  if( ft == FACTOR_INTEGER_TEXT &&					\
      !KmlStringContainsOneIntegerOnly(val) )				\
    return KmlErrorMake(ke, elt->line_num, elt->col_num,		\
      "in <%s>, attribute %s does not have integer value",		\
      elt->label, name);						\
}

#define test_child()							\
{									\
  if( ft != FACTOR_NO_TEXT && KmlChildCount(child) > 0 )		\
    return KmlErrorMake(ke, child->line_num, child->col_num,		\
      "child <%s> of <%s> has unexpected children",			\
      child->label, elt->label);					\
  if( ft == FACTOR_INTEGER_TEXT &&					\
      !KmlStringContainsOneIntegerOnly(child->text) )			\
    return KmlErrorMake(ke, child->line_num, child->col_num,		\
      "child <%s> of <%s> does not have integer value",			\
      child->label, elt->label, child->text);				\
}


bool KmlCheck(KML_ELT elt, char *fmt, KML_ERROR *ke)
{
  int index;  char *p, *val;
  char name[200];  TERM_TYPE et;  FACTOR_TYPE ft;  KML_ELT child;
  if( DEBUG1 )
    fprintf(stderr, "[ KmlCheck(elt, \"%s\")\n", fmt);

  /* check attributes */
  p = fmt;  index = 0;
  while( NextTerm(&p, name, &et, &ft) && et != TERM_COLON ) switch( et )
  {
    case TERM_COMPULSORY:

      if( !NextAttribute(elt, &index, name, &val) )
	return KmlErrorMake(ke, elt->line_num, elt->col_num,
	  "in <%s>, attribute %s missing or out of order", elt->label, name);
      test_attribute();
      break;

    case TERM_OPTIONAL:

      if( NextAttribute(elt, &index, name, &val) )
	test_attribute();
      break;

    case TERM_SEQUENCE:

      while( NextAttribute(elt, &index, name, &val) )
	test_attribute();
      break;

    default:

      assert(false);
  }
  if( index < KmlAttributeCount(elt) )
    return KmlErrorMake(ke, elt->line_num, elt->col_num,
      "in <%s>, unexpected attribute %s", elt->label,
      MArrayGet(elt->attribute_names, index));

  /* check children */
  index = 0;
  while( NextTerm(&p, name, &et, &ft) )  switch( et )
  {
    case TERM_COMPULSORY:

      if( !NextChild(elt, &index, name, &child) )
	return KmlErrorMake(ke, elt->line_num, elt->col_num,
	  "in <%s>, child <%s> missing or out of order", elt->label, name);
      test_child();
      break;

    case TERM_OPTIONAL:

      if( NextChild(elt, &index, name, &child) )
	test_child();
      break;

    case TERM_SEQUENCE:

      while( NextChild(elt, &index, name, &child) )
	test_child();
      break;

    default:

      assert(false);
  }
  if( index < KmlChildCount(elt) )
  {
    child = KmlChild(elt, index);
    return KmlErrorMake(ke, child->line_num, child->col_num,
      "unexpected child <%s> in <%s>", child->label, elt->label);
  }

  /* all fine */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KmlWrite(KML_ELT elt, KML_FILE kf)                                  */
/*                                                                           */
/*  Write elt to kf.                                                         */
/*                                                                           */
/*****************************************************************************/

void KmlWrite(KML_ELT elt, KML_FILE kf)
{
  int i;  char *str;  KML_ELT child;
  assert(elt != NULL);
  KmlBegin(kf, elt->label);
  MArrayForEach(elt->attribute_names, &str, &i)
    KmlAttribute(kf, str, MArrayGet(elt->attribute_values, i));
  if( elt->text != NULL )
  {
    KmlPrintf(kf, "%s", elt->text);
    KmlEndNoIndent(kf, elt->label);
  }
  else
  {
    MArrayForEach(elt->children, &child, &i)
      KmlWrite(child, kf);
    KmlEnd(kf, elt->label);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "freeing"                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KmlFree(KML_ELT elt, bool free_labels, bool free_attribute_names,   */
/*    bool free_attribute_values)                                            */
/*                                                                           */
/*  Free the memory occuped by elt and its descendants.                      */
/*                                                                           */
/*  Implementation note.  This function utilizes a detail of the             */
/*  specification of free():  if its parameter is NULL, it does nothing.     */
/*                                                                           */
/*****************************************************************************/

void KmlFree(KML_ELT elt, bool free_labels, bool free_attribute_names,
  bool free_attribute_values, bool free_text)
{
  KML_ELT child;  char *str;  int i;

  /* free the label */
  if( free_labels )
    MFree(elt->label);

  /* free the attribute names */
  if( free_attribute_names )
    MArrayForEach(elt->attribute_names, &str, &i)
      MFree(str);
  MArrayFree(elt->attribute_names);

  /* free the attribute values */
  if( free_attribute_values )
    MArrayForEach(elt->attribute_values, &str, &i)
      MFree(str);
  MArrayFree(elt->attribute_values);

  /* free the children */
  MArrayForEach(elt->children, &child, &i)
    KmlFree(child, free_labels, free_attribute_names, free_attribute_values,
      free_text);
  MArrayFree(elt->children);

  /* free the text and elt itself */
  if( free_text )
    MFree(elt->text);
  MFree(elt);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlExtractLabel(KML_ELT elt)                                       */
/*                                                                           */
/*  Return the label of elt, and set it to NULL at the same time.            */
/*                                                                           */
/*****************************************************************************/

char *KmlExtractLabel(KML_ELT elt)
{
  char *res;
  res = elt->label;
  elt->label = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlExtractAttributeName(KML_ELT elt, int index)                    */
/*                                                                           */
/*  Return the name of the index'th attribute of elt, and set it to NULL     */
/*  at the same time.                                                        */
/*                                                                           */
/*****************************************************************************/

char *KmlExtractAttributeName(KML_ELT elt, int index)
{
  char *res;
  if( index < 0 )
    index = KmlAttributeCount(elt) + index;
  assert(index < MArraySize(elt->attribute_names));
  res = MArrayGet(elt->attribute_names, index);
  MArrayPut(elt->attribute_names, index, NULL);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlExtractAttributeValue(KML_ELT elt, int index)                   */
/*                                                                           */
/*  Return the value of the index'th attribute of elt, and set it to NULL    */
/*  at the same time.                                                        */
/*                                                                           */
/*****************************************************************************/

char *KmlExtractAttributeValue(KML_ELT elt, int index)
{
  char *res;
  if( index < 0 )
    index = KmlAttributeCount(elt) + index;
  assert(index < MArraySize(elt->attribute_values));
  res = MArrayGet(elt->attribute_values, index);
  MArrayPut(elt->attribute_values, index, NULL);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KmlExtractText(KML_ELT elt)                                        */
/*                                                                           */
/*  Return the text of elt and set it to NULL at the same time.              */
/*                                                                           */
/*****************************************************************************/

char *KmlExtractText(KML_ELT elt)
{
  char *res;
  res = elt->text;
  elt->text = NULL;
  return res;
}
