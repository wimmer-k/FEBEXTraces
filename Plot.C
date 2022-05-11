#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TGraph.h"
#include "TLine.h"
#include "TH2F.h"
#include "TF1.h"

#include "/u/kwimmer/progs/Traces/inc/Trace.hh"

void ViewTrace(int n){

  TFile *f = new TFile("tracetest.root");
  TTree* tr = (TTree*)f->Get("tr");

  TraceEvent* trE = new TraceEvent();
  tr->SetBranchAddress("trEvent",&trE);
  TCanvas *c = new TCanvas("c","c",900,400);
  Int_t status = tr->GetEvent(n);
  if(trE->GetMult()>0){
    vector<TGraph*> g;
    g.resize(trE->GetMult());
    for(int t=0;t<trE->GetMult();t++){
    Trace *trace = trE->GetTrace(t);
    if (trace==NULL || trace->GetLength()<1){
      cout << "bad trace, aborting" << endl;
      continue;
    }
    int data[2000];
    int x[2000];
    
    for(int i=0;i<trace->GetLength();i++){
      x[i] = i;
      data[i] = (int)trace->GetTrace()[i];
    }
    g[t] = new TGraph(trace->GetLength(),x,data);
    if(t==0){
      g[t]->Draw("APL");
    } else {
      g[t]->Draw("PL");
    }
    
    }// mult
  }// mult > 0
}
void ViewAllTraces(int ch){

  TFile *f = new TFile("tracetest.root");
  TTree* tr = (TTree*)f->Get("tr");

  TraceEvent* trE = new TraceEvent();
  tr->SetBranchAddress("trEvent",&trE);
  TCanvas *c = new TCanvas("c","c",900,400);
  vector<TGraph*> g;
  TH2F *traces = new TH2F("traces","traces", 2000,0,2000,1200,-10000,10000);
  for(int n=0; n<tr->GetEntries(); n++){
    Int_t status = tr->GetEvent(n);
    if(trE->GetMult()>0){
      g.resize(trE->GetMult());
      for(int t=0;t<trE->GetMult();t++){
	Trace *trace = trE->GetTrace(t);
	cout << trace->GetChn() << endl;
	if(trace->GetChn()==ch){
	  if (trace==NULL || trace->GetLength()<1){
	    cout << "bad trace, aborting" << endl;
	    continue;
	  }
	  for(int j=0;j<trace->GetLength();j++){
	    traces->Fill(j,(int)trace->GetTrace()[j]);
	  }
	  // int data[2000];
	  // int x[2000];
	  
	  // for(int i=0;i<trace->GetLength();i++){
	  //   x[i] = i;
	  //   data[i] = (int)trace->GetTrace()[i];
	    
	  // }
	  // g[t] = new TGraph(trace->GetLength(),x,data);
	  // if(t==0){
	  //   g[t]->Draw("APL");
	  // } else {
	  //   g[t]->Draw("PL");
	  // }
	}// proper channel
      }// mult
    }// mult > 0
  }// entries
  traces->Draw("colz");
}
  
