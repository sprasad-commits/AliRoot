/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id$ */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// class that contains an object from the data base and knows about its      //
// validity range (meta data)                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


#include "AliRunData.h"
#include "AliObjectMetaData.h"

ClassImp(AliRunData)


//_____________________________________________________________________________
AliRunData::AliRunData() :
  TObject(),
  fObject(NULL),
  fObjMetaData()
{
// default constructor

}

//_____________________________________________________________________________
AliRunData::AliRunData(TObject* object, const AliObjectMetaData& objMetaData) :
  TObject(),
  fObject(object),
  fObjMetaData(objMetaData)
{
// constructor

}

//_____________________________________________________________________________
AliRunData::~AliRunData()
{
// destructor

  delete fObject;
}


//_____________________________________________________________________________
AliRunData::AliRunData(const AliRunData& entry) :
  TObject(entry),
  fObjMetaData(entry.fObjMetaData)
{
// copy constructor

}

//_____________________________________________________________________________
AliRunData& AliRunData::operator = (const AliRunData& entry)
{
// assignment operator

  delete fObject;
  fObject = entry.fObject->Clone();
  fObjMetaData = entry.fObjMetaData;
  return *this;
}



//_____________________________________________________________________________
const char* AliRunData::GetName() const
{
// get the name

  return fObjMetaData.GetName();
}


//_____________________________________________________________________________
Int_t AliRunData::Compare(const TObject* object) const
{
// check whether this is preferred to object

  if (!object || !object->InheritsFrom(AliRunData::Class())) return 1;
  return fObjMetaData.Compare(&((AliRunData*)object)->GetObjectMetaData());
}

