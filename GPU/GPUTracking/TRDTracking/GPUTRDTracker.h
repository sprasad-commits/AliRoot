//**************************************************************************\
//* This file is property of and copyright by the ALICE Project            *\
//* ALICE Experiment at CERN, All rights reserved.                         *\
//*                                                                        *\
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *\
//*                  for The ALICE HLT Project.                            *\
//*                                                                        *\
//* Permission to use, copy, modify and distribute this software and its   *\
//* documentation strictly for non-commercial purposes is hereby granted   *\
//* without fee, provided that the above copyright notice appears in all   *\
//* copies and that both the copyright notice and this permission notice   *\
//* appear in the supporting documentation. The authors make no claims     *\
//* about the suitability of this software for any purpose. It is          *\
//* provided "as is" without express or implied warranty.                  *\
//**************************************************************************

/// \file GPUTRDTracker.h
/// \brief Online TRD tracker based on extrapolated TPC tracks

/// \author Ole Schmidt

#ifndef GPUTRDTRACKER_H
#define GPUTRDTRACKER_H

#include "GPUCommonDef.h"
#include "GPUProcessor.h"
#include "GPUTRDDef.h"
#include "GPUDef.h"
#include "GPUTRDTrackerDebug.h"
#include "GPUTRDTrack.h"

#ifndef __OPENCL__
#include <vector>
#endif

class AliExternalTrackParam;
class AliMCEvent;

namespace GPUCA_NAMESPACE
{
namespace gpu
{

#ifdef GPUCA_ALIROOT_LIB
#define TRD_GEOMETRY_CONST
#else
#define TRD_GEOMETRY_CONST const
#endif

class GPUTRDTrackletWord;
class GPUTRDGeometry;
class GPUTPCGMMerger;
class GPUChainTracking;

//-------------------------------------------------------------------------
class GPUTRDTracker : public GPUProcessor
{
 public:
#ifndef GPUCA_GPUCODE
  GPUTRDTracker();
  GPUTRDTracker(const GPUTRDTracker& tracker) CON_DELETE;
  GPUTRDTracker& operator=(const GPUTRDTracker& tracker) CON_DELETE;
  ~GPUTRDTracker();

  void SetMaxData();
  void RegisterMemoryAllocation();
  void InitializeProcessor();
  void* SetPointersBase(void* base);
  void* SetPointersTracklets(void* base);
  void* SetPointersTracks(void* base);

  bool Init(TRD_GEOMETRY_CONST GPUTRDGeometry* geo = nullptr);
  void CountMatches(const int trackID, std::vector<int>* matches) const;
  void DoTracking();
  void SetNCandidates(int n);
  void PrintSettings() const;
  bool IsInitialized() const { return mIsInitialized; }
  void SetTrackingChain(GPUChainTracking* c) { mChainTracking = c; }
#endif

  enum EGPUTRDTracker { kNLayers = 6,
                        kNStacks = 5,
                        kNSectors = 18,
                        kNChambers = 540 };

  // struct to hold the information on the space points
  struct GPUTRDSpacePointInternal {
    float mR;                 // x position (7mm above anode wires)
    float mX[2];              // y and z position (sector coordinates)
    My_Float mCov[3];         // sigma_y^2, sigma_yz, sigma_z^2
    float mDy;                // deflection over drift length
    int mId;                  // index
    int mLabel[3];            // MC labels
    unsigned short mVolumeId; // basically derived from TRD chamber number
  };

  struct Hypothesis {
    int mLayers;
    int mCandidateId;
    int mTrackletId;
    float mChi2;

    GPUd() float GetReducedChi2() { return mLayers > 0 ? mChi2 / mLayers : mChi2; }
    GPUd() Hypothesis() : mLayers(0), mCandidateId(-1), mTrackletId(-1), mChi2(9999.f) {}
    GPUd() Hypothesis(int layers, int candidateId, int trackletId, float chi2) : mLayers(layers), mCandidateId(candidateId), mTrackletId(trackletId), mChi2(chi2) {}
  };

  short MemoryPermanent() const { return mMemoryPermanent; }
  short MemoryTracklets() const { return mMemoryTracklets; }
  short MemoryTracks() const { return mMemoryTracks; }

  GPUhd() void SetGeometry(TRD_GEOMETRY_CONST GPUTRDGeometry* geo) { mGeo = geo; }
  void Reset(bool fast = false);
  GPUd() int LoadTracklet(const GPUTRDTrackletWord& tracklet, const int* labels = nullptr);
  template <class T>
  GPUd() int LoadTrack(const T& trk, const int label = -1)
  {
    if (mNTracks >= mNMaxTracks) {
#ifndef GPUCA_GPUCODE_DEVICE
      printf("Error: Track dropped (no memory available) -> must not happen\n");
#endif
      return (1);
    }
#ifdef GPUCA_ALIROOT_LIB
    new (&mTracks[mNTracks++]) GPUTRDTrack(trk);
#else
    mTracks[mNTracks++] = trk;
#endif
    if (label >= 0) {
      mTracks[mNTracks - 1].SetLabel(label);
    }
    return (0);
  }
  GPUd() void DoTrackingThread(int iTrk, const GPUTPCGMMerger* merger, int threadId = 0);
  GPUd() bool CalculateSpacePoints();
  GPUd() bool FollowProlongation(GPUTRDPropagator* prop, GPUTRDTrack* t, int threadId);
  GPUd() int GetDetectorNumber(const float zPos, const float alpha, const int layer) const;
  GPUd() bool AdjustSector(GPUTRDPropagator* prop, GPUTRDTrack* t, const int layer) const;
  GPUd() int GetSector(float alpha) const;
  GPUd() float GetAlphaOfSector(const int sec) const;
  GPUd() float GetRPhiRes(float snp) const { return (0.04f * 0.04f + 0.33f * 0.33f * (snp - 0.126f) * (snp - 0.126f)); } // parametrization obtained from track-tracklet residuals
  GPUd() void RecalcTrkltCov(const float tilt, const float snp, const float rowSize, My_Float (&cov)[3]);
  GPUd() void CheckTrackRefs(const int trackID, bool* findableMC) const;
  GPUd() void FindChambersInRoad(const GPUTRDTrack* t, const float roadY, const float roadZ, const int iLayer, int* det, const float zMax, const float alpha) const;
  GPUd() bool IsGeoFindable(const GPUTRDTrack* t, const int layer, const float alpha) const;
  GPUd() void SwapTracklets(const int left, const int right);
  GPUd() int PartitionTracklets(const int left, const int right);
  GPUd() void Quicksort(const int left, const int right, const int size);
  GPUd() void InsertHypothesis(Hypothesis hypo, int& nCurrHypothesis, int idxOffset);

  // settings
  GPUd() void SetMCEvent(AliMCEvent* mc) { mMCEvent = mc; }
  GPUd() void EnableDebugOutput() { mDebugOutput = true; }
  GPUd() void SetPtThreshold(float minPt) { mMinPt = minPt; }
  GPUd() void SetMaxEta(float maxEta) { mMaxEta = maxEta; }
  GPUd() void SetChi2Threshold(float chi2) { mMaxChi2 = chi2; }
  GPUd() void SetChi2Penalty(float chi2) { mChi2Penalty = chi2; }
  GPUd() void SetMaxMissingLayers(int ly) { mMaxMissingLy = ly; }

  GPUd() AliMCEvent* GetMCEvent() const { return mMCEvent; }
  GPUd() bool GetIsDebugOutputOn() const { return mDebugOutput; }
  GPUd() float GetPtThreshold() const { return mMinPt; }
  GPUd() float GetMaxEta() const { return mMaxEta; }
  GPUd() float GetChi2Threshold() const { return mMaxChi2; }
  GPUd() float GetChi2Penalty() const { return mChi2Penalty; }
  GPUd() int GetMaxMissingLayers() const { return mMaxMissingLy; }
  GPUd() int GetNCandidates() const { return mNCandidates; }

  // output
  GPUd() int NTracks() const { return mNTracks; }
  GPUd() GPUTRDTrack* Tracks() const { return mTracks; }
  GPUd() int NTracklets() const { return mNTracklets; }
  GPUd() GPUTRDSpacePointInternal* SpacePoints() const { return mSpacePoints; }
  GPUd() GPUTRDTrackletWord* Tracklets() const { return mTracklets; }
  GPUd() void DumpTracks();

 protected:
  float* mR;                               // rough radial position of each TRD layer
  bool mIsInitialized;                     // flag is set upon initialization
  short mMemoryPermanent;                  // size of permanent memory for the tracker
  short mMemoryTracklets;                  // size of memory for TRD tracklets
  short mMemoryTracks;                     // size of memory for tracks (used for i/o)
  int mNMaxTracks;                         // max number of tracks the tracker can handle (per event)
  int mNMaxSpacePoints;                    // max number of space points hold by the tracker (per event)
  GPUTRDTrack* mTracks;                    // array of trd-updated tracks
  int mNCandidates;                        // max. track hypothesis per layer
  int mNTracks;                            // number of TPC tracks to be matched
  int mNEvents;                            // number of processed events
  GPUTRDTrackletWord* mTracklets;          // array of all tracklets, later sorted by HCId
  int mMaxThreads;                         // maximum number of supported threads
  int mNTracklets;                         // total number of tracklets in event
  int* mNTrackletsInChamber;               // number of tracklets in each chamber
  int* mTrackletIndexArray;                // index of first tracklet for each chamber
  Hypothesis* mHypothesis;                 // array with multiple track hypothesis
  GPUTRDTrack* mCandidates;                // array of tracks for multiple hypothesis tracking
  GPUTRDSpacePointInternal* mSpacePoints;  // array with tracklet coordinates in global tracking frame
  int* mTrackletLabels;                    // array with MC tracklet labels
  TRD_GEOMETRY_CONST GPUTRDGeometry* mGeo; // TRD geometry
  bool mDebugOutput;                       // store debug output
  float mMinPt;                            // min pt of TPC tracks for tracking
  float mMaxEta;                           // TPC tracks with higher eta are ignored
  float mMaxChi2;                          // max chi2 for tracklets
  int mMaxMissingLy;                       // max number of missing layers per track
  float mChi2Penalty;                      // chi2 added to the track for no update
  float mZCorrCoefNRC;                     // tracklet z-position depends linearly on track dip angle
  AliMCEvent* mMCEvent;                    //! externaly supplied optional MC event
  const GPUTPCGMMerger* mMerger;           // supplying parameters for GPUTPCGMPropagator
  GPUTRDTrackerDebug* mDebug;              // debug output
  GPUChainTracking* mChainTracking;        // Tracking chain with access to input data / parameters
};
} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif // GPUTRDTRACKER_H
