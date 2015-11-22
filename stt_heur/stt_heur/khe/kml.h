
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
/*  FILE:         kml.h                                                      */
/*  MODULE:       XML reading and writing                                    */
/*                                                                           */
/*****************************************************************************/
#ifndef KML_HEADER_FILE
#define KML_HEADER_FILE
#include <stdio.h>
#include <stdbool.h>

typedef struct kml_file_rec *KML_FILE;		/* an XML direct write file */
typedef struct kml_elt_rec *KML_ELT;		/* an XML element           */
typedef struct kml_error_rec *KML_ERROR;	/* an XML error record      */

/* KML_ERROR */
extern bool KmlErrorMake(KML_ERROR *ke, int line_num, int col_num,
  char *fmt, ...);
extern int KmlErrorLineNum(KML_ERROR ke);
extern int KmlErrorColNum(KML_ERROR ke);
extern char *KmlErrorString(KML_ERROR ke);

/* KML_FILE writing (not involving KML_ELT objects) */
extern KML_FILE KmlMakeFile(FILE *fp, int initial_indent, int indent_step);
extern void KmlBegin(KML_FILE kf, char *label);
extern void KmlAttribute(KML_FILE kf, char *name, char *value);
extern void KmlPrintf(KML_FILE kf, char *fmt, ...);
extern void KmlEnd(KML_FILE kf, char *label);

/* complete elements which are simple combinations of the above */
extern void KmlEltAttribute(KML_FILE kf, char *label, char *name, char *value);
extern void KmlEltPrintf(KML_FILE kf, char *label, char *fmt, ...);
extern void KmlEltAttributeEltPrintf(KML_FILE kf, char *label, char *name,
  char *value, char *label2, char *fmt, ...);

/* KML_ELT construction */
extern KML_ELT KmlMakeElt(int line_num, int col_num, char *label);
extern void KmlAddAttribute(KML_ELT elt, char *name, char *value);
extern void KmlAddChild(KML_ELT elt, KML_ELT child);
extern void KmlDeleteChild(KML_ELT elt, KML_ELT child);
extern void KmlAddText(KML_ELT elt, char *text);

/* KML_ELT query */
extern int KmlLineNum(KML_ELT elt);
extern int KmlColNum(KML_ELT elt);
extern char *KmlLabel(KML_ELT elt);
extern KML_ELT KmlParent(KML_ELT elt);
extern int KmlAttributeCount(KML_ELT elt);
extern char *KmlAttributeName(KML_ELT elt, int index);
extern char *KmlAttributeValue(KML_ELT elt, int index);
extern int KmlChildCount(KML_ELT elt);
extern KML_ELT KmlChild(KML_ELT elt, int index);
extern bool KmlContainsChild(KML_ELT elt, char *label, KML_ELT *child_elt);
extern char *KmlText(KML_ELT elt);

/* KML_ELT reading and writing */
extern void KmlWrite(KML_ELT elt, KML_FILE kf);
extern bool KmlRead(FILE *fp, KML_ELT *res, KML_ERROR *ke);
extern bool KmlReadString(char *str, KML_ELT *res, KML_ERROR *ke);

/* verification */
extern bool KmlCheck(KML_ELT elt, char *fmt, KML_ERROR *ke);

/* freeing */
extern void KmlFree(KML_ELT elt, bool free_labels, bool free_attribute_names,
  bool free_attribute_values, bool free_text);
extern char *KmlExtractLabel(KML_ELT elt);
extern char *KmlExtractAttributeName(KML_ELT elt, int index);
extern char *KmlExtractAttributeValue(KML_ELT elt, int index);
extern char *KmlExtractText(KML_ELT elt);

#endif
