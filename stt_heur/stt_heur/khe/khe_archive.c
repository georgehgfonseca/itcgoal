
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
/*  FILE:         khe_archive.c                                              */
/*  DESCRIPTION:  An archive holding instances and solution groups           */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#include <stdarg.h>
#define MAX_ERROR_STRING 200


/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE - one archive of problems and solution groups                */
/*                                                                           */
/*****************************************************************************/

struct khe_archive_rec {
  void				*back;			/* back pointer      */
  char				*id;			/* optional Id       */
  KHE_ARCHIVE_METADATA		meta_data;		/* optional MetaData */
  ARRAY_KHE_INSTANCE		instance_array;		/* instance array    */
  TABLE_KHE_INSTANCE		instance_table;		/* instance table    */
  ARRAY_KHE_SOLN_GROUP		soln_group_array;	/* solution groups   */
  TABLE_KHE_SOLN_GROUP		soln_group_table;	/* solution groups   */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE KheArchiveMake(void)                                         */
/*                                                                           */
/*  Make an initially empty archive.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_ARCHIVE KheArchiveMake(char *id, KHE_ARCHIVE_METADATA md)
{
  KHE_ARCHIVE res;
  MMake(res);
  res->back = NULL;
  res->id = id;
  res->meta_data = md;
  MArrayInit(res->instance_array);
  MTableInit(res->instance_table);
  MArrayInit(res->soln_group_array);
  MTableInit(res->soln_group_table);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheArchiveId(KHE_ARCHIVE archive)                                  */
/*                                                                           */
/*  Return the Id of archive, possibly NULL.                                 */
/*                                                                           */
/*****************************************************************************/

char *KheArchiveId(KHE_ARCHIVE archive)
{
  return archive->id;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveSetBack(KHE_ARCHIVE archive, void *back)                  */
/*                                                                           */
/*  Set the back pointer of archive.                                         */
/*                                                                           */
/*****************************************************************************/

void KheArchiveSetBack(KHE_ARCHIVE archive, void *back)
{
  archive->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheArchiveBack(KHE_ARCHIVE archive)                                */
/*                                                                           */
/*  Retrieve the back pointer of archive.                                    */
/*                                                                           */
/*****************************************************************************/

void *KheArchiveBack(KHE_ARCHIVE archive)
{
  return archive->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ARCHIVE_METADATA KheArchiveMetaData(KHE_ARCHIVE archive)             */
/*                                                                           */
/*  Return the MetaData attribute of archive.                                */
/*                                                                           */
/*****************************************************************************/

KHE_ARCHIVE_METADATA KheArchiveMetaData(KHE_ARCHIVE archive)
{
  return archive->meta_data;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveSetMetaData(KHE_ARCHIVE archive, KHE_ARCHIVE_METADATA md) */
/*                                                                           */
/*  Set the metadata attribute of archive to md.                             */
/*                                                                           */
/*****************************************************************************/

void KheArchiveSetMetaData(KHE_ARCHIVE archive, KHE_ARCHIVE_METADATA md)
{
  archive->meta_data = md;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "instances"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveAddInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins)        */
/*                                                                           */
/*  Add ins to archive, assuming it is safe to do so (not duplicate Id).     */
/*                                                                           */
/*****************************************************************************/

void KheArchiveAddInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins)
{
  MArrayAddLast(archive->instance_array, ins);
  if( KheInstanceId(ins) != NULL )
    MTableInsert(archive->instance_table, KheInstanceId(ins), ins);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheArchiveInstanceCount(KHE_ARCHIVE archive)                         */
/*                                                                           */
/*  Return the number of instances in archive.                               */
/*                                                                           */
/*****************************************************************************/

int KheArchiveInstanceCount(KHE_ARCHIVE archive)
{
  return MArraySize(archive->instance_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheArchiveInstance(KHE_ARCHIVE archive, int i)              */
/*                                                                           */
/*  Return the i'th instance of archive.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheArchiveInstance(KHE_ARCHIVE archive, int i)
{
  return MArrayGet(archive->instance_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveRetrieveInstance(KHE_ARCHIVE archive, char *id,           */
/*    KHE_INSTANCE *ins)                                                     */
/*                                                                           */
/*  Retrieve an instance with the given id from archive.                     */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveRetrieveInstance(KHE_ARCHIVE archive, char *id,
  KHE_INSTANCE *ins)
{
  int pos;
  return MTableRetrieve(archive->instance_table, id, ins, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Sumbodule "solution groups"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheArchiveAddSolnGroup(KHE_ARCHIVE archive,                         */
/*    KHE_SOLN_GROUP soln_group)                                             */
/*                                                                           */
/*  Add soln_group to archive, assuming it is safe (not duplicate Id).       */
/*                                                                           */
/*****************************************************************************/

void KheArchiveAddSolnGroup(KHE_ARCHIVE archive, KHE_SOLN_GROUP soln_group)
{
  MArrayAddLast(archive->soln_group_array, soln_group);
  if( KheSolnGroupId(soln_group) != NULL )
    MTableInsert(archive->soln_group_table, KheSolnGroupId(soln_group),
      soln_group);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheArchiveSolnGroupCount(KHE_ARCHIVE archive)                        */
/*                                                                           */
/*  Return the number of solution groups in archive.                         */
/*                                                                           */
/*****************************************************************************/

int KheArchiveSolnGroupCount(KHE_ARCHIVE archive)
{
  return MArraySize(archive->soln_group_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP KheArchiveSolnGroup(KHE_ARCHIVE archive, int i)           */
/*                                                                           */
/*  Return the i'th solution group of archive.                               */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN_GROUP KheArchiveSolnGroup(KHE_ARCHIVE archive, int i)
{
  return MArrayGet(archive->soln_group_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveRetrieveSolnGroup(KHE_ARCHIVE archive, char *id,          */
/*    KHE_SOLN_GROUP *soln_group)                                            */
/*                                                                           */
/*  Retrieve a solution group with the given id from archive.                */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveRetrieveSolnGroup(KHE_ARCHIVE archive, char *id,
  KHE_SOLN_GROUP *soln_group)
{
  int pos;
  return MTableRetrieve(archive->soln_group_table, id, soln_group, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing archives"                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveEltToArchive(KML_ELT archive_elt, KHE_ARCHIVE *archive,   */
/*    bool infer_time_breaks, bool infer_resource_partitions, KML_ERROR *ke) */
/*                                                                           */
/*  Convert archive_elt into *archive.                                       */
/*                                                                           */
/*****************************************************************************/

static bool KheArchiveEltToArchive(KML_ELT archive_elt, KHE_ARCHIVE *archive,
  bool infer_resource_partitions, KML_ERROR *ke)
{
  char *id;  KML_ELT metadata_elt, instances_elt, instance_elt;
  KML_ELT soln_groups_elt, soln_group_elt;  KHE_ARCHIVE res;  int i;

  /* fail if archive_elt has problems */
  *archive = NULL;
  if( strcmp(KmlLabel(archive_elt), "HighSchoolTimetableArchive") != 0 )
    return KmlErrorMake(ke, KmlLineNum(archive_elt), KmlColNum(archive_elt),
      "file does not begin with <HighSchoolTimetableArchive>");
  if( !KmlCheck(archive_elt, "+Id : +MetaData +Instances +SolutionGroups", ke) )
    return false;

  /* create archive with optional id and optional metadata */
  id = KmlAttributeCount(archive_elt) == 0 ? NULL :
    KmlExtractAttributeValue(archive_elt, 0);
  res = KheArchiveMake(id, NULL);
  if( KmlContainsChild(archive_elt, "MetaData", &metadata_elt) &&
      !KheArchiveMetaDataMakeFromKml(metadata_elt, res, ke) )
    return false;

  /* build and add instances */
  if( KmlContainsChild(archive_elt, "Instances", &instances_elt) )
  {
    if( !KmlCheck(instances_elt, ": *Instance", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(instances_elt);  i++ )
    {
      instance_elt = KmlChild(instances_elt, i);
      if( !KheInstanceMakeFromKml(instance_elt, res,
	    infer_resource_partitions, ke) )
	return false;
    }
  }

  /* build and add solution groups */
  if( KmlContainsChild(archive_elt, "SolutionGroups", &soln_groups_elt) )
  {
    if( !KmlCheck(soln_groups_elt, ": *SolutionGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(soln_groups_elt);  i++ )
    {
      soln_group_elt = KmlChild(soln_groups_elt, i);
      if( !KheSolnGroupMakeFromKml(soln_group_elt, res, ke) )
	return false;
    }
  }

  KmlFree(archive_elt, true, true, true, true);
  *archive = res;
  *ke = NULL;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveRead(FILE *fp, KHE_ARCHIVE *archive,                      */
/*    bool infer_time_breaks, KML_ERROR *ke)                                 */
/*                                                                           */
/*  Read *archive from fp.                                                   */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveRead(FILE *fp, KHE_ARCHIVE *archive,
  bool infer_resource_partitions, KML_ERROR *ke)
{
  KML_ELT archive_elt;
  if( !KmlRead(fp, &archive_elt, ke) )
    return false;
  return KheArchiveEltToArchive(archive_elt, archive,
    infer_resource_partitions, ke);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveReadFromString(char *str, KHE_ARCHIVE *archive,           */
/*    bool infer_time_breaks, bool infer_resource_partitions, KML_ERROR *ke) */
/*                                                                           */
/*  Like KheArchiveRead except that the archive is read from str.            */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveReadFromString(char *str, KHE_ARCHIVE *archive,
  bool infer_resource_partitions, KML_ERROR *ke)
{
  KML_ELT archive_elt;
  if( !KmlReadString(str, &archive_elt, ke) )
    return false;
  return KheArchiveEltToArchive(archive_elt, archive,
    infer_resource_partitions, ke);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheArchiveWrite(KHE_ARCHIVE archive, bool with_reports, FILE *fp)   */
/*                                                                           */
/*  Write archive to fp, which must be open for writing.  Include reports    */
/*  in the written archive if with_reports is true.                          */
/*                                                                           */
/*****************************************************************************/

bool KheArchiveWrite(KHE_ARCHIVE archive, bool with_reports, FILE *fp)
{
  KML_FILE kf;  KHE_INSTANCE ins;  int i;  KHE_SOLN_GROUP soln_group;
  kf = KmlMakeFile(fp, 0, 2);

  /* header with optional Id, followed by optional metadata */
  KmlBegin(kf, "HighSchoolTimetableArchive");
  if( archive->id != NULL )
    KmlAttribute(kf, "Id", archive->id);
  if( archive->meta_data != NULL &&
      !KheArchiveMetaDataWrite(archive->meta_data, kf) )
    return false;

  /* instances */
  if( MArraySize(archive->instance_array) > 0 )
  {
    KmlBegin(kf, "Instances");
    MArrayForEach(archive->instance_array, &ins, &i)
      if( !KheInstanceWrite(ins, kf) )
	return false;
    KmlEnd(kf, "Instances");
  }

  /* soln groups */
  if( MArraySize(archive->soln_group_array) > 0 )
  {
    KmlBegin(kf, "SolutionGroups");
    MArrayForEach(archive->soln_group_array, &soln_group, &i)
      if( !KheSolnGroupWrite(soln_group, with_reports, kf) )
	return false;
    KmlEnd(kf, "SolutionGroups");
  }

  /* close header and exit */
  KmlEnd(kf, "HighSchoolTimetableArchive");
  return true;
}
