#ifndef __TRACE_HH
#define __TRACE_HH

#include <iostream>
#include <vector>
#include <cstdlib>
#include <math.h>
#include "TObject.h"
#include "Tracedefs.h"

using namespace std;

class Trace : public TObject {
public:
  Trace(){
    Clear();
  }
  void Clear(){
    flength = -1;
    fboard = -1;
    fchn = -1;
    fts = -1;
    ftrace.clear();
    fen = -1;
  }
  void SetTS(ULong64_t ts){fts = ts;}
  void SetLength(int length){
    flength = length;
    ftrace.resize(length);
  }
  void SetBoard(int boardid){fboard = boardid;}
  void SetChn(int chnid){fchn = chnid;}
  void SetEnergy(int en){fen = en;}
  void SetTrace(int n, short v){ftrace[n] = v;}
  
  ULong64_t GetTS(){return fts;}
  int GetLength(){return flength;}
  int GetBoard(){return fboard;}
  int GetChn(){return fchn;}
  int GetEnergy(){return fen;}

  vector <short> GetTrace(){return ftrace;}
  void Print(){
    cout << "--------------------- Trace Print ------------------------" << endl;
    cout << "flength  = " << flength  << endl;
    cout << "fboard   = " << fboard   << endl;
    cout << "fchn     = " << fchn     << endl;
    cout << "fts      = " << fts      << endl;
    cout << "fen      = " << fen      << endl;
  }

  protected:
  //! The board from which this trace came.
  int fboard;

  //! The channel from which this trace came.
  int fchn;

  //! The time stamp
  ULong64_t fts;

  //! The energy
  int fen;
  
  //! The number of points in the trace.
  int flength;

  //! the wave form (if you want to write traces to the root file remove the "//!" in the next line)
  vector <short> ftrace;


  ClassDef(Trace, 1);
};

class TraceEvent : public TObject {
public:
  TraceEvent(){
    Clear();
  }
  void Clear(){
    ftraces.clear();
    fmult = 0;
    fWRts = -1;
  }
  void AddTrace(Trace add){
    ftraces.push_back(add);
    fmult++;
  }

  void SetWhiteRabbit(ULong64_t wr){fWRts = wr;}
  
  UShort_t GetMult(){return fmult;}
  Trace* GetTrace(int n){return &ftraces[n];}
  vector<Trace>* GetTrace(){return &ftraces;}
  ULong64_t  GetWhiteRabbit(){return fWRts;}
protected:
  UShort_t fmult;
  ULong64_t  fWRts;
  vector<Trace> ftraces;

  ClassDef(TraceEvent, 1);
};


#endif

