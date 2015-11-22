
/*****************************************************************************/
/*                                                                           */
/*  THE KHE HIGH SCHOOL TIMETABLING ENGINE                                   */
/*  COPYRIGHT (C) 2010 Jeffrey H. Kingston                                   */
/*                                                                           */
/*  Jeffrey H. Kingston (jeff@it.usyd.edu.au)                                */
/*  School of Information Technologies                                       */
/*  The University of Sydney 2006                                            */
/*  AUSTRALIA                                                                */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either Version 3, or (at your option)      */
/*  any later version.                                                       */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston MA 02111-1307 USA   */
/*                                                                           */
/*  FILE:         khe_instance_metadata.c                                    */
/*  DESCRIPTION:  Instance metadata                                          */
/*                                                                           */
/*****************************************************************************/
#include <stdarg.h>
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE_METADATA - instance metadata                                */
/*                                                                           */
/*****************************************************************************/

struct khe_instance_metadata_rec {
  char			*name;			/* instance name             */
  char			*contributor;		/* contributor               */
  char			*date;			/* date                      */
  char			*country;		/* country                   */
  char			*description;		/* description               */
  char			*remarks;		/* remarks (optional)        */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE_METADATA KheInstanceMetaDataMake(char *name, char *contr,   */
/*    char *date, char *country, char *description, char *remarks)           */
/*                                                                           */
/*  Make instance metadata with these attributes.                            */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE_METADATA KheInstanceMetaDataMake(char *name, char *contributor,
  char *date, char *country, char *description, char *remarks)
{
  KHE_INSTANCE_METADATA res;
  MMake(res);
  res->name = name;
  res->contributor = contributor;
  res->date = date;
  res->country = country;
  res->description = description;
  res->remarks = remarks;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInstanceMetaDataName(KHE_INSTANCE_METADATA md)                  */
/*                                                                           */
/*  Return the Name attribute of md.                                         */
/*                                                                           */
/*****************************************************************************/

char *KheInstanceMetaDataName(KHE_INSTANCE_METADATA md)
{
  return md->name;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInstanceMetaDataContributor(KHE_INSTANCE_METADATA md)           */
/*                                                                           */
/*  Return the Contributor attribute of md.                                  */
/*                                                                           */
/*****************************************************************************/

char *KheInstanceMetaDataContributor(KHE_INSTANCE_METADATA md)
{
  return md->contributor;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInstanceMetaDataDate(KHE_INSTANCE_METADATA md)                  */
/*                                                                           */
/*  Return the Date attribute of md.                                         */
/*                                                                           */
/*****************************************************************************/

char *KheInstanceMetaDataDate(KHE_INSTANCE_METADATA md)
{
  return md->date;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInstanceMetaDataCountry(KHE_INSTANCE_METADATA md)               */
/*                                                                           */
/*  Return the Country attribute of md.                                      */
/*                                                                           */
/*****************************************************************************/

char *KheInstanceMetaDataCountry(KHE_INSTANCE_METADATA md)
{
  return md->country;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInstanceMetaDataDescription(KHE_INSTANCE_METADATA md)           */
/*                                                                           */
/*  Return the Description attribute of md.                                  */
/*                                                                           */
/*****************************************************************************/

char *KheInstanceMetaDataDescription(KHE_INSTANCE_METADATA md)
{
  return md->description;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheInstanceMetaDataRemarks(KHE_INSTANCE_METADATA md)               */
/*                                                                           */
/*  Return the optional Remarks attribute of md.                             */
/*                                                                           */
/*****************************************************************************/

char *KheInstanceMetaDataRemarks(KHE_INSTANCE_METADATA md)
{
  return md->remarks;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceMetaDataMakeFromKml(KML_ELT md_elt, KHE_INSTANCE ins,    */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Make metadata from md_elt and add it to ins.                             */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceMetaDataMakeFromKml(KML_ELT md_elt, KHE_INSTANCE ins,
  KML_ERROR *ke)
{
  KHE_INSTANCE_METADATA res;
  if( !KmlCheck(md_elt,
      ": $Name $Contributor $Date $Country $Description +$Remarks", ke) )
    return false;
  res = KheInstanceMetaDataMake(KmlExtractText(KmlChild(md_elt, 0)),
    KmlExtractText(KmlChild(md_elt, 1)), KmlExtractText(KmlChild(md_elt, 2)),
    KmlExtractText(KmlChild(md_elt, 3)), KmlExtractText(KmlChild(md_elt, 4)),
    KmlChildCount(md_elt) == 6 ? KmlExtractText(KmlChild(md_elt, 5)) : "");
  KheInstanceSetMetaData(ins, res);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInstanceMetaDataWrite(KHE_INSTANCE_METADATA md, KML_FILE kf)     */
/*                                                                           */
/*  Write instance metadata to xml.                                          */
/*                                                                           */
/*****************************************************************************/

bool KheInstanceMetaDataWrite(KHE_INSTANCE_METADATA md, KML_FILE kf)
{
  KmlBegin(kf, "MetaData");

  /* name */
  if( md->name == NULL )
    return false;
  KmlEltPrintf(kf, "Name", "%s", md->name);

  /* contributor */
  if( md->contributor == NULL )
    return false;
  KmlEltPrintf(kf, "Contributor", "%s", md->contributor);

  /* date */
  if( md->date == NULL )
    return false;
  KmlEltPrintf(kf, "Date", "%s", md->date);

  /* country */
  if( md->country == NULL )
    return false;
  KmlEltPrintf(kf, "Country", "%s", md->country);

  /* description */
  if( md->description == NULL )
    return false;
  KmlEltPrintf(kf, "Description", "%s", md->description);

  /* remarks (optional) */
  if( md->remarks != NULL && strlen(md->remarks) > 0 )
    KmlEltPrintf(kf, "Remarks", "%s", md->remarks);
  KmlEnd(kf, "MetaData");
  return true;
}
