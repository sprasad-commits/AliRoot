//_________________________________________________________________________
//Created by Christine Nattrass, Rebecca Scott, Irakli Martashvili
//University of Tennessee at Knoxville
//
// This class is designed for the analysis of the hadronic component of 
// transverse energy.  It is used by AliAnalysisTaskHadEt.
//_________________________________________________________________________
#ifndef ALIANALYSISETCOMMON_H
#define ALIANALYSISETCOMMON_H

#include "TString.h"
#include "TMath.h"

class TH2F;
class TH1F;
class TF1;
class AliVEvent;
class TList;
class AliESDtrackCuts;
class Rtypes;
class TParticle;
class TDatabasePDG;
class AliAnalysisEtCuts;

#ifndef ALIANALYSISLEVYPT_H
#define ALIANALYSISLEVYPT_H
class AliAnalysisLevyPt{
 public:
  virtual ~AliAnalysisLevyPt(){;};
  Double_t Evaluate(const Double_t *pt, const Double_t *par) const
  {
    Double_t lMass  = 0.497;
    Double_t ldNdy  = par[0];
    Double_t l2pi   = 2*TMath::Pi();
    Double_t lTemp = par[1];
    Double_t lPower = par[2];

    Double_t lBigCoef = ((lPower-1)*(lPower-2)) / (l2pi*lPower*lTemp*(lPower*lTemp+lMass*(lPower-2)));
    Double_t lInPower = 1 + (TMath::Sqrt(pt[0]*pt[0]+lMass*lMass)-lMass) / (lPower*lTemp);

    return ldNdy * pt[0] * lBigCoef * TMath::Power(lInPower,(-1)*lPower);
  };
  ClassDef(AliAnalysisLevyPt, 1);
};

#endif // ALIANALYSISLEVYPT_H

class AliAnalysisEtCommon
{
public:
   
  AliAnalysisEtCommon();
    virtual ~AliAnalysisEtCommon();

    /** Analyse the event! */
    virtual Int_t AnalyseEvent(AliVEvent *event);


    /** Initialise the analysis, must be overloaded. */
    virtual void Init();


    /** Reset event specific values (Et etc.) */
    virtual void ResetEventValues();

    /** Cuts info */
    AliAnalysisEtCuts * GetCuts() const { return fCuts; } 
    virtual void SetCuts(const AliAnalysisEtCuts *cuts) 
    { fCuts = (AliAnalysisEtCuts *) cuts; } 


    void SetTPCITSTrackCuts(const AliESDtrackCuts *cuts){ fEsdtrackCutsITSTPC = (AliESDtrackCuts *) cuts;}
    void SetTPCOnlyTrackCuts(const AliESDtrackCuts *cuts){ fEsdtrackCutsTPC = (AliESDtrackCuts *) cuts;}
    void SetITSTrackCuts(const AliESDtrackCuts *cuts){ fEsdtrackCutsITS = (AliESDtrackCuts *) cuts;}
    void SetDataSet(Int_t val){fDataSet = val;}
    Int_t DataSet()const {return fDataSet;}

protected:   
    
    TString fHistogramNameSuffix; /** The suffix for the histogram names */

    AliAnalysisEtCuts *fCuts; // keeper of basic cuts

    Int_t fDataSet;//Integer corresponding to data set.  Used as a switch to set appropriate track cuts.  By default set to 2010 p+p
    //2009 = 900 GeV p+p data from 2009
    //2010 = 7 TeV p+p data from 2010
    //20100 = 2.76 TeV Pb+Pb data from 2010

    /** PDG Database */
    //TDatabasePDG *fPdgDB;//data base used for looking up pdg codes
    //these codes are stored as variables because otherwise there were issues using this with the plugin

    static Int_t fgPiPlusCode;//pdg pi plus code
    static Int_t fgPiMinusCode;//pdg pi minus code
    static Int_t fgKPlusCode;// pdg k plus code
    static Int_t fgKMinusCode;//pdg k minus code
    static Int_t fgProtonCode;//pdg proton code
    static Int_t fgAntiProtonCode;//pdg antiproton code
    static Int_t fgLambdaCode;// pdg lambda code
    static Int_t fgAntiLambdaCode;//pdg antilambda code
    static Int_t fgK0SCode;//pdg k0 short code
    static Int_t fgOmegaCode;//pdg omega code
    static Int_t fgAntiOmegaCode;//pdg anti-omega code
    static Int_t fgXi0Code;//pdg xi-0 code
    static Int_t fgAntiXi0Code;//pdg anti-xi0 code
    static Int_t fgXiCode;//pdg xi code
    static Int_t fgAntiXiCode;//pdg anti-xi code
    static Int_t fgSigmaCode;//pdg sigma code
    static Int_t fgAntiSigmaCode;//pdg anti-sigma code
    static Int_t fgK0LCode;//pdg k0 long code
    static Int_t fgNeutronCode;//pdg neutron code
    static Int_t fgAntiNeutronCode;//pdg anti-neutron code
    static Int_t fgEPlusCode;//pdg positron code
    static Int_t fgEMinusCode;//pdg electron code
    static Int_t fgMuPlusCode;//pdg positron code
    static Int_t fgMuMinusCode;//pdg electron code
    static Int_t fgGammaCode;//pdg gamma code
    static Int_t fgPi0Code;//pdg neutral pion code
    static Int_t fgEtaCode;//pdg eta code
    static Int_t fgOmega0Code;//pdg eta code
    static Float_t fgPionMass;//pdg pion mass
    static Float_t fgKaonMass;//pdg kaon mass
    static Float_t fgProtonMass;//pdg proton mass
    static Float_t fgElectronMass;//pdg electron mass


    Float_t Et(TParticle *part, float mass = -1000);
    Float_t Et(Float_t p, Float_t theta, Int_t pid, Short_t charge) const;
    AliESDtrackCuts* fEsdtrackCutsITSTPC;//esd track cuts for ITS+TPC tracks
    AliESDtrackCuts* fEsdtrackCutsTPC;//esd track cuts for TPC tracks (which may also contain ITS hits)
    AliESDtrackCuts* fEsdtrackCutsITS;//esd track cuts for ITS stand alone tracks

    static Float_t fgPtTPCCutOff;//cut off for tracks in TPC
    static Float_t fgPtITSCutOff;//cut off for tracks in ITS
    
    //Set by default to the D6T scales
    Float_t K0Weight(Float_t pt);//Function which gives the factor to reweigh a K0 so it roughly matches the data
    Float_t LambdaWeight(Float_t pt);//Function which gives the factor to reweigh a Lambda so it roughly matches the data
    Float_t AntiLambdaWeight(Float_t pt);//Function which gives the factor to reweigh a Lambda so it roughly matches the data
    TF1 *fK0PythiaD6T;//function with Levy fit parameters for K0S in PYTHIA D6T
    TF1 *fLambdaPythiaD6T;//function with Levy fit parameters for Lambda in PYTHIA D6T
    TF1 *fAntiLambdaPythiaD6T;//function with Levy fit parameters for AntiLambda in PYTHIA D6T
    TF1 *fK0Data;//function with Levy fit parameters for K0S in data
    TF1 *fLambdaData;//function with Levy fit parameters for Lambda in data
    TF1 *fAntiLambdaData;//function with Levy fit parameters for AntiLambda in data

    TF1 *fLambdaEnhancement;
    TF1 *fProtonEnhancement;
    Float_t LambdaBaryonEnhancement(Float_t pt);//Function which gives the factor to reweigh a lambda or antilambda so it roughly matches baryon enhancement seen at RHIC
    Float_t ProtonBaryonEnhancement(Float_t pt);//Function which gives the factor to reweigh a lambda or antilambda so it roughly matches baryon enhancement seen at RHIC


 private:
    //Declare it private to avoid compilation warning
    AliAnalysisEtCommon & operator = (const AliAnalysisEtCommon & g) ;//cpy assignment
    AliAnalysisEtCommon(const AliAnalysisEtCommon & g) ; // cpy ctor

    ClassDef(AliAnalysisEtCommon, 1);
};

#endif // ALIANALYSISETCOMMON_H
