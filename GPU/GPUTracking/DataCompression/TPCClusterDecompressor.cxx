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

/// \file TPCClusterDecompressor.cxx
/// \author David Rohr

#include "TPCClusterDecompressor.h"
#include "ClusterNativeAccessExt.h"
#include "GPUParam.h"
#include "GPUTPCCompressionTrackModel.h"
#include <algorithm>
#include <cstring>

using namespace GPUCA_NAMESPACE::gpu;
using namespace o2::tpc;

int TPCClusterDecompressor::decompress(const CompressedClusters* clustersCompressed, o2::tpc::ClusterNativeAccessFullTPC& clustersNative, std::vector<o2::tpc::ClusterNative>& clusterBuffer, const GPUParam& param)
{
  std::vector<ClusterNative> clusters[NSLICES][GPUCA_ROW_COUNT];
  unsigned int offset = 0;
  for (unsigned int i = 0; i < clustersCompressed->nTracks; i++) {
    unsigned int slice = clustersCompressed->sliceA[i];
    unsigned int row = clustersCompressed->rowA[i];
    GPUTPCCompressionTrackModel track;
    for (unsigned int j = 0; j < clustersCompressed->nTrackClusters[i]; j++) {
      unsigned int pad = 0, time = 0;
      if (j) {
        unsigned char tmpSlice = clustersCompressed->sliceLegDiffA[offset - i - 1];
        bool changeLeg = (tmpSlice >= NSLICES);
        if (changeLeg) {
          tmpSlice -= NSLICES;
        }
        if (clustersCompressed->nComppressionModes & 2) {
          slice += tmpSlice;
          if (slice >= NSLICES) {
            slice -= NSLICES;
          }
          row += clustersCompressed->rowDiffA[offset - i - 1];
          if (row >= GPUCA_ROW_COUNT) {
            row -= GPUCA_ROW_COUNT;
          }
        } else {
          slice = tmpSlice;
          row = clustersCompressed->rowDiffA[offset - i - 1];
        }
        if (changeLeg && track.Mirror()) {
          break;
        }
        if (track.Propagate(param.tpcGeometry.Row2X(row), param.SliceParam[slice].Alpha)) {
          break;
        }
        unsigned int timeTmp = clustersCompressed->timeResA[offset - i - 1];
        if (timeTmp & 800000) {
          timeTmp |= 0xFF000000;
        }
        time = timeTmp + ClusterNative::packTime(param.tpcGeometry.LinearZ2Time(slice, track.Z()));
        pad = clustersCompressed->padResA[offset - i - 1] + ClusterNative::packPad(param.tpcGeometry.LinearY2Pad(slice, row, track.Y()));
      } else {
        time = clustersCompressed->timeA[i];
        pad = clustersCompressed->padA[i];
      }
      std::vector<ClusterNative>& clusterVector = clusters[slice][row];
      clusterVector.emplace_back(time, clustersCompressed->flagsA[offset], pad, clustersCompressed->sigmaTimeA[offset], clustersCompressed->sigmaPadA[offset], clustersCompressed->qMaxA[offset], clustersCompressed->qTotA[offset]);
      float y = param.tpcGeometry.LinearPad2Y(slice, row, clusterVector.back().getPad());
      float z = param.tpcGeometry.LinearTime2Z(slice, clusterVector.back().getTime());
      if (j == 0) {
        track.Init(param.tpcGeometry.Row2X(row), y, z, param.SliceParam[slice].Alpha, clustersCompressed->qPtA[i], param);
      }
      if (j + 1 < clustersCompressed->nTrackClusters[i] && track.Filter(y, z, row)) {
        break;
      }
      offset++;
    }
  }
  clusterBuffer.resize(clustersCompressed->nAttachedClusters + clustersCompressed->nUnattachedClusters);
  offset = 0;
  unsigned int offset2 = 0;
  for (unsigned int i = 0; i < NSLICES; i++) {
    for (unsigned int j = 0; j < GPUCA_ROW_COUNT; j++) {
      ClusterNative* ptr = &clusterBuffer[offset];
      clustersNative.clusters[i][j] = ptr;
      memcpy((void*)&clusterBuffer[offset], (const void*)clusters[i][j].data(), clusters[i][j].size() * sizeof(clusterBuffer[0]));
      offset += clusters[i][j].size();
      unsigned int time = 0;
      unsigned short pad = 0;
      for (unsigned int k = 0; k < clustersCompressed->nSliceRowClusters[i * GPUCA_ROW_COUNT + j]; k++) {
        if (clustersCompressed->nComppressionModes & 2) {
          unsigned int timeTmp = clustersCompressed->timeDiffU[offset2];
          if (timeTmp & 800000) {
            timeTmp |= 0xFF000000;
          }
          time += timeTmp;
          pad += clustersCompressed->padDiffU[offset2];
        } else {
          time = clustersCompressed->timeDiffU[offset2];
          pad = clustersCompressed->padDiffU[offset2];
        }
        clusterBuffer[offset] = ClusterNative(time, clustersCompressed->flagsU[offset2], pad, clustersCompressed->sigmaTimeU[offset2], clustersCompressed->sigmaPadU[offset2], clustersCompressed->qMaxU[offset2], clustersCompressed->qTotU[offset2]);
        offset++;
        offset2++;
      }
      clustersNative.nClusters[i][j] = clusters[i][j].size() + clustersCompressed->nSliceRowClusters[i * GPUCA_ROW_COUNT + j];
      std::sort(ptr, ptr + clustersNative.nClusters[i][j]);
    }
  }

  return 0;
}
