
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
/*  FILE:         khe_soln_group.c                                           */
/*  DESCRIPTION:  One group of solutions within an archive                   */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP - one group of solutions within an archive                */
/*                                                                           */
/*****************************************************************************/

struct khe_soln_group_rec {
  void				*back;			/* back pointer      */
  KHE_ARCHIVE			archive;		/* enclosing archive */
  char				*id;			/* Id                */
  KHE_SOLN_GROUP_METADATA	meta_data;		/* MetaData          */
  ARRAY_KHE_SOLN		solutions;		/* the solutions     */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnGroupMake(KHE_ARCHIVE archive, char *id,                     */
/*    KHE_SOLN_GROUP_METADATA md, KHE_SOLN_GROUP *soln_group)                */
/*                                                                           */
/*  Make an initially empty solution group with these attributes.            */
/*                                                                           */
/*****************************************************************************/

bool KheSolnGroupMake(KHE_ARCHIVE archive, char *id,
  KHE_SOLN_GROUP_METADATA md, KHE_SOLN_GROUP *soln_group)
{
  KHE_SOLN_GROUP res;

  /* make sure archive does not already contain this id  */
  if( id != NULL && archive != NULL &&
      KheArchiveRetrieveSolnGroup(archive, id, &res) )
  {
    *soln_group = NULL;
    return false;
  }

  /* OK, so go ahead and make the instance */
  MMake(res);
  res->back = NULL;
  res->archive = archive;
  res->id = id;
  res->meta_data = md;
  MArrayInit(res->solutions);
  if( archive != NULL )
    KheArchiveAddSolnGroup(archive, res);
  *soln_group = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupSetBack(KHE_SOLN_GROUP soln_group, void *back)          */
/*                                                                           */
/*  Set the back pointer of soln_group                                       */
/*                                                                           */
/*****************************************************************************/

void KheSolnGroupSetBack(KHE_SOLN_GROUP soln_group, void *back)
{
  soln_group->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheSolnGroupBack(KHE_SOLN_GROUP soln_group)                        */
/*                                                                           */
/*  Return the back pointer of soln_group.                                   */
/*                                                                           */
/*****************************************************************************/

void *KheSolnGroupBack(KHE_SOLN_GROUP soln_group)
{
  return soln_group->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE KheSolnGroupArchive(KHE_SOLN_GROUP soln_group)               */
/*                                                                           */
/*  Return the archive containing soln_group.                                */
/*                                                                           */
/*****************************************************************************/

KHE_ARCHIVE KheSolnGroupArchive(KHE_SOLN_GROUP soln_group)
{
  return soln_group->archive;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheSolnGroupId(KHE_SOLN_GROUP soln_group)                          */
/*                                                                           */
/*  Return the Id attribute of soln_group.                                   */
/*                                                                           */
/*****************************************************************************/

char *KheSolnGroupId(KHE_SOLN_GROUP soln_group)
{
  return soln_group->id;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP_METADATA KheSolnGroupMetaData(KHE_SOLN_GROUP soln_group)  */
/*                                                                           */
/*  Return the MetaData attribute of soln_group.                             */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN_GROUP_METADATA KheSolnGroupMetaData(KHE_SOLN_GROUP soln_group)
{
  return soln_group->meta_data;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupSetMetaData(KHE_SOLN_GROUP soln_group,                  */
/*    KHE_SOLN_GROUP_METADATA md)                                            */
/*                                                                           */
/*  Set the metadata attribute of soln_group to md.                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnGroupSetMetaData(KHE_SOLN_GROUP soln_group,
  KHE_SOLN_GROUP_METADATA md)
{
  soln_group->meta_data = md;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "solutions"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupAddSoln(KHE_SOLN_GROUP soln_group, KHE_SOLN soln)       */
/*                                                                           */
/*  Add soln to soln_group.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnGroupAddSoln(KHE_SOLN_GROUP soln_group, KHE_SOLN soln)
{
  MAssert(KheSolnGroupArchive(soln_group) ==
    KheInstanceArchive(KheSolnInstance(soln)), "KheSolnGroupAddSoln: "
    "soln_group and soln's instance lie in different archives");
  MArrayAddLast(soln_group->solutions, soln);
  KheSolnSetSolnGroup(soln, soln_group);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupDeleteSoln(KHE_SOLN_GROUP soln_group, KHE_SOLN soln)    */
/*                                                                           */
/*  Delete soln from soln_group.                                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnGroupDeleteSoln(KHE_SOLN_GROUP soln_group, KHE_SOLN soln)
{
  int pos;
  if( !MArrayContains(soln_group->solutions, soln, &pos) )
    MAssert(false, "KheSolnGroupDeleteSoln: soln not present");
  MArrayRemove(soln_group->solutions, pos);
  KheSolnSetSolnGroup(soln, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnGroupSolnCount(KHE_SOLN_GROUP soln_group)                     */
/*                                                                           */
/*  Return the number of solutions in soln_group.                            */
/*                                                                           */
/*****************************************************************************/

int KheSolnGroupSolnCount(KHE_SOLN_GROUP soln_group)
{
  return MArraySize(soln_group->solutions);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheSolnGroupSoln(KHE_SOLN_GROUP soln_group, int i)              */
/*                                                                           */
/*  Return the i'th solution of soln_group.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheSolnGroupSoln(KHE_SOLN_GROUP soln_group, int i)
{
  return MArrayGet(soln_group->solutions, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnGroupMakeFromKml(KML_ELT soln_group_elt,                     */
/*    KHE_ARCHIVE archive, KML_ERROR *ke)                                    */
/*                                                                           */
/*  Make a solution group based on soln_group_elt and add it to archive.     */
/*                                                                           */
/*****************************************************************************/

bool KheSolnGroupMakeFromKml(KML_ELT soln_group_elt, KHE_ARCHIVE archive,
  KML_ERROR *ke)
{
  KHE_SOLN_GROUP res;  int i;  KML_ELT soln_elt;  char *id;

  /* check soln_group_elt and make res */
  if( !KmlCheck(soln_group_elt, "Id : MetaData *Solution", ke) )
    return false;
  id = KmlExtractAttributeValue(soln_group_elt, 0);
  if( !KheSolnGroupMake(archive, id, NULL, &res) )
    return KmlErrorMake(ke, KmlLineNum(soln_group_elt),
      KmlColNum(soln_group_elt),"<SolutionGroup> Id \"%s\" used previously",id);

  /* add metadata and solutions */
  if( !KheSolnGroupMetaDataMakeFromKml(KmlChild(soln_group_elt, 0), res, ke) )
    return false;
  for( i = 1;  i < KmlChildCount(soln_group_elt);  i++ )
  {
    soln_elt = KmlChild(soln_group_elt, i);
    if( !KheSolnMakeFromKml(soln_elt, res, ke) )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnGroupWrite(KHE_SOLN_GROUP soln_group, bool with_reports,     */
/*    KML_FILE kf)                                                           */
/*                                                                           */
/*  Write soln_group to kf, with reports if with_reports is true.            */
/*                                                                           */
/*****************************************************************************/

bool KheSolnGroupWrite(KHE_SOLN_GROUP soln_group, bool with_reports,
  KML_FILE kf)
{
  KHE_SOLN soln;  int i;
  if( soln_group->id == NULL || soln_group->meta_data == NULL )
    return false;
  KmlBegin(kf, "SolutionGroup");
  KmlAttribute(kf, "Id", soln_group->id);
  if( !KheSolnGroupMetaDataWrite(soln_group->meta_data, kf) )
    return false;
  MArrayForEach(soln_group->solutions, &soln, &i)
    if( !KheSolnWrite(soln, with_reports, kf) )
      return false;
  KmlEnd(kf, "SolutionGroup");
  return true;
}
