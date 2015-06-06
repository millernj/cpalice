
#include "TChain.h"
#include "TTree.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TColor.h"
#include "AliAnalysisTask.h"
#include "AliAnalysisManager.h"

#include "AliESDEvent.h"
#include "AliESDInputHandler.h"
#include "AliESDTZERO.h"
#include "AliESDInputHandlerRP.h"
#include "AliTOFcluster.h"
#include "AliCluster.h"
#include "AliTOFGeometry.h"

#include "AliAnalysisTaskPJ.h"

// example of an analysis task creating a p_t spectrum
// Authors: Panos Cristakoglou, Jan Fiete Grosse-Oetringhaus, Christian Klein-Boesing
// Reviewed: A.Gheata (19/02/10)

ClassImp(AliAnalysisTaskPJ)

//________________________________________________________________________
AliAnalysisTaskPJ::AliAnalysisTaskPJ(const char *name) 
: AliAnalysisTaskSE(name), fESD(0), fOutputList(0), fHistPt(0), fHistTOF(0),fHistNumTOFTOT(0),fHistNumTOFTDC(0),
    fHistTotalClusEM(0),fHistTotalClusTOF(0),fHistUnmatchedClusEM(0),fHistDeltaE(0),fHistUnmatchedClusTOF(0),
    fHistDeltaADC(0),fHistNumCC(0),fHistNumTC(0),fHistUnmatchedEMEnergy(0),fHistUnmatchedTOF(0),
    fHistUnmatchedTOFEnergy(0),fEvtNum(0),fHistNum(0)

{
  // Constructor
  for (int i = 0; i < 100; i++) {
    fHistEtaPhiCC[i] = 0;
    fHistEtaPhiTC[i] = 0;
  }

  // Define input and output slots here
  // Input slot #0 works with a TChain
  DefineInput(0, TChain::Class());
  // Output slot #0 id reserved by the base class for AOD
  // Output slot #1 writes into a TH1 container
  DefineOutput(1, TList::Class());
}


//________________________________________________________________________
AliAnalysisTaskPJ::AliAnalysisTaskPJ() 
  : AliAnalysisTaskSE(), fESD(0), fOutputList(0), fHistPt(0), fHistTOF(0),fHistNumTOFTOT(0),fHistNumTOFTDC(0),
    fHistTotalClusEM(0),fHistTotalClusTOF(0),fHistUnmatchedClusEM(0),fHistDeltaE(0),fHistUnmatchedClusTOF(0),
    fHistDeltaADC(0),fHistNumCC(0),fHistNumTC(0),fHistUnmatchedEMEnergy(0),fHistUnmatchedTOF(0),
    fHistUnmatchedTOFEnergy(0),fEvtNum(0),fHistNum(0)
{
  // Default Constructor
  for (int i = 0; i < 100; i++) {
    fHistEtaPhiCC[i] = 0;
    fHistEtaPhiTC[i] = 0;
  }
  
}

//________________________________________________________________________
void AliAnalysisTaskPJ::UserCreateOutputObjects()
{
  // Create histograms
  // Called once

  fOutputList = new TList();
  fOutputList->SetOwner(); // otherwise it produces leaks in merging
  
  fHistPt = new TH1F("fHistPt", "P_{T} distribution", 100, 0.1, 10.1);
  fHistPt->GetXaxis()->SetTitle("P_{T} (GeV/c)");
  fHistPt->GetYaxis()->SetTitle("dN/dP_{T} (c/GeV)");
  fHistPt->SetMarkerStyle(kFullCircle);  

  fHistTOF = new TH2F("fHistTOF", "Unmatched TOF Eta and Phi", 100, -.7, .7, 100, 0, .75*TMath::Pi());
  fHistTOF->GetXaxis()->SetTitle("Eta");
  fHistTOF->GetYaxis()->SetTitle("Phi");

  fHistTotalClusEM = new TH1F("fHistTotalClusEM", "Total # of Clusters per Event in EMCAL", 200, 0, 200);
  fHistTotalClusEM->GetXaxis()->SetTitle("# of Clusters per Event");
  fHistTotalClusEM->GetYaxis()->SetTitle("Counts");

  fHistTotalClusTOF = new TH1F("fHistTotalClusTOF", "Total # of Clusters per Event in TOF Restricted to EMCAL ROI", 200, 0, 200);
  fHistTotalClusTOF->GetXaxis()->SetTitle("# of Clusters per Event");
  fHistTotalClusTOF->GetYaxis()->SetTitle("Counts");

  fHistUnmatchedClusEM = new TH1F("fHistUnmatchedClusEM", "Total # of Unmatched Clusters per Event in EMCAL", 6, 40, 46);
  fHistUnmatchedClusEM->GetXaxis()->SetTitle("# of Unmatched Clusters per Event");
  fHistUnmatchedClusEM->GetYaxis()->SetTitle("Counts");

  fHistUnmatchedClusTOF = new TH1F("fHistUnmatchedClusTOF", "Total # of Unmatched Clusters per Event in TOF Restricted to EMCAL ROI", 5, 4, 9);
  fHistUnmatchedClusTOF->GetXaxis()->SetTitle("# of Unmatched Clusters per Event");
  fHistUnmatchedClusTOF->GetYaxis()->SetTitle("Counts");

  fHistUnmatchedTOFEnergy = new TH1F("fHistUnmatchedTOFEnergy", "TOF Calculated Energy Distribution for Unmatched Clusters with an $e^-$ Mass Assumption", 1000000, 0, 1000*1E9);
  fHistUnmatchedTOFEnergy->GetXaxis()->SetTitle("Calculated Energy(eV)");
  fHistUnmatchedTOFEnergy->GetYaxis()->SetTitle("Counts");

  fHistDeltaE = new TH2F("fHistDeltaE", "Unmatched EmCal Eta and Phi", 100, -.7, .7, 100, 0, .75*TMath::Pi());
  fHistDeltaE->GetXaxis()->SetTitle("Eta");
  fHistDeltaE->GetYaxis()->SetTitle("Phi");

  fHistUnmatchedEMEnergy = new TH1F("fHistUnmatchedEMEnergy", "Unmatched EMCAL Cluster Energy Distribution", 100000, 0, 1000);
  fHistUnmatchedEMEnergy->GetXaxis()->SetTitle("Energy(eV)");
  fHistUnmatchedEMEnergy->GetYaxis()->SetTitle("Counts");

  fHistDeltaADC = new TH2F("fHistDeltaADC", "2D ADC-DeltaR", 10000, 0, 5, 10000, 100, 300);
  fHistDeltaADC->GetXaxis()->SetTitle("Delta R");
  fHistDeltaADC->GetYaxis()->SetTitle("TOF ADC");

  fHistUnmatchedTOF = new TH1F("fHistUnmatchedTOF", "Time of Flight to TOF for Unmatched Clusters", 250000, 0, 500);
  fHistUnmatchedTOF->GetXaxis()->SetTitle("Time(ns)");
  fHistUnmatchedTOF->GetYaxis()->SetTitle("Counts");

  for (int i = 0; i < 100; i++) {
    char name[100];sprintf(name,"fHistEtaPhiCC%d",i);
    fHistEtaPhiCC[i] = new TH2F(name,name,100,-.9,.9,100,100,190);
    fHistEtaPhiCC[i]->GetXaxis()->SetTitle("#eta");
    fHistEtaPhiCC[i]->GetYaxis()->SetTitle("#phi"); }


  for (int i = 0; i < 100; i++) {
    char name[100]; sprintf(name,"fHistEtaPhiTC%d",i);
    fHistEtaPhiTC[i] = new TH2F(name,name,100,-.9,.9,100,100,190);
    fHistEtaPhiTC[i]->GetXaxis()->SetTitle("#eta");
    fHistEtaPhiTC[i]->GetYaxis()->SetTitle("#phi");
  }

  fHistNumCC = new TH1F("fHistNumCC", "# of Calo clusters per event", 100,0,100);
  fHistNumCC->GetXaxis()->SetTitle("# Calo Clusters");
  fHistNumCC->GetYaxis()->SetTitle("Frequency");

  fHistNumTC = new TH1F("fHistNumTC", "# of EMCAL TOF clusters per event", 100,0,100);
  fHistNumTC->GetXaxis()->SetTitle("# EMCAL TOF Clusters");
  fHistNumTC->GetYaxis()->SetTitle("Frequency");

  fHistNumTOFTOT =  new TH1F("fHistNumTC", "# of TOF TOT clusters per event", 100,0,100);
  fHistNumTOFTOT -> GetYaxis()->SetTitle("# TOF TOT Clusters");
  fHistNumTOFTOT -> GetXaxis()->SetTitle("TOF TOT (ns)");

  fHistNumTOFTDC =  new TH1F("fHistNumTC", "# of TOF TDC clusters per event", 100,0,100);
  fHistNumTOFTDC -> GetYaxis()->SetTitle("# TOF TDC Clusters");
  fHistNumTOFTDC -> GetXaxis()->SetTitle("TOF TDC (ns)");

  fOutputList->Add(fHistPt);
  for (int i = 0; i < 100; i++) {fOutputList->Add(fHistEtaPhiCC[i]); fOutputList->Add(fHistEtaPhiTC[i]);}
  fOutputList->Add(fHistNumCC);
  fOutputList->Add(fHistNumTC);
  fOutputList->Add(fHistNumTOFTOT);
  fOutputList->Add(fHistNumTOFTDC);
  PostData(1,fOutputList); // important for merging
}

//________________________________________________________________________
void AliAnalysisTaskPJ::UserExec(Option_t *) 
{
  // Main loop
  // Called for each event

  if (!fInputEvent) {
    printf("ERROR: Input event not available\n");
    return;
  }
  
  // Get rec points
  AliESDInputHandlerRP *handler = 
    dynamic_cast<AliESDInputHandlerRP*>(AliAnalysisManager::GetAnalysisManager()->GetInputEventHandler());
  if (!handler) { printf("No RP handler\n"); return; }
  AliESDEvent *esd  = handler->GetEvent();
  if (!esd) { printf("No AliESDEvent\n"); return; }
  
  Double32_t T0 = esd->GetT0();
  //get reconstructed vertex position
  Double_t vertex_position[3];
  esd->GetVertex()->GetXYZ(vertex_position);
  
  TTree* tofClusterTree = handler->GetTreeR("TOF");
  TTree* Tree = handler->GetTreeR("TOF");
  if (!tofClusterTree) {printf("<WARN>No TOF clusters!\n");return;}
  
  TBranch* tofClusterBranch = tofClusterTree->GetBranch("TOF");
  if (!tofClusterBranch) {printf("Can't get the branch with the TOF digits !\n"); return;}
  
  TClonesArray dummy("AliTOFcluster",1000), * tofClusters = &dummy; tofClusterBranch->SetAddress(&tofClusters);
  
  tofClusterTree->GetEvent(0);//  this may be the error
  
  Int_t nClusters = tofClusters->GetEntriesFast(); printf("<INFO>NTOFclusters=%d\n",nClusters);
  int countinEMCal = 0;
  cout<<"TOF Clusters: " <<tofClusters->GetEntriesFast()<<endl;
  // loop over clusters
  //  while (nClusters--) {
  for (Int_t iClusters = 0; iClusters < nClusters; iClusters++) { 
  AliTOFcluster *cluster=(AliTOFcluster*)tofClusters->UncheckedAt(iClusters);
    if(cluster) {
      Float_t x = cluster->GetR()*TMath::Cos(cluster->GetPhi());
      Float_t y = cluster->GetR()*TMath::Sin(cluster->GetPhi());
      Float_t z = cluster->GetZ();
      TVector3 vpos(x,y,z);
      Double_t cphi = vpos.Phi();
      if(cphi < 0) cphi +=TMath::TwoPi();
      Double_t ceta = vpos.Eta();

      if (cluster->GetPhi() > 1.35 && cluster->GetPhi() < 3.15 && ceta < 0.7 && ceta > -0.7) {
	countinEMCal++; }
    }
  }
  fHistTotalClusTOF->Fill(countinEMCal);
  
  
  printf("<INFO>Ntracks=%d\n",fInputEvent->GetNumberOfTracks());

  // Track loop to fill a pT spectrum
  for (Int_t iTracks = 0; iTracks < fInputEvent->GetNumberOfTracks(); iTracks++) {
    //AliVParticle* track = fInputEvent->GetTrack(iTracks);
    AliESDtrack* track = (AliESDtrack*)fInputEvent->GetTrack(iTracks);
    if (!track) { printf("ERROR: Could not receive track %d\n", iTracks);continue;}
    printf("Track %d has been matched to EMCAL %d", track->GetID(), track->GetEMCALcluster());
    //AliTOFcluster* tof = (AliTOFcluster*);
    //tof.fIdx = track->GetTOFcluster();
    //EmCal = new AliESDCaloCluster();
    //EmCal->SetID(track->GetEMCALcluster());
    //cout<<"EMCCal Energy "<<EmCal->E();
    //Int_t tofIndex = track->GetTOFcluster();
    // printf("\t<I>Track %d has tofIndex %d\n",iTracks,tofIndex);
    fHistPt->Fill(track->Pt());
    //fHistDeltaE->Fill(tof->GetADC(), EmCal->E());
  } //track loop
 
  
  //Init geometry and array that will contain the clusters
  TRefArray* caloClusters = new TRefArray();
  //Pass the geometry transformation matrix from ESDs to geometry
  
  // cell array
  //  AliVCaloCells &cells= *(fInputEvent->GetEMCALCells());
  
  //Get CaloClusters Array
  // GetEMCALClusters will provide only the EMCAL clusters where GetNCaloClusters
  // returns the total number EMCAL + PHOS
  // Bool_t IsEMCAL();
  fInputEvent->GetEMCALClusters(caloClusters);
  Int_t nclus = caloClusters->GetEntries();   
  //Int_t nCells = clus->GetNCells();
  fHistTotalClusEM->Fill(nclus);
  Bool_t matchedTOF[tofClusters->GetEntriesFast()];
  Bool_t unMatchedTOF[tofClusters->GetEntriesFast()];
  Bool_t unMatchedEmCal[nclus];
  Bool_t matchedEmCal;
  Int_t UnmatchedTOF;
  Int_t UnmatchedEM;

  matchedEmCal = false;
  for (Int_t icl = 0; icl < nclus; icl++) {    
    AliVCluster* clus = (AliVCluster*)caloClusters->At(icl);
    Int_t nCells = clus->GetNCells();
    //Float_t energy = clus->E();
    Float_t pos[3];
    clus->GetPosition(pos);
    TVector3 vpos(pos[0],pos[1],pos[2]);
    //cout<< "Pos: " <<"X: "<<pos[0]<<","<< "Y: "<<pos[1]<<"Z: "<<pos[2] <<endl;
    //cout<<"Pos R: "<< sqrt((pos[0]*pos[0])+(pos[1]*pos[1])+(pos[2]*pos[2]))<<endl;
    Double_t cphi = vpos.Phi();
    if(cphi < 0) cphi +=TMath::TwoPi();
    Double_t ceta = vpos.Eta();
    // cout<<"Calo Cluster Phi Value is:  "<<cphi<<endl; cout<<"Calo Cluster Eta Value is: "<<ceta<<endl;
    
   // if (fEvtNum%2000 == 0 && fHistNum < 100) {fHistEtaPhiCC[fHistNum]->Fill(cphi, ceta);}
    
    // Time of Flight (TOF)
    Double_t EmCaltof = clus->GetTOF();
    Double_t EmCalEnergy = clus->E();
    //fHistTOF->Fill(EmCaltof);
    //Print basic cluster information
    cout << "Cluster: " << icl+1 << "/" << nclus << "Phi: " << 
       cphi*TMath::RadToDeg() << "; Eta: " << ceta << "; NCells: " << nCells << endl;
    cout<< "Number of TofClusters: "<< tofClusters->GetEntriesFast()<<endl<<endl;   /* } calocluster loop */

    //Finding Delta R for Emcal and Tof points
    for (Int_t iToFTrack = 0; iToFTrack <  tofClusters->GetEntriesFast(); iToFTrack++) 
      {//cout<<"iToFTrack = "<<iToFTrack<<endl;
	AliTOFcluster *cluster=(AliTOFcluster*)tofClusters->UncheckedAt(iToFTrack);
	Float_t time =(AliTOFGeometry::TdcBinWidth()*cluster->GetTDC())*1E-3; // in ns
	Float_t tot = (AliTOFGeometry::TdcBinWidth()*cluster->GetToT())*1E-3;//in ns
	fHistNumTOFTOT-> Fill(tot);
	fHistNumTOFTDC->Fill(time);
	//TOF x,y,z
	Float_t TOFx = cluster->GetR()*TMath::Cos(cluster->GetPhi());
	Float_t TOFy = cluster->GetR()*TMath::Sin(cluster->GetPhi());
	Float_t TOFz = cluster->GetZ();
	
	//Double_t p[3];
	//cluster->GetPxPyPz(p);
	//cout<<"Px= "<<p[0]<<"Py="<<p[1]<<"Pz= "<<p[2];
	//Double_t momentum = TMath::Sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
	
	TVector3 TOFvpos(TOFx,TOFy,TOFz);
	Double_t TOFphi = cluster->GetPhi();
	if(TOFphi < 0) {TOFphi +=TMath::TwoPi();}
	//if((momentum != TMath::Abs(p[2]))&&(momentum != 0)){TOFeta = 0.5*TMath::Log((momentum + p[2])/(momentum - p[2]));}
	Double_t TOFeta = TOFvpos.Eta();
	Double_t TOFADC = cluster->GetADC();
	//cout<<"ceta: "<<ceta<<endl;
	//cout<<"TOFeta: "<<TOFeta<<endl;
	//cout<<"cphi: "<<cphi<<endl;
	//cout<<"TOFphi: "<<TOFphi<<endl;
	if (TOFphi < 3.15 && TOFphi > 1.35 && TOFeta < .7 && TOFeta > -.7 && time > 12.33333)
	  {
	    Double_t R1 = TOFeta;
	//cout<<"R1: "<<R1<<endl;
	    Double_t R2 = TOFphi;
	//cout<<"R2: "<<R2<<endl;
	    Double_t Rtof = sqrt(pow(R1, 2) + pow(R2,2));
	    //cout<<"DeltaR: "<<DeltaR<<endl;
        Double_t DeltaR = sqrt(pow(ceta-R1, 2) + pow(cphi-R2, 2));
        //These two if statements are for deciding which data you want to see.
	//Putting the fill in the first one will show all matched cluster and the second, all unmatched clusters
	if (abs(DeltaR)<.1 && matchedTOF[iToFTrack] == false)
        {
	  matchedTOF[iToFTrack] = true;
        matchedEmCal = true;
        }
        if ( icl+1==nclus && matchedTOF[iToFTrack] == false)
	{
		UnmatchedTOF++;
		fHistUnmatchedTOF->Fill(time);
		if (fEvtNum%2000 == 0 && fHistNum < 100){fHistEtaPhiTC[fHistNum]->Fill(TOFeta, TOFphi*180.0/TMath::Pi());}
	  unMatchedTOF[iToFTrack] = true;
	}

	  }
        if(matchedEmCal == false){
	if (fEvtNum%2000 == 0 && fHistNum < 100){
	fHistEtaPhiCC[fHistNum]->Fill(ceta, cphi*180.0/TMath::Pi());} 
	UnmatchedEM++;
	unMatchedEmCal[icl]=true;
	fHistUnmatchedEMEnergy->Fill(EmCalEnergy*1E3);}
	  }
  }
//This is all TOF e- Mass Assumotion Stuff
  for(Int_t icl=0; icl<nclus; icl++)
    {
      AliVCluster* clus = (AliVCluster*)caloClusters->At(icl);
      if(unMatchedEmCal[icl]==true)
	{
	  Double_t energy = clus->E();
	  for(Int_t iToFTrack = 0; iToFTrack<tofClusters->GetEntriesFast(); iToFTrack++)
	      {
		AliTOFcluster *cluster=(AliTOFcluster*)tofClusters->UncheckedAt(iToFTrack);
		Float_t TOFx = cluster->GetR()*TMath::Cos(cluster->GetPhi());
		Float_t TOFy = cluster->GetR()*TMath::Sin(cluster->GetPhi());
		Float_t TOFz = cluster->GetZ();

		TVector3 TOFvpos(TOFx,TOFy,TOFz);
		Double_t TOFeta = TOFvpos.Eta();
		Double32_t time =(AliTOFGeometry::TdcBinWidth()*cluster->GetTDC())*1E-3; // in ns
		Float_t tot = (AliTOFGeometry::TdcBinWidth()*cluster->GetToT())*1E-3;//in ns
		if(unMatchedTOF[iToFTrack]==true)
		  {
		    if (time!=0){
		      cout<<"Time"<<time<<"\n";
		      Double_t veloc = ((3.70/(TMath::Sin(2*TMath::ATan(TMath::Exp(-TOFeta)))))/((time)*1E-9));
		      cout<<"Velocity"<<veloc<<"\n";
		     Double_t elecmass = (.511*1E6);
		     Double_t c = (3.00*1E8);
		     cout<<(1-pow((veloc/c),2))<<"\n";
		     if((pow((veloc/c),2))<1){
		       cout<<"EMCAL Energy"<<energy*1E6<<"\n";
		       Double_t elecenergycalc = sqrt(pow((elecmass*pow(c, 2)),2)+pow((c*elecmass*veloc/sqrt(1-pow((veloc/c),2))),2));
		       cout<<"Energy TOF"<<(abs(elecenergycalc*1E-12))<<"\n";
		       fHistUnmatchedTOFEnergy->Fill(elecenergycalc*1E-12);
		     }
		     }
		    }
		  }
	      }
    }

  fHistUnmatchedClusTOF->Fill(UnmatchedTOF);
  fHistUnmatchedClusEM->Fill(UnmatchedEM);
  //clean up to avoid mem leaks
  delete tofClusterTree;
  //Keep every 2000th event's info in histogram
  if (fEvtNum%2000 == 0) fHistNum++;
  //increment event counter
  fEvtNum++;
  PostData(1, fOutputList);
  
}

//________________________________________________________________________
void AliAnalysisTaskPJ::Terminate(Option_t *) 
{
  // Draw result to the screen
  // Called once at the end of the query
  
  fOutputList = dynamic_cast<TList*> (GetOutputData(1));
  if (!fOutputList) {printf("ERROR: Output list not available\n");return;}
  
  fHistPt = dynamic_cast<TH1F*> (fOutputList->At(0));
  if (!fHistPt) {printf("ERROR: fHistPt not available\n");return;}
 
  /*  TCanvas *h = new TCanvas("histoEtaPhi", "EtaPhi", 10,10,510,510);
  h->cd();
  char str[32];
  for(int i=0;i<101;i++){
  sprintf(str, "./EtaPhiHistos/TofEtaPhi%d.pdf", i);
  h->Clear();
  fHistEtaPhiTC[i]->Draw();
  h->SaveAs(str);
  sprintf(str, "./EtaPhiHistos/EMCalEtaPhi%d.pdf", i);
  h->Clear(); 
  fHistEtaPhiCC[i]->Draw();
  h->SaveAs(str);
  }

   
  TCanvas *c1 = new TCanvas("AliAnalysisTaskPJ","Pt",10,10,510,510);
  c1->cd(1)->SetLogy();fHistPt->DrawCopy("E");
  
  //TCanvas *c2 = new TCanvas("histo","TOF",10,10,510,510);
  //c2->cd(); fHistTOF->Draw();
  
  TCanvas *c3 = new TCanvas("histototalclusem", "Total # of Clusters per Event in Emcal", 10,10,510,510);
  c3->cd(); fHistTotalClusEM->Draw();
  
  TCanvas *c2 = new TCanvas("histoTOFunmatchedclus","Total # of Unmatched Clusters per Event in TOF Restricted to EMCAL ROI",10,10,510,510);
  c2->cd(); fHistUnmatchedClusTOF->Draw();
  */
  TCanvas *c4 = new TCanvas("histoEMEnergy","Energy Distribution for Unmatched Clusters in Emcal",10,10,510,510);
  c4->cd(); fHistUnmatchedEMEnergy->Draw();
  
  /*TCanvas *c5 = new TCanvas("histoTOFtotalclus", "Total # of Clusters per Event in TOF Restricted to EMCAL ROI", 10,10,510,510);
  c5->cd(); fHistTotalClusTOF->Draw();
  
  TCanvas *c9 = new TCanvas("histoEMUnmatchedclus", "Total # of Unmatched Clusters per event in Emcal", 10,10,510,510);
  c9->cd(); fHistUnmatchedClusEM->Draw();
  */
  TCanvas *c6 = new TCanvas("histoTOFEnergy", "Calculated Energy Distribution for Unmatched Clusters in TOF based on an $e^-$ Mass Assumption", 10,10,510,510);
  c6->cd();fHistUnmatchedTOFEnergy->Draw();
  /*
  TCanvas *c7 = new TCanvas("histoUnmatchedTOFflight", "Time of Flight Distribution for Unmatched TOF Clusters", 10, 10, 510, 510);
  c7->cd();fHistUnmatchedTOF->Draw();
  */
}
