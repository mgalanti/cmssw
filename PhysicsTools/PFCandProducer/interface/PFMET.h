#ifndef PhysicsTools_PFCandProducer_PFMET_
#define PhysicsTools_PFCandProducer_PFMET_

// system include files
#include <memory>
#include <string>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"

/**\class PFMET 
\brief Computes the MET from a collection of PFCandidates. HF missing!

\todo Add HF energy to the MET calculation (access HF towers)

\author Colin Bernet
\date   february 2008
*/




class PFMET : public edm::EDProducer {
 public:

  explicit PFMET(const edm::ParameterSet&);

  ~PFMET();
  
  virtual void produce(edm::Event&, const edm::EventSetup&);

  virtual void beginJob(const edm::EventSetup & c);

 private:
 

  
  /// Input PFCandidates
  edm::InputTag   inputTagPFCandidates_;
  
  /// HF calibration factor (in 31X applied by PFProducer)
  double hfCalibFactor_;

  /// verbose ?
  bool   verbose_;

};

#endif
