#include <stdlib.h>
#include "Riostream.h"

// root
#include "TH1.h"
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"

// go4
#include "TGo4Version.h"
#include "TGo4MbsFile.h"
#include "TGo4MbsEvent.h"
#include "TGo4MbsSubEvent.h"

#include "CommandLineInterface.hh"

// febex structs
#include "FEBEX.h"
#include "Trace.hh"


int* UnpackWR(int*, TraceEvent*);
void Unpack(int* ,TraceEvent*);
int checkWR[4] = {0x3e1, 0x4e1, 0x5e1, 0x6e1};


int main(int argc, char** argv){

  char* InputFile = NULL;
  char* OutputFile = NULL;
  int vl = 0;
  
  CommandLineInterface* interface = new CommandLineInterface();

  interface->Add("-i", "inputfile", &InputFile);
  interface->Add("-o", "outputfile", &OutputFile);
  interface->Add("-v", "verbose", &vl);
  interface->CheckFlags(argc, argv);

  if(InputFile == NULL || OutputFile == NULL){
    cerr<<"You have to provide at least the input file and the output file!"<<endl;
    exit(1);
  }
  cout<<"input file: " << InputFile <<endl;
  cout<<"output file: "<<OutputFile<< endl;
  
  TGo4EventSource* source= new TGo4MbsFile(InputFile);
  TGo4MbsEvent* event = new TGo4MbsEvent();
  event->SetEventSource(source);
  event->Init();
  event->ResetIterator();
  
  TFile *ofile = new TFile(OutputFile,"RECREATE");
  TTree* tree = new TTree("tr","Traces");
  TraceEvent * trEvent = new TraceEvent();
  tree->Branch("trEvent",&trEvent, 320000);
  tree->BranchRef();
  
  TGo4MbsSubEvent* psubevt = new TGo4MbsSubEvent();
  for(int i =0; i <100;i++){
    int errmes = event->Fill();
    //cout << "event->Fill() " << errmes << endl;
    //cout << "event->GetCount() " << event->GetCount() << endl;
    event->ResetIterator();
    int subevent_ctr = 0;
    Int_t* p = 0;
    Int_t len = 0; 
    Int_t ID = -1;
    Int_t sub_len = 0;
    // loop subevents
    while( (psubevt = event->NextSubEvent()) != 0){ 
      p = psubevt->GetDataField();
      len = psubevt->GetIntLen();
      ID = psubevt->GetProcid();
      sub_len = (psubevt->GetDlen() - 2) / 2;
      if(vl>2){
	cout << (hex) << "ID " << ID << "\t" << (dec) << ID << endl;
	cout << (hex) << "len " << len << "\t" << (dec) << len << endl;
	cout << (hex) << "sub_len " << sub_len << "\t" << (dec) << sub_len << endl;
      }
      if(ID == GEID){
	if(vl>1){
	  psubevt->PrintEvent();
	  psubevt->PrintMbsSubevent();
	  cout << "--------------------------------" << endl;
	}
    	trEvent->Clear();
	if(withWR){
	  p = UnpackWR(p,trEvent);
	  if(vl>1)
	    cout << "WR TS = " << trEvent->GetWhiteRabbit() << endl;
	}
    	Unpack(p,trEvent);
	//cout << "fill " << endl;
	tree->Fill();
    	psubevt->Clear();
      }
      //cout << "subevent_ctr " << ++subevent_ctr << endl;
      psubevt->Clear();
    }// sub event
    //cout << "i\t" << i << endl;
  }// event loop
  cout << "end " << endl;
  ofile->cd();
  tree->Write("",TObject::kOverwrite);
  ofile->Close();
  
  
}

int* UnpackWR(int *p, TraceEvent* tr){
  // detector ID
  int id = *p++;
  //cout << "ID :" << (hex) << *p << "\t" << (dec) << *p << endl; 

  // 4 words TS
  ULong64_t timestamp[4];
  int check[4];
  for(int i=0;i<4;i++){
    //cout << "i : " << (hex) << *p << "\t" << (dec) << *p << "\t " << (hex) << (*p & 0xffff0000) << "\t"<< (dec) << (*p & 0xffff0000) << "\t"<< (hex) << (*p & 0x0000ffff) << "\t"<< (dec) << (*p & 0x0000ffff) << endl;
    check[i] = (ULong64_t)(*p & 0xffff0000) >> 16;
    if(check[i] != checkWR[i])
      return 0;
    timestamp[i] = (ULong64_t)(*p & 0x0000ffff);
    p++;
  }
  //cout << (hex) << *p << "\t" << (dec) << *p << endl;

  tr->SetWhiteRabbit(timestamp[0] + (timestamp[1] << 16) + (timestamp[2] << 32) + (timestamp[3] << 48));
  return p;
  
}

void Unpack(int* p, TraceEvent* tr){
  //skip stuff at the beginning
  FEBEX_Add* FEBEX_add  = (FEBEX_Add*) p;
  while(FEBEX_add->add == 0xadd){
    //cout << (hex) << FEBEX_add->add << "\t" << (dec) << FEBEX_add->add << endl;
    p++;
    FEBEX_add = (FEBEX_Add*) p;
  }

  
  int board = -1;
  int nchan;
  
  int nmod = FEBEX_MODULES;
  bool febex_data = true;
  FEBEX_Header* FEBEXhead  = (FEBEX_Header*) p;
  //cout << (hex) << FEBEXhead->ff << "\t" << (dec) << FEBEXhead->ff << endl;
  while(febex_data){
    if(FEBEXhead->ff == 0xFF){
      //cout << "header ff" << endl;
      board = FEBEXhead->chan_head;
      p++;
      FEBEX_Chan_Size *fbx_size=(FEBEX_Chan_Size*) p;
      nchan = ((fbx_size->chan_size)/4) - 1;
      //cout << "nchan " << nchan << endl;
      if(nchan == 0)
	nmod--;

      p++;
      FEBEX_Half_Time *fbx_hT=(FEBEX_Half_Time*) p;

      p++;
      FEBEX_Evt_Time *fbx_time=(FEBEX_Evt_Time*) p;
      ULong64_t tmp_ext_time = ((fbx_hT->ext_time));

      p++;
      FEBEX_Flag_Hits *fbx_flag=(FEBEX_Flag_Hits*) p;
      // here are pile ups and hitpattern (not really needed?)

      p++;
      //cout << (hex) << *p << "\t" << (dec) << *p << endl;
    }// header ff
    else if (FEBEXhead->ff == 0xF0){
      //cout << "header f0" << endl;
      p--;
      vector<Trace> traces;
      traces.resize(nchan);

      for(int i=0;i<nchan;i++){
	p++;
	FEBEX_Chan_Header *fbx_Ch=(FEBEX_Chan_Header*) p;
	int chID = fbx_Ch->Ch_ID;
	if(board<0){
	  cout << "something wrong, don't have the board ID " << endl;
	  p+=3;
	  continue;
	}
	//cout << "board " << board << ", chID " << chID << endl;
	traces.at(i).SetBoard(board);
	traces.at(i).SetChn(chID);
	//cout << "energy " << endl;
	p++;
	FEBEX_TS *fbx_Ch_TS=(FEBEX_TS*) p;
	ULong64_t tmp_ext_chan_ts = (fbx_Ch->ext_chan_ts);
	traces.at(i).SetTS( (fbx_Ch_TS->chan_ts + (tmp_ext_chan_ts<<32)) *10 ); //ns

	p++;
	FEBEX_En *fbx_Ch_En=(FEBEX_En*) p;
	//cout << "fbx_Ch_En->chan_en " << fbx_Ch_En->chan_en << endl;
	int energy = fbx_Ch_En->chan_en;
	if(energy & 0x00800000){
	  energy = 0xFF000000 | fbx_Ch_En->chan_en;
	}
	traces.at(i).SetEnergy(energy);
	//tr->Print();
	p++;
      }// nchan
      nmod--;
      // now comes the traces
      //cout << "nmod " << nmod << endl;
      for(int i=0;i<nchan;i++){
	p++;
	if((*p & 0xFF) == 0x00000034){
	  p++;
	  //cout << "trace" << endl;
	  int tracesize = *p++;
	  int tracehead = *p++;
	  //cout << "size " << tracesize << " head " << tracehead << (hex) << " = " << tracehead << (dec) << endl;
	  if( ((tracehead & 0xf0000000) >> 24) != 0xa0){
	    cout << " wrong trace head " << tracehead << (hex) << " = " << tracehead << (dec) << endl;
	    continue;
	  }
	  tracesize = 2*(tracesize/4 - 2);
	  traces.at(i).SetLength(tracesize);
	  for(int t=0; t<tracesize;t+=2){
	    int part1 = *p & 0x3fff;
	    int part2 = (*p >> 16) & 0x3fff;
	    p++;
	    traces.at(i).SetTrace(t,part1);
	    traces.at(i).SetTrace(t+1,part2);
	  }
	} // header 
      }// nchan
      for(int i=0;i<nchan;i++)
	tr->AddTrace(traces.at(i));
    }// header f0
    
    // still data
    if(nmod!=0){
      p++;
      // get new data
      FEBEXhead  = (FEBEX_Header*) p;
    }
    else
      febex_data = false;
  }// febex data
}
