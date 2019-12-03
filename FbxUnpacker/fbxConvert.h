#ifndef FBXCONVERT_H
#define FBXCONVERT_H

#include <string>
#include <vector>
#include <map>

#include "weights.h"

#include "fbxsdk.h"
#include "fbxsdk/fileio/fbxiosettings.h"

void convert           (
                              std::string fbxPathFile
                       );

void navigateSkeleton  (
                              std::vector<std::string>                         & names,
                              std::vector<double>                              & jointsPositions,
                              std::vector<int>                                 & fathers,
                              FbxNode                                          * node,
                              int                                                father
                       );

void saveTrimeshObj    (
                        const std::string                                      & filename,
                        const std::vector<double>                              & v,
                        const std::vector<int>                                 & f
                       );

void saveSkeleton      (
                        const std::string                                      & filename,
                        const std::vector<std::string>                         & names,
                        const std::vector<double>                              & jointsPositions,
                        const std::vector<int>                                 & fathers
                       );

void saveWeights       (
                        const std::string                                      & filename,
                        const Weights                                          & weights
                       );

void saveSkelAnimation (
                        const std::string                                      & filename,
                        const std::vector<double>                              & t,
                              std::vector<std::vector<std::vector<double>>>    & skelKeyframes //This is sooo ugly D: (but i don't care lol)
                       );

void getNodeKeyframe   (
                              FbxNode                                          * node,
                        const FbxTime                                          & t,
                              std::vector<std::vector<double>>                 & deformedKeyframes,
                        const std::map<std::string, unsigned long>             & nodeIdByName
                       );

std::vector<double>
  fromFbxMatrixToVector(
                         const FbxAMatrix                                      & matrix
                       );

#endif // FBXCONVERT_H
