#include <Validation/HcalDigis/interface/HcalDigisClient.h>
// -*- C++ -*-
//
// Package:    HcalDigisClient
// Class:      HcalDigisClient
//
/**\class HcalDigisClient HcalDigisClient.cc Validation/HcalDigisClient/src/HcalDigisClient.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
 */
//
// Original Author:  Ali Fahim,22 R-013,+41227672649,
//         Created:  Wed Mar 23 22:54:28 CET 2011
//
//


// system include files

HcalDigisClient::HcalDigisClient(const edm::ParameterSet& iConfig) {
    outputFile_ = iConfig.getUntrackedParameter<std::string > ("outputFile", "HcalDigisClient.root");
    dirName_ = iConfig.getParameter<std::string > ("DQMDirName");
    msm_ = new std::map<std::string, MonitorElement*>();


}


HcalDigisClient::~HcalDigisClient() {
    delete msm_;
}


void HcalDigisClient::booking(DQMStore::IBooker &ib, std::string subdetopt) {

    std::string strtmp;
    HistLim ietaLim(85, -42.5, 42.5);

    for (int depth = 1; depth <= 4; depth++) {
        strtmp = "HcalDigiTask_occupancy_vs_ieta_depth" + str(depth) + "_" + subdetopt;
        book1D(ib,strtmp, ietaLim);
    }
    
}

void HcalDigisClient::runClient(DQMStore::IBooker &ib, DQMStore::IGetter &ig) {
    ig.setCurrentFolder(dirName_);
    std::vector<MonitorElement*> hcalMEs;
    // Since out folders are fixed to three, we can just go over these three folders
    // i.e., CaloTowersV/CaloTowersTask, HcalRecHitsV/HcalRecHitTask, NoiseRatesV/NoiseRatesTask.
    std::vector<std::string> fullPathHLTFolders = ig.getSubdirs();
    for (unsigned int i = 0; i < fullPathHLTFolders.size(); i++) {
        ig.setCurrentFolder(fullPathHLTFolders[i]);
        std::vector<std::string> fullSubPathHLTFolders = ig.getSubdirs();
        for (unsigned int j = 0; j < fullSubPathHLTFolders.size(); j++) {
            if (strcmp(fullSubPathHLTFolders[j].c_str(), "HcalDigisV/HcalDigiTask") == 0) {
                hcalMEs = ig.getContents(fullSubPathHLTFolders[j]);
                if (!HcalDigisEndjob(hcalMEs, "HB")) 
		  edm::LogError("HcalDigisClient") << "Error in HcalDigisEndjob! HB"; 
                if (!HcalDigisEndjob(hcalMEs, "HE")) 
		  edm::LogError("HcalDigisClient") << "Error in HcalDigisEndjob! HE"; 
                if (!HcalDigisEndjob(hcalMEs, "HO")) 
		  edm::LogError("HcalDigisClient") << "Error in HcalDigisEndjob! HO"; 
                if (!HcalDigisEndjob(hcalMEs, "HF")) 
		  edm::LogError("HcalDigisClient") << "Error in HcalDigisEndjob! HF";             }
        }
    }
}

int HcalDigisClient::HcalDigisEndjob(const std::vector<MonitorElement*> &hcalMEs, std::string subdet_) {

    using namespace std;
    string strtmp;


    MonitorElement * nevtot(0);
    MonitorElement * ieta_iphi_occupancy_map1(0);
    MonitorElement * ieta_iphi_occupancy_map2(0);
    MonitorElement * ieta_iphi_occupancy_map3(0);
    MonitorElement * ieta_iphi_occupancy_map4(0);


    // std::cout << " Number of histos " <<     hcalMEs.size() << std::endl;

    for (unsigned int ih = 0; ih < hcalMEs.size(); ih++) {
      
         if (hcalMEs[ih]->getName() == "nevtot") nevtot = hcalMEs[ih];

         strtmp = "HcalDigiTask_ieta_iphi_occupancy_map_depth1_" + subdet_;
         if (hcalMEs[ih]->getName() == strtmp) ieta_iphi_occupancy_map1 = hcalMEs[ih];
         strtmp = "HcalDigiTask_ieta_iphi_occupancy_map_depth2_" + subdet_;
         if (hcalMEs[ih]->getName() == strtmp) ieta_iphi_occupancy_map2 = hcalMEs[ih];
         strtmp = "HcalDigiTask_ieta_iphi_occupancy_map_depth3_" + subdet_;
         if (hcalMEs[ih]->getName() == strtmp) ieta_iphi_occupancy_map3 = hcalMEs[ih];
         strtmp = "HcalDigiTask_ieta_iphi_occupancy_map_depth4_" + subdet_;
         if (hcalMEs[ih]->getName() == strtmp) ieta_iphi_occupancy_map4 = hcalMEs[ih];

    }

    if (nevtot                   == 0 ||
	ieta_iphi_occupancy_map1 == 0 ||
	ieta_iphi_occupancy_map2 == 0 ||
	ieta_iphi_occupancy_map3 == 0 ||
	ieta_iphi_occupancy_map4 == 0   
	) {
      edm::LogError("HcalDigisClient") << "No nevtot or maps histo found..."; 
      return 0;
    }

    int ev = nevtot->getEntries();
    if(ev <= 0) {
      edm::LogError("HcalDigisClient") << "normalization factor <= 0!"; 
      return 0;
    }

    float fev = (float) nevtot->getEntries();

    float sumphi_1, sumphi_2, sumphi_3, sumphi_4;
    float phi_factor;
    float cnorm;

    int nx = ieta_iphi_occupancy_map1->getNbinsX();
    int ny = ieta_iphi_occupancy_map1->getNbinsY();
    
    for (int i = 1; i <= nx; i++) {
        for (int j = 1; j <= ny; j++) {

            // occupancies

            strtmp = "HcalDigiTask_ieta_iphi_occupancy_map_depth1_" + subdet_;
            cnorm = ieta_iphi_occupancy_map1->getBinContent(i, j) / fev;
            ieta_iphi_occupancy_map1->setBinContent(i, j, cnorm);

            strtmp = "HcalDigiTask_ieta_iphi_occupancy_map_depth2_" + subdet_;
            cnorm = ieta_iphi_occupancy_map2->getBinContent(i, j) / fev;
            ieta_iphi_occupancy_map2->setBinContent(i, j, cnorm);

            strtmp = "HcalDigiTask_ieta_iphi_occupancy_map_depth3_" + subdet_;
            cnorm = ieta_iphi_occupancy_map3->getBinContent(i, j) / fev; 
            ieta_iphi_occupancy_map3->setBinContent(i, j, cnorm);

            strtmp = "HcalDigiTask_ieta_iphi_occupancy_map_depth4_" + subdet_;
            cnorm = ieta_iphi_occupancy_map4->getBinContent(i, j) / fev; 
            ieta_iphi_occupancy_map4->setBinContent(i, j, cnorm);

        }
    }	    

    for (int i = 1; i <= 82; i++) {

        int ieta = i - 42; // -41 -1, 0 40
        if (ieta >= 0) ieta += 1; // -41 -1, 1 41  - to make it detector-like

        if (ieta >= -20 && ieta <= 20) {
          phi_factor = 72.; 
        } else {
          if (ieta >= 40 || ieta <= -40) 
              phi_factor = 18.; 
           else 
              phi_factor = 36.; 
        }    
	
        sumphi_1 = 0.;
        sumphi_2 = 0.;
        sumphi_3 = 0.;
        sumphi_4 = 0.;

        for (int iphi = 1; iphi <= 72; iphi++) {
            sumphi_1 += ieta_iphi_occupancy_map1->getTH1()->GetBinContent(ieta_iphi_occupancy_map1->getTH1()->FindFixBin(double(ieta),double(iphi)));
            sumphi_2 += ieta_iphi_occupancy_map2->getTH1()->GetBinContent(ieta_iphi_occupancy_map2->getTH1()->FindFixBin(double(ieta),double(iphi)));
            sumphi_3 += ieta_iphi_occupancy_map3->getTH1()->GetBinContent(ieta_iphi_occupancy_map3->getTH1()->FindFixBin(double(ieta),double(iphi)));
            sumphi_4 += ieta_iphi_occupancy_map4->getTH1()->GetBinContent(ieta_iphi_occupancy_map4->getTH1()->FindFixBin(double(ieta),double(iphi)));
        }
 
        //REMOVED (JRD) if (ieta >= 0) ieta -= 1; // -41 -1, 0 40  - to bring back to strtmp num !!! 
        double deta = double(ieta);

        // occupancies vs ieta
        cnorm = sumphi_1 / phi_factor;
        strtmp = "HcalDigiTask_occupancy_vs_ieta_depth1_" + subdet_;
        fill1D(strtmp, deta, cnorm);

        cnorm = sumphi_2 / phi_factor;
        strtmp = "HcalDigiTask_occupancy_vs_ieta_depth2_" + subdet_;
        fill1D(strtmp, deta, cnorm);

        cnorm = sumphi_3 / phi_factor;
        strtmp = "HcalDigiTask_occupancy_vs_ieta_depth3_" + subdet_;
        fill1D(strtmp, deta, cnorm);

        cnorm = sumphi_4 / phi_factor;
        strtmp = "HcalDigiTask_occupancy_vs_ieta_depth4_" + subdet_;
        fill1D(strtmp, deta, cnorm);

    } // end of i-loop

  return 1;
}

MonitorElement* HcalDigisClient::monitor(std::string name) {
    if (!msm_->count(name)) return NULL;
    else return msm_->find(name)->second;
}

std::string HcalDigisClient::str(int x) {
    std::stringstream out;
    out << x;
    return out.str();
}

double HcalDigisClient::integralMETH2D(MonitorElement* ME, int i0, int i1, int j0, int j1) {
    double sum(0);
    for (int i = i0; i <= i1; i++) {
        for (int j = j0; j <= j1; j++) {
            sum += ME->getBinContent(i, j);
        }
    }

    return sum;
}

void HcalDigisClient::scaleMETH2D(MonitorElement* ME, double s) {
    int nx = ME->getNbinsX();
    int ny = ME->getNbinsY();

    double content(0);
    for (int i = 1; i <= nx; i++) {
        for (int j = 1; j <= ny; j++) {
            content = ME->getBinContent(i, j);
            content *= s;
            ME->setBinContent(i, j, content);
        }
    }
}

//define this as a plug-in
DEFINE_FWK_MODULE(HcalDigisClient);

