
#include "DQM/SiStripMonitorClient/interface/SiStripActionExecutor.h"
#include "DQMServices/Core/interface/DQMStore.h"

#include "CondFormats/SiStripObjects/interface/SiStripFedCabling.h"
#include "CalibFormats/SiStripObjects/interface/SiStripDetCabling.h"

#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/SiStripDetId/interface/TECDetId.h"
#include "DataFormats/SiStripDetId/interface/TIBDetId.h"
#include "DataFormats/SiStripDetId/interface/TOBDetId.h"
#include "DataFormats/SiStripDetId/interface/TIDDetId.h"

#include "DQM/SiStripCommon/interface/SiStripFolderOrganizer.h"
#include "DQM/SiStripMonitorClient/interface/SiStripUtility.h"
#include "DQM/SiStripMonitorClient/interface/SiStripSummaryCreator.h"
#include "DQM/SiStripMonitorClient/interface/SiStripTrackerMapCreator.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"


#include <iostream>
#include <iomanip>
using namespace std;
//
// -- Constructor
// 
SiStripActionExecutor::SiStripActionExecutor() {
  edm::LogInfo("SiStripActionExecutor") << 
    " Creating SiStripActionExecutor " << "\n" ;
  summaryCreator_= 0;
  tkMapCreator_ = 0; 

  bookedGlobalStatus_ = false;
}
//
// --  Destructor
// 
SiStripActionExecutor::~SiStripActionExecutor() {
  edm::LogInfo("SiStripActionExecutor") << 
    " Deleting SiStripActionExecutor " << "\n" ;
  if (summaryCreator_) delete   summaryCreator_;
  if (tkMapCreator_) delete   tkMapCreator_;
}
//
// -- Read Configurationn File
//
bool SiStripActionExecutor::readConfiguration() {
  
  if (!summaryCreator_) {
    summaryCreator_ = new SiStripSummaryCreator();
  }
  if (summaryCreator_->readConfiguration()) return true;
  else return false;
}
//
// -- Read Configurationn File
//
bool SiStripActionExecutor::readTkMapConfiguration() {
  
  if (tkMapCreator_) delete tkMapCreator_;
  tkMapCreator_ = new SiStripTrackerMapCreator();
  if (tkMapCreator_->readConfiguration()) return true;
  else return false;
}
//
// -- Read Configurationn File
//
bool SiStripActionExecutor::readConfiguration(int& sum_freq) {
  bool result = false;
  if (readConfiguration()) {
    sum_freq = summaryCreator_->getFrequency();
    if (sum_freq != -1) result = true;
  }
  return result;
}
//
// -- Create and Fill Summary Monitor Elements
//
void SiStripActionExecutor::createSummary(DQMStore* dqm_store) {
  if (summaryCreator_) {
    dqm_store->cd();
    string dname = "SiStrip/MechanicalView";
    if (dqm_store->dirExists(dname)) {
      dqm_store->cd(dname);
      summaryCreator_->createSummary(dqm_store);
    }
  }
}
//
// -- Create and Fill Summary Monitor Elements
//
void SiStripActionExecutor::createSummaryOffline(DQMStore* dqm_store) {
  if (summaryCreator_) {
    dqm_store->cd();
    string dname = "MechanicalView";
    if (goToDir(dqm_store, dname)) {
      summaryCreator_->createSummary(dqm_store);
    }
  }
}
//
// -- create tracker map
//
void SiStripActionExecutor::createTkMap(const edm::ParameterSet & tkmapPset, 
           const edm::ESHandle<SiStripFedCabling>& fedcabling, DQMStore* dqm_store) {
  if (tkMapCreator_) tkMapCreator_->create(tkmapPset, fedcabling, dqm_store);
}
//
// -- create reportSummary MEs
//
void SiStripActionExecutor::bookGlobalStatus(DQMStore* dqm_store) {

  if (!bookedGlobalStatus_) {
    dqm_store->cd();
    
    dqm_store->setCurrentFolder("SiStrip/EventInfo");    
    SummaryReport = dqm_store->bookFloat("reportSummary");
    
    SummaryReportMap = dqm_store->book2D("reportSummaryMap","SiStrip Report Summary Map",6,0.5,6.5,9,0.5,9.5);
    SummaryReportMap->setAxisTitle("Sub Detector Type", 1);
    SummaryReportMap->setAxisTitle("Layer/Disc Number", 2);
    SummaryReportMap->setBinLabel(1, "TIB");
    SummaryReportMap->setBinLabel(2, "TOB");
    SummaryReportMap->setBinLabel(3, "TIDF");
    SummaryReportMap->setBinLabel(4, "TIDB");
    SummaryReportMap->setBinLabel(5, "TECF");
    SummaryReportMap->setBinLabel(6, "TECB");
    
    dqm_store->setCurrentFolder("SiStrip/EventInfo/reportSummaryContents");      
    
    SummaryTIB  = dqm_store->bookFloat("SiStrip_TIB");
    SummaryTOB  = dqm_store->bookFloat("SiStrip_TOB");
    SummaryTIDF = dqm_store->bookFloat("SiStrip_TIDF");
    SummaryTIDB = dqm_store->bookFloat("SiStrip_TIDB");
    SummaryTECF = dqm_store->bookFloat("SiStrip_TECF");
    SummaryTECB = dqm_store->bookFloat("SiStrip_TECB");
    
    bookedGlobalStatus_ = true;
    fillDummyGlobalStatus();
  }
}
//
// -- Fill Dummy Global Status
//
void SiStripActionExecutor::fillDummyGlobalStatus(){
  
  resetGlobalStatus();

  SummaryReport->Fill(-1.0);

  SummaryTIB->Fill(-1.0);
  SummaryTOB->Fill(-1.0);
  SummaryTIDF->Fill(-1.0);
  SummaryTIDB->Fill(-1.0);
  SummaryTECF->Fill(-1.0);
  SummaryTECB->Fill(-1.0);
  
  for (unsigned int xbin = 1; xbin < 7; xbin++) {
    for (unsigned int ybin = 1; ybin < 10; ybin++) {
      SummaryReportMap->Fill(xbin, ybin, -1.0);
    }
  }

}
// 
// -- Fill Global Status
//
void SiStripActionExecutor::fillGlobalStatus(const edm::ESHandle<SiStripDetCabling>& detcabling, DQMStore* dqm_store) {
  if (!bookedGlobalStatus_) bookGlobalStatus(dqm_store);
  float gStatus = -1.0;
  // get connected detectors
  std::vector<uint32_t> SelectedDetIds;
  detcabling->addActiveDetectorsRawIds(SelectedDetIds);
  int nDetErr = 0;
  int nDetTot = 0;
  int nMeErr = 0;
  int nMeTot = 0;
  int nDetTIBErr, nDetTOBErr, nDetTIDFErr, nDetTIDBErr, nDetTECFErr, nDetTECBErr;
  int nDetTIBTot, nDetTOBTot, nDetTIDFTot, nDetTIDBTot, nDetTECFTot, nDetTECBTot;
  float statusTIB, statusTOB,  statusTIDF,  statusTIDB,  statusTECF,  statusTECB;
  int nMeTIBTot, nMeTOBTot, nMeTIDFTot, nMeTIDBTot, nMeTECFTot, nMeTECBTot;
  int nMeTIBErr, nMeTOBErr, nMeTIDFErr, nMeTIDBErr, nMeTECFErr, nMeTECBErr;

  statusTIB = statusTOB = statusTIDF = statusTIDB = statusTECF = statusTECB = -1;
  nDetTIBErr = nDetTOBErr = nDetTIDFErr = nDetTIDBErr = nDetTECFErr = nDetTECBErr = 0;
  nDetTIBTot = nDetTOBTot = nDetTIDFTot = nDetTIDBTot = nDetTECFTot = nDetTECBTot = 0;
  nMeTIBTot = nMeTOBTot = nMeTIDFTot = nMeTIDBTot = nMeTECFTot = nMeTECBTot = 0;
  nMeTIBErr = nMeTOBErr = nMeTIDFErr = nMeTIDBErr = nMeTECFErr = nMeTECBErr = 0;

  SiStripFolderOrganizer folder_organizer;
  for (std::vector<uint32_t>::const_iterator idetid=SelectedDetIds.begin(), iEnd=SelectedDetIds.end();idetid!=iEnd;++idetid){    
    uint32_t detId = *idetid;
    if (detId == 0 || detId == 0xFFFFFFFF){
      edm::LogError("SiStripAnalyser") 
                          << "SiStripAnalyser::fillGlobalStatus : " 
                          << "Wrong DetId !!!!!! " <<  detId << " Neglecting !!!!!! ";
      continue;
    }
    StripSubdetector subdet(*idetid);
    string dir_path;
    folder_organizer.getFolderName(detId, dir_path);     
    vector<MonitorElement*> detector_mes = dqm_store->getContents(dir_path);
   
    if (detector_mes.size() == 0 ) continue;
    nDetTot++;
    int error_me = 0;
    for (vector<MonitorElement *>::const_iterator it = detector_mes.begin();
	 it!= detector_mes.end(); it++) {
      MonitorElement * me = (*it);     
      if (!me) continue;
      if (me->getQReports().size() == 0) continue;
      int istat =  SiStripUtility::getMEStatus((*it)); 
      if (istat == dqm::qstatus::ERROR)  error_me++;
    }
    if (error_me > 0) {
     nDetErr++;
    }
    switch (subdet.subdetId()) 
      {
      case StripSubdetector::TIB:
	{
	  nDetTIBTot++;
	  if (error_me > 0) nDetTIBErr++;
	  break;       
	}
      case StripSubdetector::TID:
	{
	  TIDDetId tidId(detId);
	  if (tidId.side() == 2) {
	    nDetTIDFTot++;
	    if (error_me > 0) nDetTIDFErr++;
	  }  else if (tidId.side() == 1) {
	    nDetTIDBTot++;
	    if (error_me > 0) nDetTIDBErr++;
	  }
	  break;       
	}
      case StripSubdetector::TOB:
	{
	  nDetTOBTot++;
	  if (error_me > 0) nDetTOBErr++;
	  break;       
	}
      case StripSubdetector::TEC:
	{
	  TECDetId tecId(detId);
	  if (tecId.side() == 2) {
	    nDetTECFTot++;
	    if (error_me > 0) nDetTECFErr++;
	  }  else if (tecId.side() == 1) {
	    nDetTECBTot++;
	    if (error_me > 0) nDetTECBErr++;
	  }
	  break;       
	}
      }
  }
  fillDummyGlobalStatus();
  if (nDetTot > 0) {
    gStatus = (1 - nDetErr*1.0/nDetTot);
 
    string dname;
    // If TIB detectors are present in the set up   
    if (nDetTIBTot  > 0) {
      statusTIB  = (1 - nDetTIBErr*1.0/nDetTIBTot);
      dname = "SiStrip/MechanicalView/TIB";
      fillSubDetStatus(dqm_store, dname, 1);  
    }
    // If TOB detectors are present in the set up 
    if (nDetTOBTot  > 0) {
       statusTOB  = (1 - nDetTOBErr*1.0/nDetTOBTot);
       dname = "SiStrip/MechanicalView/TOB";
       fillSubDetStatus(dqm_store, dname, 2);  
    }
    // If TIDF detectors are present in the set up    
    if (nDetTIDFTot > 0) {
      statusTIDF = (1 - nDetTIDFErr*1.0/nDetTIDFTot);
      dname = "SiStrip/MechanicalView/TID/side_2";
      fillSubDetStatus(dqm_store, dname, 3);  
    }
    // If TIDB detectors are present in the set up 
    if (nDetTIDBTot > 0) {
      statusTIDB = (1 - nDetTIDBErr*1.0/nDetTIDBTot);
      dname = "SiStrip/MechanicalView/TID/side_1";
      fillSubDetStatus(dqm_store, dname, 4);  
    }
    // If TECF detectors are present in the set up 
    if (nDetTECFTot > 0) {
      statusTECF = (1 - nDetTECFErr*1.0/nDetTECFTot);
      dname = "SiStrip/MechanicalView/TEC/side_2";
      fillSubDetStatus(dqm_store, dname, 5);  
    }
    // If TECF detectors are present in the set up 
    if (nDetTECBTot > 0) {
      statusTECB = (1 - nDetTECBErr*1.0/nDetTECBTot);
      dname = "SiStrip/MechanicalView/TEC/side_1";
      fillSubDetStatus(dqm_store, dname, 6);  
    }   
    cout <<"# of Det TIB : (tot)"<<setw(5)<<nDetTIBTot<< " (error) "<<nDetTIBErr <<" ==> "<<statusTIB<< endl; 
    cout <<"# of Det TOB : (tot)"<<setw(5)<<nDetTOBTot<< " (error) "<<nDetTOBErr <<" ==> "<<statusTOB<< endl; 
    cout <<"# of Det TIDF: (tot)"<<setw(5)<<nDetTIDFTot<<" (error) "<<nDetTIDFErr<<" ==> "<<statusTIDF<< endl; 
    cout <<"# of Det TIDB: (tot)"<<setw(5)<<nDetTIDBTot<<" (error) "<<nDetTIDBErr<<" ==> "<<statusTIDB<< endl; 
    cout <<"# of Det TECF: (tot)"<<setw(5)<<nDetTECFTot<<" (error) "<<nDetTECFErr<<" ==> "<<statusTECF<< endl; 
    cout <<"# of Det TECB: (tot)"<<setw(5)<<nDetTECBTot<<" (error) "<<nDetTECBErr<<" ==> "<<statusTECB<< endl; 
  } else {

    string dname;
    dname = "SiStrip/MechanicalView/TIB";
    fillSubDetStatus(dqm_store, dname, nMeTIBTot, nMeTIBErr, 1);  
    dname = "SiStrip/MechanicalView/TOB";
    fillSubDetStatus(dqm_store, dname, nMeTOBTot, nMeTOBErr, 2);  
    dname = "SiStrip/MechanicalView/TID/side_2";
    fillSubDetStatus(dqm_store, dname,  nMeTIDFTot, nMeTIDFErr, 3);  
    dname = "SiStrip/MechanicalView/TID/side_1";
    fillSubDetStatus(dqm_store, dname,  nMeTIDBTot, nMeTIDBErr, 4);  
    dname = "SiStrip/MechanicalView/TEC/side_2";
    fillSubDetStatus(dqm_store, dname,  nMeTECFTot, nMeTECFErr, 5);  
    dname = "SiStrip/MechanicalView/TEC/side_1";
    fillSubDetStatus(dqm_store, dname,  nMeTECBTot, nMeTECBErr, 6);  

    nMeTot = nMeTIBTot + nMeTOBTot + nMeTIDFTot + nMeTIDBTot + nMeTECFTot + nMeTECBTot;
    nMeErr = nMeTIBErr + nMeTOBErr + nMeTIDFErr + nMeTIDBErr + nMeTECFErr + nMeTECBErr;
    if (nMeTot > 0) gStatus = (1 - nMeErr*1.0/nMeTot);

    if (nMeTIBTot  > 0) statusTIB  = (1 - nMeTIBErr*1.0/nMeTIBTot);
    if (nMeTOBTot  > 0) statusTOB  = (1 - nMeTOBErr*1.0/nMeTOBTot);
    if (nMeTIDFTot > 0) statusTIDF = (1 - nMeTIDFErr*1.0/nMeTIDFTot);
    if (nMeTIDBTot > 0) statusTIDB = (1 - nMeTIDBErr*1.0/nMeTIDBTot);
    if (nMeTECFTot > 0) statusTECF = (1 - nMeTECFErr*1.0/nMeTECFTot);
    if (nMeTECBTot > 0) statusTECB = (1 - nMeTECBErr*1.0/nMeTECBTot);

    cout <<"# of MEs tested TIB : (tot)"<<setw(5)<<nMeTIBTot<< " (error) "<<nMeTIBErr <<" ==> "<<statusTIB<< endl; 
    cout <<"# of MEs tested TOB : (tot)"<<setw(5)<<nMeTOBTot<< " (error) "<<nMeTOBErr <<" ==> "<<statusTOB<< endl; 
    cout <<"# of MEs tested TIDF: (tot)"<<setw(5)<<nMeTIDFTot<<" (error) "<<nMeTIDFErr<<" ==> "<<statusTIDF<< endl; 
    cout <<"# of MEs tested TIDB: (tot)"<<setw(5)<<nMeTIDBTot<<" (error) "<<nMeTIDBErr<<" ==> "<<statusTIDB<< endl; 
    cout <<"# of MEs tested TECF: (tot)"<<setw(5)<<nMeTECFTot<<" (error) "<<nMeTECFErr<<" ==> "<<statusTECF<< endl; 
    cout <<"# of MEs tested TECB: (tot)"<<setw(5)<<nMeTECBTot<<" (error) "<<nMeTECBErr<<" ==> "<<statusTECB<< endl; 
 
  }
    SummaryReport->Fill(gStatus);

    SummaryTIB->Fill(statusTIB);
    SummaryTOB->Fill(statusTOB);
    SummaryTIDF->Fill(statusTIDF);
    SummaryTIDB->Fill(statusTIDB);
    SummaryTECF->Fill(statusTECF);
    SummaryTECB->Fill(statusTECB);
  
}
//
// -- fill subDetStatus
//
void SiStripActionExecutor::fillSubDetStatus(DQMStore* dqm_store, string& dname, unsigned int xbin) {
  if (SummaryReportMap->kind() != MonitorElement::DQM_KIND_TH2F) return;
  TH2F* hist2 = SummaryReportMap->getTH2F();
  if (!hist2) return;
  if (dqm_store->dirExists(dname)) {
    dqm_store->cd(dname);
    SiStripFolderOrganizer folder_organizer;
    vector<string> subDirVec = dqm_store->getSubdirs();
    unsigned int ybin = 0;
    for (vector<string>::const_iterator ic = subDirVec.begin();
	 ic != subDirVec.end(); ic++) {
      int tot_det = 0, err_det =0;
      dqm_store->cd((*ic));
      vector<string> mids;
      SiStripUtility::getModuleFolderList(dqm_store, mids);
      for (vector<string>::const_iterator im = mids.begin();
           im != mids.end(); im++) {
         uint32_t detId = atoi((*im).c_str());
	 string subdir_path;
	 folder_organizer.getFolderName(detId, subdir_path);
	 vector<MonitorElement*> meVec = dqm_store->getContents(subdir_path);
         if (meVec.size() == 0) continue;
         tot_det++; 
         int err_me = 0;
	 for (vector<MonitorElement*>::const_iterator it = meVec.begin();
	      it != meVec.end(); it++) {
	   MonitorElement * me = (*it);     
	   if (!me) continue;
	   if (me->getQReports().size() == 0) continue;
	   int istat =  SiStripUtility::getMEStatus((*it)); 
	   if (istat == dqm::qstatus::ERROR)   err_me++;
	 }
         if (err_me > 0) err_det++;
      }
      ybin++;
      float eff_fac = 1 - (err_det*1.0/tot_det);
      if (tot_det > 0.0) hist2->SetBinContent(xbin,ybin, eff_fac);
    }
  }
}
void SiStripActionExecutor::fillSubDetStatus(DQMStore* dqm_store, string& dname,  
                            int& tot_me_subdet, int& error_me_subdet, unsigned int xbin) {
  if (SummaryReportMap->kind() != MonitorElement::DQM_KIND_TH2F) return;
  TH2F* hist2 = SummaryReportMap->getTH2F();
  if (!hist2) return;
  if (dqm_store->dirExists(dname)) {
    dqm_store->cd(dname);
    vector<string> subDirVec = dqm_store->getSubdirs();
    unsigned int ybin = 0;
    tot_me_subdet = error_me_subdet = 0;
    for (vector<string>::const_iterator ic = subDirVec.begin();
	 ic != subDirVec.end(); ic++) {
      vector<string> mids;
      SiStripUtility::getModuleFolderList(dqm_store, mids);
      vector<MonitorElement*> meVec;
      
      meVec = dqm_store->getAllContents((*ic));
      int error_me = 0;
      int tot_me = 0;
      for (vector<MonitorElement*>::const_iterator it = meVec.begin();
               it != meVec.end(); it++) {
	MonitorElement * me = (*it);     
	if (!me) continue;
        tot_me++;
        tot_me_subdet++;
	if (me->getQReports().size() == 0) continue;
	int istat =  SiStripUtility::getMEStatus((*it)); 
	if (istat == dqm::qstatus::ERROR)  {
          error_me++;
          error_me_subdet++;
        }
      }
      ybin++;
      float eff_fac = 1 - (error_me*1.0/tot_me);
      if (tot_me > 0.0) hist2->SetBinContent(xbin,ybin, eff_fac);
    }
  }
}
//
// -- create reportSummary MEs
//
void SiStripActionExecutor::resetGlobalStatus() {
  if (bookedGlobalStatus_) {
    
    SummaryReport->Reset();
    
    SummaryReportMap->Reset();
    
    SummaryTIB->Reset();
    SummaryTOB->Reset();
    SummaryTIDF->Reset();
    SummaryTIDB->Reset();
    SummaryTECF->Reset();
    SummaryTECB->Reset();
  }
}
//
// -- go to a given Directory
//
bool SiStripActionExecutor::goToDir(DQMStore * dqm_store, string name) {
  string currDir = dqm_store->pwd();
  string dirName = currDir.substr(currDir.find_last_of("/")+1);
  if (dirName.find(name) == 0) {
    return true;
  }
  vector<string> subDirVec = dqm_store->getSubdirs();
  for (vector<string>::const_iterator ic = subDirVec.begin();
       ic != subDirVec.end(); ic++) {
    dqm_store->cd(*ic);
    if (!goToDir(dqm_store, name))  dqm_store->goUp();
    else return true;
  }
  return false;  
}
